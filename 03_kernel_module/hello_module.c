#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/timekeeping.h>
#include <linux/utsname.h>

#define PROC_NAME "hello_info"

static struct proc_dir_entry *hello_proc;
static ktime_t load_time;

static int hello_proc_show(struct seq_file *m, void *v)
{
	ktime_t now = ktime_get_boottime();
	s64 loaded_ms = ktime_to_ms(ktime_sub(now, load_time));

	seq_puts(m, "module_name: hello_module\n");
	seq_printf(m, "kernel_version: %s\n", init_uts_ns.name.release);
	seq_printf(m, "uptime_ms: %llu\n", jiffies_to_msecs(jiffies));
	seq_printf(m, "loaded_for_ms: %lld\n", loaded_ms);
	return 0;
}

static int hello_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, hello_proc_show, NULL);
}

static const struct proc_ops hello_proc_ops = {
	.proc_open = hello_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

/*
 * __init bao kernel rang ham nay chi dung luc nap module; vung nho init
 * co the duoc giai phong sau khi nap xong de tiet kiem tai nguyen.
 */
static int __init hello_init(void)
{
	load_time = ktime_get_boottime();
	hello_proc = proc_create(PROC_NAME, 0444, NULL, &hello_proc_ops);
	if (!hello_proc)
		return -ENOMEM;

	printk(KERN_INFO "hello_module: loaded and created /proc/%s\n", PROC_NAME);
	return 0;
}

/*
 * __exit danh dau ham chi can khi unload module. Neu build-in vao kernel,
 * compiler co the loai bo phan exit vi no se khong bao gio duoc goi.
 */
static void __exit hello_exit(void)
{
	proc_remove(hello_proc);
	printk(KERN_INFO "hello_module: unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux SysProject");
MODULE_DESCRIPTION("Hello kernel module with /proc/hello_info");
