#!/usr/bin/env python3
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import fnmatch
import html
import json
import os
import socket
import shutil
import subprocess
import sys
import urllib.parse

ROOT = Path(__file__).resolve().parents[1]
HOST = "127.0.0.1"
PORT = 8088


def run_cmd(args, cwd=ROOT, timeout=120):
    try:
        proc = subprocess.run(
            args,
            cwd=str(cwd),
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=timeout,
        )
        return {"ok": proc.returncode == 0, "code": proc.returncode, "output": proc.stdout}
    except subprocess.TimeoutExpired as exc:
        return {"ok": False, "code": 124, "output": f"Timeout after {timeout}s\n{exc.stdout or ''}"}
    except FileNotFoundError as exc:
        return {"ok": False, "code": 127, "output": str(exc)}


def safe_path(value):
    p = Path(value or ".").expanduser()
    return p if p.is_absolute() else (ROOT / p).resolve()


def table(rows):
    if not rows:
        return "Khong co ket qua."
    return "\n".join(f"{i + 1:>3}. {row}" for i, row in enumerate(rows))


def valid_cron_field(value, low=None, high=None):
    if value == "*":
        return True
    if not value.isdigit():
        return False
    number = int(value)
    return (low is None or number >= low) and (high is None or number <= high)


def action_file_create_dir(data):
    path = safe_path(data.get("path"))
    path.mkdir(parents=True, exist_ok=True)
    return {"ok": True, "output": f"Da tao thu muc: {path}"}


def action_file_create_file(data):
    path = safe_path(data.get("path"))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.touch(exist_ok=True)
    return {"ok": True, "output": f"Da tao file: {path}"}


def action_file_delete(data):
    path = safe_path(data.get("path"))
    if not path.exists():
        return {"ok": False, "output": f"Khong ton tai: {path}"}
    if path.is_dir():
        shutil.rmtree(path)
    else:
        path.unlink()
    return {"ok": True, "output": f"Da xoa: {path}"}


def action_file_copy(data):
    src, dst = safe_path(data.get("src")), safe_path(data.get("dst"))
    if not src.exists():
        return {"ok": False, "output": f"Nguon khong ton tai: {src}"}
    if src.is_dir():
        shutil.copytree(src, dst, dirs_exist_ok=True)
    else:
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)
    return {"ok": True, "output": f"Da copy {src} -> {dst}"}


def action_file_move(data):
    src, dst = safe_path(data.get("src")), safe_path(data.get("dst"))
    if not src.exists():
        return {"ok": False, "output": f"Nguon khong ton tai: {src}"}
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.move(str(src), str(dst))
    return {"ok": True, "output": f"Da move {src} -> {dst}"}


def action_file_find_name(data):
    base = safe_path(data.get("base") or ".")
    pattern = data.get("pattern") or "*"
    rows = []
    for dirpath, _, files in os.walk(base):
        names = files + [name for name in os.listdir(dirpath) if (Path(dirpath) / name).is_dir()]
        for name in sorted(set(names)):
            if fnmatch.fnmatch(name, pattern):
                rows.append(str(Path(dirpath) / name))
    return {"ok": True, "output": table(rows[:300])}


def action_file_find_content(data):
    base = safe_path(data.get("base") or ".")
    keyword = data.get("keyword") or ""
    rows = []
    if not keyword:
        return {"ok": False, "output": "Nhap keyword truoc."}
    for path in base.rglob("*") if base.is_dir() else [base]:
        if not path.is_file():
            continue
        try:
            for no, line in enumerate(path.read_text(errors="ignore").splitlines(), 1):
                if keyword in line:
                    rows.append(f"{path}:{no}: {line[:160]}")
        except OSError:
            pass
    return {"ok": True, "output": table(rows[:300])}


def action_cron_list(_):
    return run_cmd(["crontab", "-l"])


