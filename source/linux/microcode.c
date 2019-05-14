#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *
get_microcode(void)
{
	FILE *f;
	char *key = NULL;
	char *value;
	size_t n;

	f = fopen("/proc/cpuinfo", "r");

	if (!f)
		return NULL;

	while (getline(&key, &n, f) >= 0) {
		value = strchr(key, ':');

		if (!value) {
			free(key);
			key = NULL;
			continue;
		}

		*value = '\0';
		value += 2;

		*(key + strcspn(key, "\t")) = '\0';
		*(value + strcspn(value, "\n")) = '\0';

		if (strcmp(key, "microcode") != 0) {
			free(key);
			key = NULL;
			continue;
		}

		value = strdup(value);
		free(key);
		key = NULL;
		break;
	}

	fclose(f);

	return value;
}
