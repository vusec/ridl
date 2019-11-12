#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/highmem.h>
#include <linux/pfn.h>
#include <linux/io.h>
#include <linux/uio.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

static struct file_operations umem_fops = {
	.owner = THIS_MODULE,
};

struct miscdevice umem_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "memkit!umem",
	.fops = &umem_fops,
	/* Make sure the device is accessible from non-root. */
	.mode = 0666,
};

int
umem_init(void)
{
	int err;

	/* Look up the functions used by /dev/mem. */
	umem_fops.llseek =
		(loff_t (*)(struct file *, loff_t, int))
		kallsyms_lookup_name("memory_lseek");
	umem_fops.read =
		(ssize_t (*)(struct file *, char *, size_t, loff_t *))
		kallsyms_lookup_name("read_mem");
	umem_fops.write =
		(ssize_t (*)(struct file *, const char *, size_t, loff_t *))
		kallsyms_lookup_name("write_mem");
	umem_fops.mmap =
		(int (*)(struct file *, struct vm_area_struct *))
		kallsyms_lookup_name("mmap_mem");
	umem_fops.open =
		(int (*)(struct inode *, struct file *))
		kallsyms_lookup_name("open_mem");

#ifndef CONFIG_MMU
	umem_fops.get_unmapped_area =
		kallsyms_lookup_name("get_unmapped_area_mem");
	umem_fops.mmap_capabilities =
		kallsyms_lookup_name("memory_mmap_capabilities");
#endif

	/* Register the miscellaneous device. */
	err = misc_register(&umem_dev);

	if (err) {
		pr_err("can't misc_register\n");
		return err;
	}

	pr_info("/dev/umem setup\n");
	return 0;
}

void
umem_exit(void)
{
	misc_deregister(&umem_dev);
	pr_info("/dev/umem removed\n");
}

