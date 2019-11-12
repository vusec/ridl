#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <sys/mman.h>

#include <args.h>
#include <memkit.h>
#include <utils.h>
#include <x86intrin.h>

size_t secret_len;
extern int interrupted;

int main(int argc, char *argv[])
{
	struct args args;
	char *kmem, *leak;

	if (parse_args(&args, argc, argv) < 0) {
		fprintf(stderr, "usage: %s [<options>]\n", argv[0]);
		return -1;
	}

	secret_len = strlen(args.secret);

	/* Set up the SIGINT handler. */
	if (init_sigint() < 0) {
		return -1;
	}

	/* Allocate the leak buffer. */
	leak = calloc(secret_len, sizeof *leak);

	if (!leak) {
		printf(" [-] unable to allocate the leak buffer\n");
		return -1;
	}

	printf(" [+] allocated the leak buffer\n");

	/* Set up memkit. */
	if (memkit_init() < 0) {
		printf(" [-] unable to set up memkit\n");
		goto err_free_leak;
	}

	printf(" [+] set up memkit\n");

	/* Allocate a buffer in the kernel for the victim to use. */
	kmem = vmalloc(4096);

	if (!kmem) {
		fprintf(stderr, " [-] unable to allocate kernel memory\n");
		goto err_memkit_exit;
	}

	printf(" [+] allocated kernel buffer at %p\n", kmem);

	/* Set the cacheability of the buffer. */
	if (set_cacheability(kmem, args.mem_type) < 0) {
		printf(" [-] unable to set the cacheability of the kernel buffer\n");
		goto err_vfree;
	}

	printf(" [+] set the kernel buffer to %s memory\n", mem_type_to_str(args.mem_type));

	/* Write the secret into the kernel buffer. */
	if (kmem_write(args.secret, secret_len, kmem) < secret_len) {
		fprintf(stderr, " [-] unable to write secret to kernel memory\n");
		goto err_vfree;
	}

	printf(" [+] wrote secret \"%s\" to kernel buffer at %p\n", args.secret, kmem);

	while (!interrupted) {
		/* Keep the kernel buffer into the L1d cache. */
		kmem_read(leak, secret_len, kmem);
	}

	printf(" [+] cleaning up\n");

	vfree(kmem);
	memkit_exit();

	return 0;

err_vfree:
	vfree(kmem);
err_memkit_exit:
	memkit_exit();
err_free_leak:
	free(leak);
	return -1;
}

