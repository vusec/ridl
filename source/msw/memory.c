#include <stdlib.h>

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

#include <human.h>

#include <info/memory.h>

char *
get_memory_size(int binary, size_t prec)
{
	unsigned long long value;

	GetPhysicallyInstalledSystemMemory(&value);

	return human_file_size((double)value * 1024, binary, prec);
}
