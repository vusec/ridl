#pragma once

struct bitmap {
	unsigned long *words;
	size_t nwords;
};

int
bitmap_alloc(struct bitmap *bmap, size_t nbits);
void
bitmap_free(struct bitmap *bmap);
size_t
bitmap_get_nbits(struct bitmap *bmap);
int
bitmap_is_set(struct bitmap *bmap, size_t n);
int
bitmap_set(struct bitmap *bmap, size_t n);
int
bitmap_clear(struct bitmap *bmap, size_t n);
size_t
bitmap_count(struct bitmap *bmap);
