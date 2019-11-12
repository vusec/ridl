#include <linux/kernel.h>
#include <linux/module.h>

int
devmem_init(void);
void
devmem_exit(void);

int
kmem_init(void);
void
kmem_exit(void);

int
umem_init(void);
void
umem_exit(void);

int
tlb_init(void);
void
tlb_exit(void);

int
pat_init(void);
void
pat_exit(void);

static int
__init
lkm_init(void)
{
	devmem_init();
	umem_init();
	kmem_init();
	tlb_init();
	pat_init();

	return 0;
}

static void
__exit
lkm_exit(void)
{
	pat_exit();
	tlb_exit();
	kmem_exit();
	umem_exit();
	devmem_exit();
}

module_init(lkm_init)
module_exit(lkm_exit)
MODULE_LICENSE("GPL");

