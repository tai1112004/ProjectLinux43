# Module 4 - Character Device Driver

Driver `mychardev` dang ky major 42, 5 minors, buffer 256 byte, ioctl reset/getsize/setsize va wait queue cho blocking read.

## Build

```bash
make
```

## Test thu cong

```bash
sudo insmod mychardev.ko
sudo mknod /dev/mychardev0 c 42 0
sudo mknod /dev/mychardev1 c 42 1
sudo chmod 666 /dev/mychardev0 /dev/mychardev1
./test_app
./test_blocking
sudo rmmod mychardev
```
