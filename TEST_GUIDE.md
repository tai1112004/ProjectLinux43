# Huong Dan Test Va Su Dung Linux SysProject

File nay huong dan test tung phan cua project. Tat ca lenh ben duoi chay tu thu muc:

```bash
cd /home/tai/Desktop/project_43/linux-sysproject
```

## 0. Chuan bi

Kiem tra toolchain:

```bash
make --version
gcc --version
uname -r
```

Neu thieu compiler/header:

```bash
sudo apt-get update
sudo apt-get install -y build-essential linux-headers-$(uname -r)
```

Neu may Kali bao khong tim thay header theo dung kernel:

```bash
sudo apt-get install -y linux-headers-amd64
```

Build toan bo project:

```bash
make clean
make
```

Neu chi muon build tung phan:

```bash
make shell
make system
make kernel
make driver
```

## 1. Su dung GUI App

Chay giao dien app:

```bash
make gui
```

Mo URL duoc in ra, vi du:

```text
http://127.0.0.1:8088
```

Neu port `8088` ban, app se tu chuyen sang port tiep theo nhu `8089`.

Trong GUI co cac tab:

- `File Manager`: tao/xoa/copy/move/tim file.
- `Cron Scheduler`: xem/them/xoa cron job.
- `Time Manager`: xem gio, timezone, hardware clock, bat NTP, dat timezone/gio.
- `Package Manager`: tim, xem thong tin, cai, go package.
- `System C Demos`: build va chay cac demo C.
- `Kernel Module`: build/load/doc proc/dmesg/unload hello module.
- `Char Driver`: build/load/tao device node/chay test/unload driver.

### Muc dich cua GUI App

GUI App la lop giao dien dieu khien nam tren cac module da co. Muc dich cua no la bien cac thao tac terminal thanh cac nut bam, form nhap va khung output de nguoi dung de thao tac hon. No khong thay doi logic Linux system programming ben trong: khi bam nut build, test, load module, tao cron, cai package... app van goi cac lenh Linux va chuong trinh C/kernel driver tuong ung.

Luu y: cac nut co thao tac he thong nhu install package, set time, load kernel module, unload driver van can `sudo`. Hay chay `make gui` trong terminal de neu sudo hoi password thi ban nhap tai terminal do.

## 2. Module 1 - Shell Manager

### Module nay dung de lam gi?

Module 1 mo phong mot cong cu quan tri Linux bang Bash. No gom cac thao tac he thong pho bien ma admin hay dung: quan ly file, lap lich cron, xem/chinh thoi gian he thong va quan ly package. Muc tieu hoc tap cua module nay la ren Bash scripting, menu tuong tac, validate input, confirm truoc thao tac nguy hiem, xu ly lenh Linux va tao terminal UI bang ANSI colors.

### Y nghia tung phan

- `main.sh`: diem vao cua chuong trinh, hien menu chinh va dieu huong sang cac module con.
- `lib/ui.sh`: thu vien UI dung chung, gom header, mau sac, thong bao OK/error/info, confirm va spinner.
- `file_manager.sh`: thuc hanh cac lenh file system nhu `mkdir`, `touch`, `rm`, `cp`, `mv`, `find`, `grep`.
- `scheduler.sh`: quan ly cron job de lap lich chay lenh tu dong theo gio/phut.
- `time_manager.sh`: xem va cau hinh thoi gian he thong, timezone, NTP.
- `pkg_manager.sh`: tim, cai, go va liet ke package bang APT/dpkg.

Chay menu terminal:

```bash
cd 01_shell_manager
make run
```

Hoac:

```bash
./main.sh
```

Test nhanh:

1. Chon `File manager`.
2. Tao file `tmp/hello.txt`.
3. Tim theo ten `*.txt`.
4. Tim theo noi dung sau khi ban ghi thu noi dung vao file.
5. Thu xoa file, chu y man hinh confirm `Y/n`.

Test cron:

Chuc nang cron dung de lap lich cho he thong tu dong chay mot lenh nao do. Vi du, ban co the lap lich backup, ghi log, dong bo file, hoac chay script bao tri moi phut/moi gio/moi ngay.

1. Chon `Cron scheduler`.
2. Xem cron jobs hien tai.
3. Them job:

```text
minute: *
hour: *
command: echo sysproject >> /tmp/sysproject-cron.log
```

4. Xem lai danh sach.
5. Xoa job theo so thu tu.

Test time/package:

- `Time manager` co cac thao tac xem thoi gian khong can sudo.
- Dat timezone, bat NTP, dat gio thu cong can sudo.
- `Package manager` search/show/list khong can sudo.
- Install/remove package can sudo va co confirm.

