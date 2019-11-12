#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <x86intrin.h>

#include <sys/mman.h>

#include <args.h>
#include <memkit.h>
#include <utils.h>

size_t secret_len;
extern int interrupted;

int main(int argc, char **argv)
{
	struct args args;
	char *buffer;

	if (parse_args(&args, argc, argv) < 0) {
		fprintf(stderr, "usage: %s [<options>]\n", argv[0]);
		return -1;
	}

	secret_len = strlen(args.secret);

	/* Set up the SIGINT handler. */
	if (init_sigint() < 0) {
		return -1;
	}

	/* Set up memkit. */
	if (memkit_init() < 0) {
		printf(" [-] unable to set up memkit\n");
		return -1;
	}

	printf(" [+] set up memkit\n");

	/* Allocate a buffer for the victim to use. */
	buffer = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	if (buffer == MAP_FAILED) {
		printf(" [-] unable to allocate the buffer\n");
		goto err_memkit_exit;
	}

	printf(" [+] allocated buffer\n");

	/* Set the cacheability of the buffer. */
	if (set_cacheability(buffer, args.mem_type) < 0) {
		printf(" [-] unable to set the cacheability of the buffer\n");
		goto err_unmap;
	}

	printf(" [+] set the buffer to %s memory\n", mem_type_to_str(args.mem_type));

	while (!interrupted) {
		/* Copy the secret and issue a mfence. */
		memcpy(buffer, args.secret, secret_len);
		_mm_mfence();
	}

	printf(" [+] cleaning up\n");

	munmap(buffer, 4096);
	memkit_exit();

	return -1;

err_unmap:
	munmap(buffer, 4096);
err_memkit_exit:
	memkit_exit();
	return 0;
}

