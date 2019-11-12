#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include <memkit.h>
#include <mkio.h>

int tlb_fd;

int tlb_open(void)
{
	tlb_fd = open("/dev/memkit/tlb", O_RDONLY);

	if (tlb_fd < 0) {
		return -1;
	}

	return 0;
}

void tlb_close(void)
{
	close(tlb_fd);
}

int tlb_resolve_va(struct ptes *ptes, pid_t pid, void *va)
{
	struct tlb_param param = {
		.pid = pid,
		.base = va,
	};

	if (ioctl(tlb_fd, MKIO_RESOLVE, &param) < 0) {
		return -1;
	}

	ptes->pgd = param.walk.pgd;
	ptes->p4d = param.walk.p4d;
	ptes->pud = param.walk.pud;
	ptes->pmd = param.walk.pmd;
	ptes->pte = param.walk.pte;

	return 0;
}

int tlb_update_va(pid_t pid, void *va, struct ptes *ptes)
{
	struct tlb_param param = {
		.pid = pid,
		.base = va,
	};

	param.walk.pgd = ptes->pgd;
	param.walk.p4d = ptes->p4d;
	param.walk.pud = ptes->pud;
	param.walk.pmd = ptes->pmd;
	param.walk.pte = ptes->pte;

	return ioctl(tlb_fd, MKIO_UPDATE, &param);
}

int tlb_flush_all(void)
{
	struct tlb_param param;

	return ioctl(tlb_fd, MKIO_FLUSH_ALL, &param);
}

int tlb_flush_pid(pid_t pid)
{
	struct tlb_param param = {
		.pid = pid,
	};

	return ioctl(tlb_fd, MKIO_FLUSH_PID, &param);
}

int tlb_flush_range(pid_t pid, void *va, size_t size)
{
	struct tlb_param param = {
		.pid = pid,
		.base = va,
		.size = size,
	};

	return ioctl(tlb_fd, MKIO_FLUSH_RANGE, &param);
}

int tlb_flush_page(pid_t pid, void *va)
{
	struct tlb_param param = {
		.pid = pid,
		.base = va,
	};

	return ioctl(tlb_fd, MKIO_FLUSH_PAGE, &param);
}

