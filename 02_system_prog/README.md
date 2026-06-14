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
- `socket_demo`: TCP echo server/client tren port 9090, server dung `select`.
- `network_info`: `getifaddrs`, MAC qua `ioctl`, thong ke `/proc/net/dev`.
- `main_demo`: menu goi cac binary da build.
