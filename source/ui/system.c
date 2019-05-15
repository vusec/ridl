#include <stdlib.h>

#include <nuklear.h>

#include <ui/display.h>
#include <ui/font.h>
#include <ui/window.h>

#include <info/cpuid.h>
#include <info/memory.h>
#include <info/microcode.h>
#include <info/os.h>

#include "style.h"
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

void
show_system_tab(struct nk_context *ctx, struct style *style, struct sys_info *info, int flags)
{
	float ratios[] = {0.3f, 0.7f};
	int ret;

	ui_set_font(ctx, style->header_font);
	ret = nk_tree_push(ctx, NK_TREE_TAB, "System", flags);
	ui_set_font(ctx, style->font);

	if (!ret)
		return;

	nk_layout_row(ctx, NK_DYNAMIC, 20, 2, ratios);
	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Operating system:");
	ui_set_font(ctx, style->font);
	nk_label_wrap(ctx, info->os_name);

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Processor:");
	ui_set_font(ctx, style->font);
	nk_label_wrap(ctx, info->cpu_name);

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Microarchitecture:");
	ui_set_font(ctx, style->font);
	nk_label_wrap(ctx, cpuid_get_codename());

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Microcode:");
	ui_set_font(ctx, style->font);
	nk_label_wrap(ctx, info->microcode);

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Memory:");
	ui_set_font(ctx, style->font);
	nk_label_wrap(ctx, info->memory);

	nk_tree_pop(ctx);
}