def action_cron_add(data):
    minute = data.get("minute", "*")
    hour = data.get("hour", "*")
    command = data.get("command", "").strip()
    if not command:
        return {"ok": False, "output": "Lenh cron dang trong."}
    if not (valid_cron_field(minute, 0, 59) and valid_cron_field(hour, 0, 23)):
        return {"ok": False, "output": "Minute/hour khong hop le."}
    current = run_cmd(["crontab", "-l"])
    lines = [] if current["code"] != 0 else [line for line in current["output"].splitlines() if line.strip()]
    lines.append(f"{minute} {hour} * * * {command}")
    proc = subprocess.run(["crontab", "-"], input="\n".join(lines) + "\n", text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"ok": proc.returncode == 0, "output": proc.stdout or "Da them cron job."}


def action_cron_delete(data):
    index = int(data.get("index") or 0)
    current = run_cmd(["crontab", "-l"])
    if current["code"] != 0:
        return current
    lines = [line for line in current["output"].splitlines() if line.strip()]
    if index < 1 or index > len(lines):
        return {"ok": False, "output": "STT khong hop le."}
    removed = lines.pop(index - 1)
    proc = subprocess.run(["crontab", "-"], input="\n".join(lines) + "\n", text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"ok": proc.returncode == 0, "output": proc.stdout or f"Da xoa: {removed}"}


ACTIONS = {
    "file.create_dir": action_file_create_dir,
    "file.create_file": action_file_create_file,
    "file.delete": action_file_delete,
    "file.copy": action_file_copy,
    "file.move": action_file_move,
    "file.find_name": action_file_find_name,
    "file.find_content": action_file_find_content,
    "cron.list": action_cron_list,
    "cron.add": action_cron_add,
    "cron.delete": action_cron_delete,
    "time.status": lambda data: run_cmd(["timedatectl"]),
    "time.hwclock": lambda data: run_cmd(["hwclock"]),
    "time.set_timezone": lambda data: run_cmd(["sudo", "timedatectl", "set-timezone", data.get("timezone", "")]),
    "time.ntp_on": lambda data: run_cmd(["sudo", "timedatectl", "set-ntp", "true"]),
    "time.set_manual": lambda data: run_cmd(["sudo", "timedatectl", "set-time", data.get("time", "")]),
    "pkg.search": lambda data: run_cmd(["apt-cache", "search", data.get("query", "")]),
    "pkg.show": lambda data: run_cmd(["apt-cache", "show", data.get("package", "")]),
    "pkg.list": lambda data: run_cmd(["bash", "-lc", f"dpkg -l | grep -i -- {sh_quote(data.get('pattern', ''))} || true"]),
    "pkg.install": lambda data: run_cmd(["sudo", "apt-get", "install", "-y", data.get("package", "")], timeout=600),
    "pkg.remove": lambda data: run_cmd(["sudo", "apt-get", "remove", "--purge", "-y", data.get("package", "")], timeout=600),
    "system.build": lambda data: run_cmd(["make"], cwd=ROOT / "02_system_prog", timeout=300),
    "system.process": lambda data: run_cmd(["./bin/process_mgmt"], cwd=ROOT / "02_system_prog"),
    "system.file": lambda data: run_cmd(["./bin/file_ops"], cwd=ROOT / "02_system_prog"),
    "system.socket": lambda data: run_cmd(["./bin/socket_demo"], cwd=ROOT / "02_system_prog"),
    "system.network": lambda data: run_cmd(["./bin/network_info"], cwd=ROOT / "02_system_prog"),
    "kernel.build": lambda data: run_cmd(["make"], cwd=ROOT / "03_kernel_module", timeout=300),
    "kernel.load": lambda data: run_cmd(["sudo", "insmod", "hello_module.ko"], cwd=ROOT / "03_kernel_module"),
    "kernel.proc": lambda data: run_cmd(["cat", "/proc/hello_info"]),
    "kernel.dmesg": lambda data: run_cmd(["sudo", "dmesg", "--ctime", "--level=info,err,warn"], timeout=20),
    "kernel.unload": lambda data: run_cmd(["sudo", "rmmod", "hello_module"]),
    "driver.build": lambda data: run_cmd(["make"], cwd=ROOT / "04_char_driver", timeout=300),
    "driver.load": lambda data: run_cmd(["sudo", "insmod", "mychardev.ko"], cwd=ROOT / "04_char_driver"),
    "driver.nodes": lambda data: run_cmd(["bash", "-lc", "sudo mknod -m 666 /dev/mychardev0 c 42 0 2>/dev/null || true; sudo mknod -m 666 /dev/mychardev1 c 42 1 2>/dev/null || true; sudo chmod 666 /dev/mychardev0 /dev/mychardev1"]),
    "driver.test_app": lambda data: run_cmd(["./test_app"], cwd=ROOT / "04_char_driver"),
    "driver.test_blocking": lambda data: run_cmd(["./test_blocking"], cwd=ROOT / "04_char_driver", timeout=20),
    "driver.unload": lambda data: run_cmd(["sudo", "rmmod", "mychardev"]),
}


