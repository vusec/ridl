#pragma once

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

int
vasprintf(char **s, const char *fmt, va_list args);
int
asprintf(char **s, const char *fmt, ...);
