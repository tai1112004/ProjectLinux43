#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include "mychardev.h"

struct my_device_data {
	struct cdev cdev;
	char buffer[MY_BUFFER_SIZE];
	int size;
	atomic_t access;
	wait_queue_head_t wq;
	int data_ready;
};

static struct my_device_data devs[MY_MAX_MINORS];

static int my_open(struct inode *inode, struct file *file)
{
	struct my_device_data *dev = container_of(inode->i_cdev, struct my_device_data, cdev);
	unsigned int minor = iminor(inode);

	if (atomic_cmpxchg(&dev->access, 0, 1) != 0)
		return -EBUSY;

	file->private_data = dev;
	printk(KERN_INFO "[mychardev] open called, minor=%u\n", minor);
	return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
	struct my_device_data *dev = file->private_data;
	atomic_set(&dev->access, 0);
	printk(KERN_INFO "[mychardev] release called, minor=%u\n", iminor(inode));
	return 0;
}

static ssize_t my_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset)
{
	struct my_device_data *dev = file->private_data;
	ssize_t len;

	if (!dev->data_ready && (file->f_flags & O_NONBLOCK))
		return -EAGAIN;
	if (wait_event_interruptible(dev->wq, dev->data_ready != 0))
		return -ERESTARTSYS;
	if (*offset >= dev->size)
		return 0;

	len = min_t(ssize_t, dev->size - *offset, size);
	if (copy_to_user(user_buffer, dev->buffer + *offset, len))
		return -EFAULT;
	*offset += len;
	return len;
}

static ssize_t my_write(struct file *file, const char __user *user_buffer, size_t size, loff_t *offset)
{
	struct my_device_data *dev = file->private_data;
	char tmp[MY_BUFFER_SIZE];
	ssize_t len;
	int i;

	if (*offset >= MY_BUFFER_SIZE)
		return -ENOSPC;

	len = min_t(ssize_t, size, MY_BUFFER_SIZE - (int)*offset);
	if (copy_from_user(dev->buffer + *offset, user_buffer, len))
		return -EFAULT;
	dev->size = *offset + len;
	*offset += len;
	dev->data_ready = 1;
	wake_up_interruptible(&dev->wq);

	/*
	 * Demo wait queue: broadcast data sang cac minor khac de test_blocking
	 * co the mo minor khac va van danh thuc reader dang block.
	 */
	memcpy(tmp, dev->buffer, dev->size);
	for (i = 0; i < MY_MAX_MINORS; i++) {
		if (&devs[i] == dev)
			continue;
		memcpy(devs[i].buffer, tmp, dev->size);
		devs[i].size = dev->size;
		devs[i].data_ready = 1;
		wake_up_interruptible(&devs[i].wq);
	}
	return len;
}

static loff_t my_llseek(struct file *file, loff_t offset, int whence)
{
	struct my_device_data *dev = file->private_data;
	loff_t new_offset;

	switch (whence) {
	case SEEK_SET:
		new_offset = offset;
		break;
	case SEEK_CUR:
		new_offset = file->f_pos + offset;
		break;
	case SEEK_END:
		new_offset = dev->size + offset;
		break;
	default:
		return -EINVAL;
	}

	if (new_offset < 0 || new_offset > MY_BUFFER_SIZE)
		return -EINVAL;

	file->f_pos = new_offset;
	return new_offset;
}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct my_device_data *dev = file->private_data;
	int value;

	switch (cmd) {
	case MY_IOCTL_RESET:
		memset(dev->buffer, 0, sizeof(dev->buffer));
		dev->size = 0;
		dev->data_ready = 0;
		return 0;
	case MY_IOCTL_SETSIZE:
		if (copy_from_user(&value, (int __user *)arg, sizeof(value)))
			return -EFAULT;
		if (value < 0 || value > MY_BUFFER_SIZE)
			return -EINVAL;
		dev->size = value;
		dev->data_ready = value > 0;
		return 0;
	case MY_IOCTL_GETSIZE:
		value = dev->size;
		if (copy_to_user((int __user *)arg, &value, sizeof(value)))
			return -EFAULT;
		return 0;
	default:
		return -ENOTTY;
	}
}

static const struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_release,
	.read = my_read,
	.write = my_write,
	.llseek = my_llseek,
	.unlocked_ioctl = my_ioctl,
};

static int __init my_init(void)
{
	dev_t devno = MKDEV(MY_MAJOR, 0);
	int ret;
	int i;

	ret = register_chrdev_region(devno, MY_MAX_MINORS, DEVICE_NAME);
	if (ret)
		return ret;

	for (i = 0; i < MY_MAX_MINORS; i++) {
		cdev_init(&devs[i].cdev, &my_fops);
		devs[i].cdev.owner = THIS_MODULE;
		init_waitqueue_head(&devs[i].wq);
		atomic_set(&devs[i].access, 0);
		devs[i].size = 0;
		devs[i].data_ready = 0;
		ret = cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), 1);
		if (ret)
			goto err;
	}
	printk(KERN_INFO "[mychardev] registered major=%d minors=%d\n", MY_MAJOR, MY_MAX_MINORS);
	return 0;

err:
	while (--i >= 0)
		cdev_del(&devs[i].cdev);
	unregister_chrdev_region(devno, MY_MAX_MINORS);
	return ret;
}

static void __exit my_exit(void)
{
	int i;

	for (i = 0; i < MY_MAX_MINORS; i++)
		cdev_del(&devs[i].cdev);
	unregister_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS);
	printk(KERN_INFO "[mychardev] unregistered\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux SysProject");
MODULE_DESCRIPTION("Character device driver demo with ioctl and wait queue");
