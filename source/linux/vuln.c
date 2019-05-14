#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <asprintf.h>

#include "vuln.h"

int
iter_vuln(struct vuln_iter *iter, const char *name)
{
	FILE *f;
	char *path = NULL;
	size_t n;
	int ret;

	if (asprintf(&path, "/sys/devices/system/cpu/vulnerabilities/%s", name) < 0)
		return -1;

	f = fopen(path, "r");
	free(path);

	if (!f)
		return -1;

	iter->line = NULL;
	ret = getline(&iter->line, &n, f);
	fclose(f);

	if (ret < 0)
		return -1;

	*(strchr(iter->line, '\n')) = '\0';

	iter->next = iter->line;
	iter->key = NULL;
	iter->value = NULL;

	return 0;
}

int
next_vuln(struct vuln_iter *iter)
{
	char *delim;

	if (!iter->next) {
		if (iter->line) {
			free(iter->line);
			iter->line = NULL;
		}

		return -1;
	}

	iter->key = iter->next;
	iter->next = iter->key + strcspn(iter->key, ";,");

	if (*iter->next) {
		*iter->next = '\0';
		++iter->next;
	} else {
		iter->next = NULL;
	}

	delim = strchr(iter->key, ':');

	if (!delim) {
		iter->value = NULL;
		return 0;
	}

	*delim = '\0';
	iter->value = delim + 2;

	return 0;
}
