#include <stdlib.h>
#include <string.h>

#include <asprintf.h>

#include <sys/utsname.h>

#include <info/os.h>

char *
get_os_name(void)
{
	struct utsname name;
	char *s = NULL;

	uname(&name);

	if (asprintf(&s, "%s %s", name.sysname, name.release) < 0)
		return NULL;

	return s;
}
