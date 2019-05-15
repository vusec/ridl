#pragma once

struct sys_info {
	char *cpu_name;
	char *os_name;
	char *microcode;
	char *memory;
};

int
query_sys_info(struct sys_info *info);
void
show_system_tab(struct nk_context *ctx, struct style *style, struct sys_info *info, int flags);
