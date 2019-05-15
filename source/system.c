#include <stdlib.h>

#include <info/cpuid.h>
#include <info/memory.h>
#include <info/microcode.h>
#include <info/os.h>

#include "system.h"

int
query_sys_info(struct sys_info *info)
{
	info->cpu_name = cpuid_get_brand_string();
	info->os_name = get_os_name();
	info->microcode = get_microcode();
	info->memory = get_memory_size(0, 2);

	return 0;
}