`Time manager` dung de quan sat va dieu chinh dong ho he thong. Dieu nay quan trong vi log, cron, chung chi TLS va nhieu dich vu Linux phu thuoc vao thoi gian dung.

`Package manager` dung de quan ly phan mem cai tren may. Confirm truoc install/remove giup tranh thao tac nguy hiem lam anh huong he thong.

Quay ve root project:

```bash
cd ..
```

## 3. Module 2 - System Programming C

### Module nay dung de lam gi?

Module 2 gom cac demo C de minh hoa lap trinh he thong Linux o userspace. Thay vi chi goi lenh shell, module nay dung truc tiep system call/API cua Linux nhu `fork`, `exec`, `wait`, `open`, `read`, `write`, `mmap`, `inotify`, socket TCP va `getifaddrs`.

### Y nghia tung demo

- `process_mgmt`: minh hoa cach Linux tao process con bang `fork`, thay the chuong trinh bang `exec`, parent doi child bang `waitpid`, doc thong tin process trong `/proc`, va gui/bat signal.
- `file_ops`: minh hoa low-level file I/O khong qua `stdio`, doc/ghi theo offset, map file vao memory bang `mmap`, va theo doi su kien file bang `inotify`.
- `socket_demo`: minh hoa network programming voi TCP server/client, `bind`, `listen`, `accept`, `connect`, `recv`, `send`, `select` va xu ly client bang `fork`.
- `network_info`: minh hoa cach lay thong tin card mang bang `getifaddrs`, lay MAC bang `ioctl`, va doc thong ke RX/TX tu `/proc/net/dev`.
- `main_demo`: menu tong de chay cac binary demo da build.

Build:

```bash
cd 02_system_prog
make
```

Chay menu demo:

```bash
./bin/main_demo
```

Hoac chay tung demo:

```bash
./bin/process_mgmt
./bin/file_ops
./bin/socket_demo
./bin/network_info
```

Ky vong ket qua:

- `process_mgmt`: in PID child, doc `/proc/<pid>/status`, gui signal, chay `/bin/ls`.
- `file_ops`: tao file tam, ghi/doc low-level I/O, sua bang `mmap`, watch `/tmp` bang `inotify`.
- `socket_demo`: tao TCP echo server local port `9090`, client gui chuoi va nhan echo.
- `network_info`: in bang interface, IPv4/IPv6, MAC, RX/TX bytes.

Luu y voi `file_ops`: khi dang watch `/tmp`, co the mo terminal khac va chay:

```bash
touch /tmp/sysproject-demo
rm /tmp/sysproject-demo
```

Quay ve root:

```bash
cd ..
```

## 4. Module 3 - Kernel Module

### Module nay dung de lam gi?

Module 3 minh hoa cach viet mot Linux kernel module don gian. Khi load module vao kernel, module tao entry `/proc/hello_info` de userspace co the doc thong tin tu kernel. Muc tieu la hieu vong doi kernel module: build, load, init, tao procfs entry, doc du lieu bang `seq_file`, ghi log bang `printk`, va unload/cleanup.

### Y nghia tung thanh phan

- `hello_module.c`: source kernel module.
- `module_init()`: dang ky ham se chay khi `insmod`.
- `module_exit()`: dang ky ham se chay khi `rmmod`.
- `MODULE_LICENSE`, `MODULE_AUTHOR`, `MODULE_DESCRIPTION`: metadata cua module.
- `__init`: danh dau code chi dung luc khoi tao module.
- `__exit`: danh dau code chi dung luc go module.
- `proc_create`: tao file ao trong `/proc`.
- `seq_file`: cach an toan, gon gang de in du lieu kernel ra userspace.
- `printk`: ghi log tu kernel, xem bang `dmesg`.
- `test_module.sh`: script tu dong build, load, verify, doc proc, xem log va unload.

Build:

```bash
cd 03_kernel_module
make
```

Test tu dong:

```bash
sudo ./test_module.sh
```

Test thu cong:

```bash
sudo insmod hello_module.ko
lsmod | grep hello_module
cat /proc/hello_info
sudo dmesg | tail -20
sudo rmmod hello_module
```

Ky vong:

- `insmod` load module thanh cong.
- `/proc/hello_info` hien module name, kernel version, uptime, loaded time.
- `dmesg` co log load/unload.

Neu `insmod` bao file ton tai/module da load:

```bash
sudo rmmod hello_module
sudo insmod hello_module.ko
```

Quay ve root:

```bash
cd ..
```

## 5. Module 4 - Character Device Driver

### Module nay dung de lam gi?

Module 4 minh hoa mot character device driver day du hon. Driver dang ky major number, tao nhieu minor device, quan ly buffer noi bo, cho phep userspace doc/ghi qua `/dev/mychardev*`, dieu khien driver bang ioctl, va dung wait queue de blocking read cho den khi co du lieu.

