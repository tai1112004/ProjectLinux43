#ifndef MYCHARDEV_H
#define MYCHARDEV_H

#include <linux/ioctl.h>

#define MY_MAJOR 42
#define MY_MAX_MINORS 5
#define DEVICE_NAME "mychardev"
#define MY_BUFFER_SIZE 256

#define MY_IOCTL_RESET   _IO('k', 1)
#define MY_IOCTL_SETSIZE _IOW('k', 2, int)
#define MY_IOCTL_GETSIZE _IOR('k', 3, int)

#endif
