# Linux System Programming Project

Project gom 4 module doc lap cho Ubuntu 22.04 / kernel 5.x-6.x:

1. `01_shell_manager`: Bash terminal manager voi ANSI UI.
2. `02_system_prog`: cac demo C ve process, file, socket va network.
3. `03_kernel_module`: kernel module tao `/proc/hello_info`.
4. `04_char_driver`: character device driver co ioctl va wait queue.
5. `gui_app`: giao dien app local dep hon de dieu khien cac module.

## Build nhanh

```bash
make            # build tat ca module co the build tren may hien tai
make system     # build demo C userspace
make kernel     # build hello kernel module
make driver     # build character driver va test app
make gui        # chay giao dien app local tai http://127.0.0.1:8088
make clean
```

Kernel module/driver can `linux-headers-$(uname -r)` va thuong can quyen `sudo` khi load/unload.