def sh_quote(value):
    return "'" + str(value).replace("'", "'\"'\"'") + "'"


INDEX_HTML = r"""<!doctype html>
<html lang="vi">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Linux SysProject Control Center</title>
<style>
:root {
  --bg: #f5f7fb;
  --ink: #172033;
  --muted: #647084;
  --panel: #ffffff;
  --line: #dce3ee;
  --cyan: #0f9fb3;
  --green: #16885a;
  --red: #c2413a;
  --yellow: #a66a00;
  --violet: #6750a4;
  --shadow: 0 14px 34px rgba(25, 37, 62, .10);
}
* { box-sizing: border-box; }
body { margin: 0; background: var(--bg); color: var(--ink); font: 14px/1.45 system-ui, -apple-system, Segoe UI, sans-serif; }
.app { min-height: 100vh; display: grid; grid-template-columns: 280px 1fr; }
.side { background: #132033; color: #eef5ff; padding: 22px 16px; display: flex; flex-direction: column; gap: 18px; }
.brand { padding: 8px 8px 18px; border-bottom: 1px solid rgba(255,255,255,.14); }
.brand h1 { font-size: 20px; line-height: 1.1; margin: 0 0 8px; letter-spacing: 0; }
.brand p { margin: 0; color: #aebbd0; }
.nav { display: grid; gap: 8px; }
.nav button { width: 100%; border: 0; border-radius: 8px; padding: 12px 12px; background: transparent; color: #dce8f8; text-align: left; cursor: pointer; display: flex; gap: 10px; align-items: center; font-weight: 650; }
.nav button:hover, .nav button.active { background: rgba(255,255,255,.12); }
.main { padding: 24px; display: grid; grid-template-rows: auto 1fr; gap: 18px; min-width: 0; }
.top { display: flex; justify-content: space-between; gap: 16px; align-items: center; }
.top h2 { margin: 0; font-size: 26px; letter-spacing: 0; }
.clock { color: var(--muted); font-weight: 650; }
.workspace { display: grid; grid-template-columns: minmax(0, 1fr) 420px; gap: 18px; align-items: start; }
.section { display: none; }
.section.active { display: block; }
.grid { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr)); gap: 14px; }
.panel { background: var(--panel); border: 1px solid var(--line); border-radius: 8px; box-shadow: var(--shadow); padding: 16px; }
.panel h3 { margin: 0 0 12px; font-size: 16px; }
.row { display: grid; gap: 10px; margin: 10px 0; }
.two { grid-template-columns: repeat(2, minmax(0, 1fr)); }
label { color: var(--muted); font-size: 12px; font-weight: 750; text-transform: uppercase; }
input { width: 100%; border: 1px solid var(--line); border-radius: 8px; padding: 10px 12px; font: inherit; color: var(--ink); background: #fbfcff; }
.actions { display: flex; flex-wrap: wrap; gap: 8px; margin-top: 12px; }
button.action { border: 0; border-radius: 8px; padding: 10px 12px; background: var(--cyan); color: #fff; font-weight: 750; cursor: pointer; }
button.action.secondary { background: var(--violet); }
button.action.warn { background: var(--yellow); }
button.action.danger { background: var(--red); }
button.action.ok { background: var(--green); }
button.action:disabled { opacity: .55; cursor: wait; }
.output { position: sticky; top: 24px; background: #0d1422; color: #d8f1ff; border-radius: 8px; overflow: hidden; box-shadow: var(--shadow); border: 1px solid #24324a; }
.output header { display: flex; justify-content: space-between; align-items: center; padding: 12px 14px; background: #111d30; border-bottom: 1px solid #263550; }
.status { color: #90e3a5; font-weight: 750; }
pre { margin: 0; padding: 14px; min-height: 520px; max-height: 70vh; overflow: auto; white-space: pre-wrap; font: 13px/1.45 ui-monospace, SFMono-Regular, Menlo, Consolas, monospace; }
.hint { color: var(--muted); margin: 0 0 12px; }
.pill { display: inline-block; padding: 4px 8px; border-radius: 999px; background: #e9f6f8; color: #087b8c; font-weight: 750; font-size: 12px; }
@media (max-width: 1040px) {
  .app { grid-template-columns: 1fr; }
  .side { position: static; }
  .workspace { grid-template-columns: 1fr; }
  .grid { grid-template-columns: 1fr; }
  .output { position: static; }
}
</style>
</head>
<body>
<div class="app">
  <aside class="side">
    <div class="brand"><h1>Linux SysProject</h1><p>Control Center</p></div>
    <nav class="nav">
      <button class="active" data-tab="files">▣ File Manager</button>
      <button data-tab="cron">◷ Cron Scheduler</button>
      <button data-tab="time">◴ Time Manager</button>
      <button data-tab="pkg">⬢ Package Manager</button>
      <button data-tab="system">▸ System C Demos</button>
      <button data-tab="kernel">◆ Kernel Module</button>
      <button data-tab="driver">▤ Char Driver</button>
    </nav>
  </aside>
  <main class="main">
    <div class="top">
      <div><span class="pill">Local App</span><h2 id="title">File Manager</h2></div>
      <div class="clock" id="clock"></div>
    </div>
    <div class="workspace">
      <div>
        <section class="section active" id="files">
          <div class="grid">
            <div class="panel"><h3>Tao / xoa</h3><p class="hint">Duong dan tuong doi se tinh tu root project.</p>
              <div class="row"><label>Path</label><input id="file-path" value="tmp/demo.txt"></div>
              <div class="actions">
                <button class="action ok" data-action="file.create_file" data-fields="path:file-path">Tao file</button>
                <button class="action secondary" data-action="file.create_dir" data-fields="path:file-path">Tao thu muc</button>
                <button class="action danger" data-confirm="Xoa path nay?" data-action="file.delete" data-fields="path:file-path">Xoa</button>
              </div>
            </div>
            <div class="panel"><h3>Copy / move</h3>
              <div class="row two"><div><label>Nguon</label><input id="copy-src" value="README.md"></div><div><label>Dich</label><input id="copy-dst" value="tmp/README.copy.md"></div></div>
              <div class="actions">
                <button class="action" data-action="file.copy" data-fields="src:copy-src,dst:copy-dst">Copy</button>
                <button class="action warn" data-confirm="Move file/thu muc nay?" data-action="file.move" data-fields="src:copy-src,dst:copy-dst">Move</button>
              </div>
            </div>
            <div class="panel"><h3>Tim theo ten</h3>
              <div class="row two"><div><label>Thu muc</label><input id="find-base" value="."></div><div><label>Pattern</label><input id="find-pattern" value="*.c"></div></div>
              <button class="action" data-action="file.find_name" data-fields="base:find-base,pattern:find-pattern">Tim</button>
            </div>
            <div class="panel"><h3>Tim noi dung</h3>
              <div class="row two"><div><label>Thu muc</label><input id="grep-base" value="."></div><div><label>Keyword</label><input id="grep-keyword" value="MODULE_LICENSE"></div></div>
              <button class="action" data-action="file.find_content" data-fields="base:grep-base,keyword:grep-keyword">Grep</button>
            </div>
          </div>
        </section>
        <section class="section" id="cron">
          <div class="grid">
            <div class="panel"><h3>Danh sach cron</h3><button class="action" data-action="cron.list">Tai danh sach</button></div>
            <div class="panel"><h3>Them job</h3>
              <div class="row two"><div><label>Phut</label><input id="cron-minute" value="*"></div><div><label>Gio</label><input id="cron-hour" value="*"></div></div>
              <div class="row"><label>Lenh</label><input id="cron-command" value="echo hello >> /tmp/sysproject.log"></div>
              <button class="action ok" data-confirm="Them cron job nay?" data-action="cron.add" data-fields="minute:cron-minute,hour:cron-hour,command:cron-command">Them job</button>
            </div>
            <div class="panel"><h3>Xoa job</h3><div class="row"><label>STT</label><input id="cron-index" value="1"></div><button class="action danger" data-confirm="Xoa cron job theo STT?" data-action="cron.delete" data-fields="index:cron-index">Xoa job</button></div>
          </div>
        </section>
        <section class="section" id="time">
          <div class="grid">
            <div class="panel"><h3>Trang thai thoi gian</h3><div class="actions"><button class="action" data-action="time.status">timedatectl</button><button class="action secondary" data-action="time.hwclock">Hardware clock</button></div></div>
            <div class="panel"><h3>Dat timezone</h3><div class="row"><label>Timezone</label><input id="timezone" value="Asia/Ho_Chi_Minh"></div><button class="action warn" data-confirm="Can sudo. Tiep tuc?" data-action="time.set_timezone" data-fields="timezone:timezone">Dat timezone</button></div>
            <div class="panel"><h3>NTP</h3><button class="action ok" data-confirm="Bat NTP sync?" data-action="time.ntp_on">Bat NTP</button></div>
            <div class="panel"><h3>Dat gio thu cong</h3><div class="row"><label>YYYY-MM-DD HH:MM:SS</label><input id="manual-time" value="2026-06-14 12:00:00"></div><button class="action danger" data-confirm="Can sudo va co the anh huong he thong. Tiep tuc?" data-action="time.set_manual" data-fields="time:manual-time">Dat gio</button></div>
          </div>
        </section>
        <section class="section" id="pkg">
          <div class="grid">
            <div class="panel"><h3>Tim package</h3><div class="row"><label>Tu khoa</label><input id="pkg-query" value="gcc"></div><button class="action" data-action="pkg.search" data-fields="query:pkg-query">Search</button></div>
            <div class="panel"><h3>Thong tin package</h3><div class="row"><label>Package</label><input id="pkg-name" value="build-essential"></div><button class="action secondary" data-action="pkg.show" data-fields="package:pkg-name">Show</button></div>
            <div class="panel"><h3>Cai / go</h3><div class="row"><label>Package</label><input id="pkg-install-name" value="build-essential"></div><div class="actions"><button class="action ok" data-confirm="Cai package bang sudo apt-get?" data-action="pkg.install" data-fields="package:pkg-install-name">Install</button><button class="action danger" data-confirm="Go package bang sudo apt-get remove --purge?" data-action="pkg.remove" data-fields="package:pkg-install-name">Remove</button></div></div>
            <div class="panel"><h3>Da cai</h3><div class="row"><label>Pattern</label><input id="pkg-pattern" value="gcc"></div><button class="action" data-action="pkg.list" data-fields="pattern:pkg-pattern">List</button></div>
          </div>
        </section>
        <section class="section" id="system"><div class="panel"><h3>System Programming C</h3><p class="hint">Build truoc, sau do chay tung demo.</p><div class="actions"><button class="action ok" data-action="system.build">Build</button><button class="action" data-action="system.process">Process</button><button class="action" data-action="system.file">File Ops</button><button class="action" data-action="system.socket">Socket</button><button class="action" data-action="system.network">Network</button></div></div></section>
        <section class="section" id="kernel"><div class="panel"><h3>Hello Kernel Module</h3><div class="actions"><button class="action ok" data-action="kernel.build">Build</button><button class="action warn" data-confirm="Load kernel module bang sudo insmod?" data-action="kernel.load">Load</button><button class="action" data-action="kernel.proc">/proc/hello_info</button><button class="action secondary" data-action="kernel.dmesg">dmesg</button><button class="action danger" data-confirm="Unload hello_module?" data-action="kernel.unload">Unload</button></div></div></section>
        <section class="section" id="driver"><div class="panel"><h3>Character Device Driver</h3><div class="actions"><button class="action ok" data-action="driver.build">Build</button><button class="action warn" data-confirm="Load mychardev bang sudo insmod?" data-action="driver.load">Load</button><button class="action secondary" data-confirm="Tao /dev/mychardev0 va /dev/mychardev1?" data-action="driver.nodes">Create nodes</button><button class="action" data-action="driver.test_app">Test app</button><button class="action" data-action="driver.test_blocking">Blocking test</button><button class="action danger" data-confirm="Unload mychardev?" data-action="driver.unload">Unload</button></div></div></section>
      </div>
      <aside class="output"><header><strong>Output</strong><span class="status" id="status">Ready</span></header><pre id="out">Chon mot thao tac de bat dau.</pre></aside>
    </div>
  </main>
</div>
<script>
const titles = {files:'File Manager', cron:'Cron Scheduler', time:'Time Manager', pkg:'Package Manager', system:'System C Demos', kernel:'Kernel Module', driver:'Character Driver'};
const out = document.getElementById('out'), statusEl = document.getElementById('status');
function tick(){ document.getElementById('clock').textContent = new Date().toLocaleString('vi-VN'); }
setInterval(tick, 1000); tick();
document.querySelectorAll('.nav button').forEach(btn => btn.addEventListener('click', () => {
  document.querySelectorAll('.nav button').forEach(b => b.classList.remove('active'));
  document.querySelectorAll('.section').forEach(s => s.classList.remove('active'));
  btn.classList.add('active');
  document.getElementById(btn.dataset.tab).classList.add('active');
  document.getElementById('title').textContent = titles[btn.dataset.tab];
}));
function payload(btn) {
  const data = {};
  (btn.dataset.fields || '').split(',').filter(Boolean).forEach(pair => {
    const [key, id] = pair.split(':');
    data[key] = document.getElementById(id).value;
  });
  return data;
}
async function run(btn) {
  if (btn.dataset.confirm && !confirm(btn.dataset.confirm)) return;
  document.querySelectorAll('button.action').forEach(b => b.disabled = true);
  statusEl.textContent = 'Running';
  out.textContent = '$ ' + btn.dataset.action + '\n';
  try {
    const res = await fetch('/api/action', {method:'POST', headers:{'Content-Type':'application/json'}, body: JSON.stringify({action: btn.dataset.action, data: payload(btn)})});
    const json = await res.json();
    statusEl.textContent = json.ok ? 'OK' : 'Error';
    out.textContent += (json.output || '').trim() || '(khong co output)';
  } catch (err) {
    statusEl.textContent = 'Error';
    out.textContent += String(err);
  } finally {
    document.querySelectorAll('button.action').forEach(b => b.disabled = false);
  }
}
document.querySelectorAll('button.action').forEach(btn => btn.addEventListener('click', () => run(btn)));
</script>
</body>
</html>
"""


