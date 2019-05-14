#pragma once

#include <limits.h>

#define _MAX(x, y) (((x) > (y)) ? (x) : (y))
#define _MIN(x, y) (((x) > (y)) ? (x) : (y))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#define BIT_SIZE(x) ((sizeof(x)) * CHAR_BIT)
#define ROUND_DOWN(x, k) ((x) & ~((k) - 1))
#define ROUND_UP(x, k) ((x + k - 1) & ~((k) - 1))

/* Bit manipulation. */
#define BIT(n) (1 << (n))
#define EXTRACT(x, k, n) (((x) >> (k)) & ((1 << (n)) - 1))
