# Module 2 - System Programming C

Moi file C la mot demo doc lap. Build bang:

```bash
make
make run
```

Binary nam trong `bin/`.

## Demo

- `process_mgmt`: `fork`, `exec`, `waitpid`, `/proc/<pid>/status`, `sigaction`, `kill`.
- `file_ops`: low-level I/O, `mmap`, `inotify`.
- `socket_demo`: HTTP server viet bang TCP socket tren port 9090. Server tra ve
  mot trang HTML cho trinh duyet va chay den khi nhan `Ctrl+C`.
- `network_info`: `getifaddrs`, MAC qua `ioctl`, thong ke `/proc/net/dev`.
- `main_demo`: menu goi cac binary da build.

## Test HTTP socket

```bash
./bin/socket_demo
```

Mo `http://127.0.0.1:9090/` trong trinh duyet. Sau khi test xong, quay lai
terminal va nhan `Ctrl+C` de dung server.
