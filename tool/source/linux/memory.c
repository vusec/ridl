#include <stdlib.h>

#include <sys/sysinfo.h>

#include <human.h>
#include <info/memory.h>

char *
get_memory_size(int binary, size_t prec)
{
	struct sysinfo info;

	sysinfo(&info);

	return human_file_size((double)info.totalram * info.mem_unit, binary,
		prec);
}
