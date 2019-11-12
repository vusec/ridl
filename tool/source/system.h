#pragma once

struct sys_info {
	char *cpu_name;
	char *os_name;
	char *microcode;
	char *memory;
};

int
query_sys_info(struct sys_info *info);
