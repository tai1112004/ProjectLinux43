#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "mychardev.h"

#define NC "\033[0m"
#define GREEN "\033[32m"
#define RED "\033[31m"
#define OK(fmt, ...) printf(GREEN "[TEST][OK] " fmt NC "\n", ##__VA_ARGS__)
#define ERR(fmt, ...) fprintf(stderr, RED "[TEST][ERR] " fmt NC "\n", ##__VA_ARGS__)

int main(void)
{
	int fd = open("/dev/mychardev0", O_RDWR);
	if (fd < 0) { ERR("open: %s", strerror(errno)); return 1; }
	OK("opened /dev/mychardev0");

	const char *msg = "Hello from userspace!";
	if (write(fd, msg, strlen(msg)) < 0) { ERR("write: %s", strerror(errno)); return 1; }
	OK("wrote message");

	lseek(fd, 0, SEEK_SET);
	char buf[256] = {0};
	ssize_t n = read(fd, buf, sizeof(buf) - 1);
	if (n < 0) { ERR("read: %s", strerror(errno)); return 1; }
	OK("read back: %s", buf);

	int size = 0;
	if (ioctl(fd, MY_IOCTL_GETSIZE, &size) == 0) OK("ioctl GETSIZE=%d", size);
	if (ioctl(fd, MY_IOCTL_RESET) == 0) OK("ioctl RESET");
	if (ioctl(fd, MY_IOCTL_GETSIZE, &size) == 0) OK("size after reset=%d", size);
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	n = read(fd, buf, sizeof(buf) - 1);
	if (n < 0 && errno == EAGAIN) OK("read after reset: empty buffer (EAGAIN)");
	close(fd);
	OK("closed");
	return 0;
}
