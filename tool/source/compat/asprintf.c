#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
vasprintf(char **s, const char *fmt, va_list args)
{
	va_list tmp_args;
	int ret;

	va_copy(tmp_args, args);
	ret = vsnprintf(NULL, 0, fmt, tmp_args);
	va_end(tmp_args);

	if (ret < 0)
		return ret;

	*s = malloc(ret + 1);

	va_copy(tmp_args, args);
	ret = vsnprintf(*s, ret + 1, fmt, tmp_args);
	va_end(tmp_args);

	return ret;
}

int
asprintf(char **s, const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vasprintf(s, fmt, args);
	va_end(args);

	return ret;
}
