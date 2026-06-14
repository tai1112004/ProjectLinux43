#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define NC "\033[0m"
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define INFO(fmt, ...) printf(YELLOW "[FILE][INFO] " fmt NC "\n", ##__VA_ARGS__)
#define OK(fmt, ...) printf(GREEN "[FILE][OK] " fmt NC "\n", ##__VA_ARGS__)
#define ERR(fmt, ...) fprintf(stderr, RED "[FILE][ERR] " fmt NC "\n", ##__VA_ARGS__)

static void demo_io(void) {
    char tmpl[] = "/tmp/file_ops_demo_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd < 0) { ERR("mkstemp: %s", strerror(errno)); return; }
    unlink(tmpl);
    char buf[1000];
    memset(buf, 'A', sizeof(buf));
    write(fd, buf, sizeof(buf));
    lseek(fd, 500, SEEK_SET);
    char readbuf[33] = {0};
    read(fd, readbuf, 32);
    OK("read 32 bytes from offset 500: %.32s", readbuf);

    char *mem = mmap(NULL, sizeof(buf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem != MAP_FAILED) {
        memcpy(mem, "MMAP-EDIT", 9);
        msync(mem, sizeof(buf), MS_SYNC);
        OK("mmap edited first bytes");
        munmap(mem, sizeof(buf));
    }
    close(fd);
}

static void demo_inotify(void) {
    int fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0) { ERR("inotify_init: %s", strerror(errno)); return; }
    int wd = inotify_add_watch(fd, "/tmp", IN_CREATE | IN_DELETE);
    if (wd < 0) { ERR("watch /tmp: %s", strerror(errno)); close(fd); return; }
    INFO("watching /tmp for 5 seconds; create/delete a file to see events");
    for (int i = 0; i < 50; ++i) {
        char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
        ssize_t len = read(fd, buf, sizeof(buf));
        if (len > 0) {
            for (char *p = buf; p < buf + len; ) {
                struct inotify_event *ev = (struct inotify_event *)p;
                OK("event mask=0x%x name=%s", ev->mask, ev->len ? ev->name : "(none)");
                p += sizeof(*ev) + ev->len;
            }
        }
        usleep(100000);
    }
    inotify_rm_watch(fd, wd);
    close(fd);
}

int main(void) {
    demo_io();
    demo_inotify();
    return 0;
}
