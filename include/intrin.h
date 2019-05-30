#pragma once

#include <stdint.h>

#if _MSC_VER
static inline size_t
sys_ffs(unsigned value)
{
	unsigned long idx;

	_BitScanForward(&idx, value);

	return idx;
}

static inline size_t
sys_ffs64(unsigned value)
{
	unsigned long idx;

	_BitScanForward64(&idx, value);

	return idx;
}

static inline size_t
sys_fls(unsigned value)
{
	unsigned long idx;

	_BitScanReverse(&idx, value);

	return idx;
}

static inline size_t
sys_fls64(uint64_t value)
{
	unsigned long idx;

	_BitScanReverse64(&idx, value);

	return idx;
}

static inline size_t
sys_popcount(unsigned value)
{
	value -= ((value >> 1) & 0x55555555);
	value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
	return (((value + (value >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

static inline size_t
sys_popcount64(uint64_t value)
{
	return sys_popcount((unsigned)value) +
		sys_popcount((unsigned)(value >> 32));
}

#elif __GNUC__
static inline size_t
sys_ffs(unsigned value)
{
	return value ? __builtin_clz(value) : 0;
}

static inline size_t
sys_ffs64(uint64_t value)
{
	return value ? __builtin_clzll(value) : 0;
}

static inline size_t
sys_fls(unsigned value)
{
	return value ? __builtin_ctz(value) : 0;
}

static inline size_t
sys_fls64(uint64_t value)
{
	return value ? __builtin_ctzll(value) : 0;
}

static inline size_t
sys_popcount(unsigned value)
{
	return __builtin_popcount(value);
}

static inline size_t
sys_popcount64(uint64_t value)
{
	return __builtin_popcountll(value);
}
#endif
