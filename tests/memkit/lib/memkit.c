#include <stdlib.h>

#include <memkit.h>

int kmem_open(void);
int tlb_open(void);

void kmem_close(void);
void tlb_close(void);

int memkit_init(void)
{
	if (kmem_open() < 0) {
		return -1;
	}

	if (tlb_open() < 0) {
		goto err_close_kmem;
	}

	return 0;

err_close_kmem:
	kmem_close();
	return -1;
}

void memkit_exit(void)
{
	tlb_close();
	kmem_close();
}