class Handler(BaseHTTPRequestHandler):
    def send_json(self, payload, status=200):
        body = json.dumps(payload).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if urllib.parse.urlparse(self.path).path != "/":
            self.send_error(404)
            return
        body = INDEX_HTML.encode()
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        if urllib.parse.urlparse(self.path).path != "/api/action":
            self.send_error(404)
            return
        length = int(self.headers.get("Content-Length", "0"))
        try:
            payload = json.loads(self.rfile.read(length) or b"{}")
        except json.JSONDecodeError:
            self.send_json({"ok": False, "output": "JSON khong hop le"}, 400)
            return
        action = payload.get("action")
        fn = ACTIONS.get(action)
        if not fn:
            self.send_json({"ok": False, "output": f"Action khong duoc ho tro: {action}"}, 400)
            return
        self.send_json(fn(payload.get("data") or {}))

    def log_message(self, fmt, *args):
        print("[%s] %s" % (self.log_date_time_string(), fmt % args))


def main():
    os.chdir(ROOT)
    port = int(os.environ.get("PORT", PORT))
    while True:
        try:
            server = ThreadingHTTPServer((HOST, port), Handler)
            break
        except OSError as exc:
            if exc.errno != 98 or port >= PORT + 20:
                raise
            port += 1
    print(f"Linux SysProject GUI running at http://{HOST}:{port}")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping GUI app.")


if __name__ == "__main__":
    main()