Day la phan gan voi kien thuc driver thuc te nhat: userspace giao tiep voi kernel qua device file, con kernel cung cap `file_operations` de xu ly `open`, `read`, `write`, `ioctl`, `release`.

### Y nghia tung thanh phan

- `mychardev.h`: dinh nghia major number, so minor, ten device va cac lenh ioctl.
- `struct my_device_data`: du lieu rieng cua moi minor device, gom `cdev`, buffer, size, atomic access, wait queue va flag `data_ready`.
- `register_chrdev_region`: dang ky vung major/minor voi kernel.
- `cdev_init` va `cdev_add`: khoi tao va them character device vao kernel.
- `my_open`: lay device data bang `container_of`, chan mo dong thoi bang `atomic_cmpxchg`.
- `my_release`: giai phong trang thai busy khi dong file.
- `my_read`: neu chua co data thi block bang `wait_event_interruptible`, sau do `copy_to_user`.
- `my_write`: nhan data tu userspace bang `copy_from_user`, cap nhat buffer, dat `data_ready` va wake up wait queue.
- `my_ioctl`: xu ly cac lenh dieu khien rieng nhu reset buffer, set size, get size.
- `cleanup`: xoa `cdev` va huy dang ky major/minor khi unload.
- `test_app.c`: test doc/ghi/ioctl tu userspace.
- `test_blocking.c`: test co che blocking read va wake queue bang thread.

Build:

```bash
cd 04_char_driver
make
```

Load driver:

```bash
sudo insmod mychardev.ko
```

Tao device nodes:

```bash
sudo mknod /dev/mychardev0 c 42 0
sudo mknod /dev/mychardev1 c 42 1
sudo chmod 666 /dev/mychardev0 /dev/mychardev1
```

Neu `mknod` bao file da ton tai thi bo qua, hoac chay:

```bash
ls -l /dev/mychardev*
```

Chay userspace test:

```bash
./test_app
```

Ky vong:

- Mo `/dev/mychardev0`.
- Ghi chuoi `"Hello from userspace!"`.
- `read()` doc lai dung chuoi.
- `ioctl GETSIZE` tra ve size.
- `ioctl RESET` reset buffer.
- Read nonblocking sau reset tra ve empty/EAGAIN.

Chay blocking test:

```bash
./test_blocking
```

Ky vong:

- Thread reader goi `read()` va bi block.
- Sau 2 giay, thread writer ghi data vao minor khac.
- Reader duoc wake up va in data doc duoc.

Xem kernel log:

```bash
sudo dmesg | tail -30
```

Unload driver:

```bash
sudo rmmod mychardev
```

Neu muon xoa device nodes:

```bash
sudo rm -f /dev/mychardev0 /dev/mychardev1
```

Quay ve root:

```bash
cd ..
```

## 6. Thu tu demo goi y khi bao cao

Thu tu nay de trinh bay de hieu:

1. Mo GUI bang `make gui`, gioi thieu cac tab.
2. Test `File Manager` trong GUI: tao file, tim file, xoa co confirm.
3. Test `System C Demos`: build va chay `process_mgmt`, `network_info`.
4. Test `Kernel Module`: build, load, doc `/proc/hello_info`, unload.
5. Test `Char Driver`: load, tao device node, chay `test_app`, chay `test_blocking`, unload.
6. Neu can, mo terminal UI cu bang `01_shell_manager/main.sh` de chung minh van giu dung yeu cau terminal ban dau.

## 7. Loi thuong gap

### Address already in use khi chay GUI

Port dang bi chiem. App da tu nhay port, hay doc dung URL no in ra. Neu muon chi dinh port:

```bash
PORT=8099 make gui
```

### Khong co gcc/cc

```bash
sudo apt-get install -y build-essential
```

### Khong build duoc kernel module

Kiem tra header:

```bash
ls /lib/modules/$(uname -r)/build
```

Neu khong ton tai:

```bash
sudo apt-get install -y linux-headers-$(uname -r)
```

Hoac tren Kali:

```bash
sudo apt-get install -y linux-headers-amd64
```

### `insmod` bao Operation not permitted

Hay chay bang sudo:

```bash
sudo insmod hello_module.ko
```

Neu dang trong moi truong VM/container bi chan kernel module, can chay tren may Linux/VM co quyen load kernel module.

### `Device or resource busy` khi unload driver/module

Dong cac chuong trinh dang mo device, roi thu lai:

```bash
sudo rmmod mychardev
```

### `sudo` trong GUI khong hien prompt

Chay GUI tu terminal:

```bash
make gui
```

Khi bam nut can sudo tren browser, quay lai terminal do de nhap password neu duoc hoi.
