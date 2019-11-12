#include <stdlib.h>
#include <string.h>

#include <macros.h>
#include <intrin.h>

#include <bitmap.h>

int
bitmap_alloc(struct bitmap *bmap, size_t nbits)
{
	if (!bmap)
		return -1;

	if (nbits) {
		bmap->nwords = (nbits + BIT_SIZE(*bmap->words) - 1) /
			BIT_SIZE(*bmap->words);
		bmap->words = calloc(bmap->nwords, sizeof *bmap->words);

		if (!bmap->words)
			return -1;
	} else {
		bmap->nwords = 0;
		bmap->words = 0;
	}

	return 0;
}

void
bitmap_free(struct bitmap *bmap)
{
	if (!bmap)
		return;

	if (bmap->words) {
		free(bmap->words);
		bmap->words = NULL;
	}

	bmap->nwords = 0;
}

size_t
bitmap_get_nbits(struct bitmap *bmap)
{
	if (!bmap)
		return 0;

	return bmap->nwords * BIT_SIZE(*bmap->words);
}

int
bitmap_is_set(struct bitmap *bmap, size_t n)
{
	size_t word, bit;

	if (!bmap)
		return 0;

	word = n / BIT_SIZE(*bmap->words);
	bit = n % BIT_SIZE(*bmap->words);

	if (word >= bmap->nwords)
		return 0;

	return !!(bmap->words[word] & (1UL << bit));
}

int
bitmap_set(struct bitmap *bmap, size_t n)
{
	size_t word, bit;

	if (!bmap)
		return -1;

	word = n / BIT_SIZE(*bmap->words);
	bit = n % BIT_SIZE(*bmap->words);

	if (word >= bmap->nwords)
		return -1;

	bmap->words[word] |= (1UL << bit);

	return 0;
}

int
bitmap_clear(struct bitmap *bmap, size_t n)
{
	size_t word, bit;

	if (!bmap)
		return -1;

	word = n / BIT_SIZE(*bmap->words);
	bit = n % BIT_SIZE(*bmap->words);

	if (word >= bmap->nwords)
		return -1;

	bmap->words[word] &= ~(1UL << bit);

	return 0;
}

size_t
bitmap_count(struct bitmap *bmap)
{
	size_t i;
	size_t count = 0;

	for (i = 0; i < bmap->nwords; ++i) {
		count += sys_popcount(bmap->words[i]);
	}

	return count;
}

int
bitmap_and(struct bitmap *bmap, struct bitmap *mask)
{
	size_t i;
	size_t nwords;

	nwords = _MIN(bmap->nwords, mask->nwords);

	for (i = 0; i < nwords; ++i) {
		bmap->words[i] &= mask->words[i];
	}

	return 0;
}

int
bitmap_or(struct bitmap *bmap, struct bitmap *mask)
{
	size_t i;
	size_t nwords;

	nwords = _MIN(bmap->nwords, mask->nwords);

	for (i = 0; i < nwords; ++i) {
		bmap->words[i] |= mask->words[i];
	}

	return 0;
}

int
bitmap_xor(struct bitmap *bmap, struct bitmap *mask)
{
	size_t i;
	size_t nwords;

	nwords = _MIN(bmap->nwords, mask->nwords);

	for (i = 0; i < nwords; ++i) {
		bmap->words[i] ^= mask->words[i];
	}

	return 0;
}
