#include <stdio.h>
#include <stdlib.h>

int
get_os_version(int component)
{
	FILE *f;
	char *cmd = NULL;
	char *line = NULL;
	size_t n;
	int ret;

	if (asprintf(&cmd, "sw_vers -productVersion | awk -F '.' '{print $%d}'",
		component) < 0)
		return -1;

	f = popen(cmd, "r");
	free(cmd);

	if (!f)
		return -1;

	ret = getline(&line, &n, f);
	pclose(f);

	if (ret < 0)
		return -1;

	ret = strtol(line, NULL, 10);
	free(line);

	return ret;
}
