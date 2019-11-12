#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/version.h>

#include <mkio.h>

static int
pat_open(struct inode *inode, struct file *file)
{
	(void)inode;
	(void)file;

	return 0;
}

static int
pat_release(struct inode *inode, struct file *file)
{
	(void)inode;
	(void)file;

	return 0;
}

static ssize_t pat_read(struct file *file, char __user *buffer,
	size_t count, loff_t *fpos)
{
#if defined(__i386__) || defined(__x86_64__)
	uint32_t lo, hi;
#endif
	uint64_t val;
	int ret;

	if (count < sizeof val) {
		return EINVAL;
	}

#if defined(__i386__) || defined(__x86_64__)
	asm volatile("rdmsr\n"
		: "=a" (lo), "=d" (hi)
		: "c" (0x277));

	val = lo | (((uint64_t)hi) << 32);
#elif defined(__aarch64__)
	asm volatile("mrs %0, mair_el1\n"
		: "=r" (val));
#endif

	ret = copy_to_user(buffer, &val, sizeof val);

	if (ret < 0) {
		return ret;
	}

	return sizeof val;
}

static void
set_pat(void *udata)
{
#if defined(__i386__) || defined(__x86_64__)
	uint32_t lo, hi;
	uint64_t pat = *(uint64_t *)udata;

	lo = pat & 0xffffffff;
	hi = (pat >> 32) & 0xffffffff;

	asm volatile("wrmsr\n"
		:: "a" (lo), "d" (hi), "c" (0x277));
#elif defined(__aarch64__)
	uint64_t pat = *(uint64_t *)pat;

	asm volatile("msr mair_el1, %0\n"
		:: "r" (pat));
#endif
}

static ssize_t pat_write(struct file *file, const char __user *buffer,
	size_t count, loff_t *fpos)
{
	uint64_t val;
	int ret;

	if (count < sizeof val) {
		return EINVAL;
	}

	ret = copy_from_user(&val, buffer, sizeof val);

	if (ret < 0) {
		return ret;
	}

	on_each_cpu(set_pat, &val, 1);

	return sizeof val;
}

static struct file_operations fops = {
	.open = pat_open,
	.release = pat_release,
	.read = pat_read,
	.write = pat_write,
};

static struct miscdevice pat_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "memkit!pat",
	.fops = &fops,
	.mode = 0666,
};

int
pat_init(void)
{
	int ret;

	ret = misc_register(&pat_dev);

	if (ret != 0) {
		return -1;
	}

	return 0;
}

void
pat_exit(void)
{
	misc_deregister(&pat_dev);
}

