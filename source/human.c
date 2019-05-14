#include <stdlib.h>

#include <asprintf.h>

char *
human_file_size(double size, int binary, size_t prec)
{
	size_t i = 0;
	const char *units[] = {
		"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB",
	};
	const char *bunits[] = {
		"B", "kiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB",
	};
	double factor = 1000;
	char *s;

	if (binary)
		factor = 1024;

	while (size > factor) {
		size /= factor;
		++i;
	}

	asprintf(&s, "%.*f %s", prec, size, binary ? bunits[i] : units[i]);

	return s;
}
