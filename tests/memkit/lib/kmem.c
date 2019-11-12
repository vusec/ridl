#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include <mkio.h>

int kmem_fd;

int kmem_open(void)
{
	kmem_fd = open("/dev/memkit/kmem", O_RDONLY);

	if (kmem_fd < 0) {
		return -1;
	}

	return 0;
}

void kmem_close(void)
{
	close(kmem_fd);
}

void *kmalloc(size_t size)
{
	struct kmem_param param = {
		.size = size,
	};

	if (ioctl(kmem_fd, MKIO_KMALLOC, &param) < 0) {
		return NULL;
	}

	return param.addr;
}

void kfree(void *addr)
{
	struct kmem_param param = {
		.addr = addr,
	};

	ioctl(kmem_fd, MKIO_KFREE, &param);
}

void *vmalloc(size_t size)
{
	struct kmem_param param = {
		.size = size,
	};

	if (ioctl(kmem_fd, MKIO_VMALLOC, &param) < 0) {
		return NULL;
	}

	return param.addr;
}

void vfree(void *addr)
{
	struct kmem_param param = {
		.addr = addr,
	};

	ioctl(kmem_fd, MKIO_VFREE, &param);
}

size_t kmem_read(void *buf, size_t count, void *addr)
{
	struct kmem_param param = {
		.buf = buf,
		.addr = addr,
		.size = count,
	};
	int ret;

	ret = ioctl(kmem_fd, MKIO_READ, &param);

	if (ret < 0) {
		return ret;
	}

	return param.size;
}

size_t kmem_write(const void *buf, size_t count, void *addr)
{
	struct kmem_param param = {
		.buf = (void *)buf,
		.addr = addr,
		.size = count,
	};
	int ret;

	ret = ioctl(kmem_fd, MKIO_WRITE, &param);

	if (ret < 0) {
		return ret;
	}

	return param.size;
}

uint64_t kload(void *addr)
{
	struct kmem_param param = {
		.addr = addr,
	};

	if (ioctl(kmem_fd, MKIO_TIMED_LOAD, &param) < 0) {
		return UINT64_MAX;
	}

	return param.size;
}

void kprefetcht0(void *addr)
{
	struct kmem_param param = {
		.addr = addr,
	};

	ioctl(kmem_fd, MKIO_PREFETCHT0, &param);
}

void kprefetcht1(void *addr)
{
	struct kmem_param param = {
		.addr = addr,
	};

	ioctl(kmem_fd, MKIO_PREFETCHT1, &param);
}

void kprefetcht2(void *addr)
{
	struct kmem_param param = {
		.addr = addr,
	};

	ioctl(kmem_fd, MKIO_PREFETCHT2, &param);
}

void kprefetchnta(void *addr)
{
	struct kmem_param param = {
		.addr = addr,
	};

	ioctl(kmem_fd, MKIO_PREFETCHNTA, &param);
}

void kclflush(void *addr)
{
	struct kmem_param param = {
		.addr = addr,
	};

	ioctl(kmem_fd, MKIO_CLFLUSH, &param);
}

void kclflushopt(void *addr)
{
	struct kmem_param param = {
		.addr = addr,
	};

	ioctl(kmem_fd, MKIO_CLFLUSHOPT, &param);
}

