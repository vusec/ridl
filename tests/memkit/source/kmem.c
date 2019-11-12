#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/slab.h>

#include <mkio.h>

static inline uint64_t rdtscp(uint32_t *core_id)
{
	uint64_t rax, rdx;

	asm volatile(
		"rdtscp\n"
		: "=a" (rax), "=d" (rdx), "=c" (*core_id));

	return (rdx << 32) | rax;
}

static int
kmem_open(struct inode *inode, struct file *file)
{
	(void)inode;
	(void)file;

	return 0;
}

static int
kmem_release(struct inode *inode, struct file *file)
{
	(void)inode;
	(void)file;

	return 0;
}

static long
kmem_ioctl(
          struct file *file,
          unsigned num,
          unsigned long param)
{
	struct kmem_param params;
	uint64_t t0;
	uint32_t core_id;
	int ret;

	(void)file;

	ret = copy_from_user(&params, (void *)param, sizeof params);

	if (ret < 0) {
		return ret;
	}

	switch (num) {
	case MKIO_KMALLOC:
		params.addr = kmalloc(params.size, GFP_KERNEL);
		break;
	case MKIO_KFREE:
		kfree(params.addr);
		break;
	case MKIO_VMALLOC:
		params.addr = vmalloc(params.size);
		break;
	case MKIO_VFREE:
		vfree(params.addr);
		break;
	case MKIO_READ:
		ret = copy_to_user(params.buf, params.addr, params.size);
		break;
	case MKIO_WRITE:
		ret = copy_from_user(params.addr, params.buf, params.size);
		break;
	case MKIO_TIMED_LOAD:
		t0 = rdtscp(&core_id);
		*(volatile char *)params.addr;
		params.dt = rdtscp(&core_id) - t0;
		break;
	case MKIO_PREFETCHT0:
		asm volatile("prefetcht0 (%0)" :: "r" (params.addr));
		break;
	case MKIO_PREFETCHT1:
		asm volatile("prefetcht1 (%0)" :: "r" (params.addr));
		break;
	case MKIO_PREFETCHT2:
		asm volatile("prefetcht2 (%0)" :: "r" (params.addr));
		break;
	case MKIO_PREFETCHNTA:
		asm volatile("prefetchnta (%0)" :: "r" (params.addr));
		break;
	case MKIO_CLFLUSH:
		asm volatile("clflush (%0)" :: "r" (params.addr));
		break;
	case MKIO_CLFLUSHOPT:
		asm volatile("clflushopt (%0)" :: "r" (params.addr));
		break;	
	}

	if (ret < 0) {
		return ret;
	}

	ret = copy_to_user((void *)param, &params, sizeof params);

	if (ret < 0) {
		return ret;
	}

	return 0;
}

static struct file_operations fops = {
	.open = kmem_open,
	.release = kmem_release,
	.unlocked_ioctl = kmem_ioctl,
};

static struct miscdevice kmem_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "memkit!kmem",
	.fops = &fops,
	.mode = 0666,
};

int
kmem_init(void)
{
	int ret;

	ret = misc_register(&kmem_dev);

	if (ret != 0) {
		return -1;
	}

	return 0;
}

void
kmem_exit(void)
{
	misc_deregister(&kmem_dev);
}

