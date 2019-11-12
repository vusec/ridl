#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define ITERS_PER_READ 10000
#ifndef ITERS
#define ITERS 250000
#endif

MODULE_LICENSE("GPL");

//char secret[4096];
volatile unsigned char *secret;

static int kvict_read(struct seq_file *file, void *v)
{
        
        //volatile char buffer[4096] __attribute__((aligned(4096)))  ;
        volatile char *buffer = kmalloc(4096, GFP_KERNEL);
	char *src =  "#00#ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz12345678";
	char buf[64];
	memcpy(buf, src, 64);
        size_t n = 0;
	while (n++ < ITERS_PER_READ) {
                int  i;
		for (i = 0; i < ITERS; ++i) {
			memcpy(buffer, buf, 64);
                        //asm volatile("mfence" ::: "memory");
		}
		if (buf[2] == '9') {
			if (buf[1] == '9')
				buf[1] = '0';
			else
				buf[1]++;
			buf[2] = '0';
		} else
			buf[2]++;
	}



	seq_printf(file, "Hello, world! %p\n", buffer);
        printk("My mod!!!\n");
        kfree(buffer);
	return 0;
}

static int kvict_open(struct inode *inode, struct file *file)
{
	return single_open(file, kvict_read, NULL);
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = kvict_open,
	.llseek = seq_lseek,
	.read = seq_read,
	.release = single_release,
};

static int __init kvict_init(void)
{
	if (!proc_create("kvict", 0, NULL, &fops))
		return -1;

    secret = kmalloc(4096, GFP_KERNEL);

	return 0;
}

static void __exit kvict_exit(void)
{
	remove_proc_entry("kvict", NULL);
}

module_init(kvict_init);
module_exit(kvict_exit);
