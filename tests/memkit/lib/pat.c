#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <memkit.h>
#include <mkio.h>

uint64_t pat_read(void)
{
	uint64_t val;
	ssize_t ret;
	int fd;

	fd = open("/dev/memkit/pat", O_RDONLY);

	if (fd < 0) {
		return 0;
	}

	ret = read(fd, &val, sizeof val);
	close(fd);

	if (ret < sizeof val) {
		return 0;
	}

	return val;
}

int pat_write(uint64_t val)
{
	ssize_t ret;
	int fd;

	fd = open("/dev/memkit/pat", O_WRONLY);

	if (fd < 0) {
		return -1;
	}

	ret = write(fd, &val, sizeof val);
	close(fd);

	if (ret < sizeof val) {
		return -1;
	}

	return 0;
}

unsigned pat_find_type(unsigned mem_type)
{
	uint64_t val = pat_read();
	unsigned found = 0;
	size_t i;

#if defined(__i386__) || defined(__x86_64__)
	for (i = 0; i < 8; ++i) {
		if (((val >> (i * 8)) & 0x7) == mem_type) {
			found |= (1 << i);
		}
	}
#endif

	return found;
}

int pat_get_type(unsigned *mem_type, uint64_t entry)
{
#if defined(__i386__) || defined(__x86_64__)
	*mem_type = (entry >> 3) & 0x7;
#endif

	return 0;
}

int pat_set_type(uint64_t *entry, unsigned mem_type)
{
#if defined(__i386__) || defined(__x86_64__)
	/* Clear the PAT bits. */
	*entry &= ~(0x7 << 3);
	
	/* Set the PAT memory type. */
	*entry |= (mem_type << 3);
#endif

	return 0;
}

const char *mem_types[] = {
	[MEM_UC] = "uncacheable",
	[MEM_WC] = "write-combine",
	[MEM_WT] = "write-through",
	[MEM_WP] = "write-protect",
	[MEM_WB] = "write-back",
	[MEM_UC_MINUS] = "uncacheable minus",
};

const char *mem_type_to_str(unsigned mem_type)
{
	return mem_types[mem_type];
}

