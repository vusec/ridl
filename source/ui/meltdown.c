#include <nuklear.h>

#include <ui/display.h>
#include <ui/font.h>
#include <ui/window.h>

#include <vuln/meltdown.h>

#include "style.h"

void
show_meltdown_tab(struct nk_context *ctx, struct style *style, struct meltdown_info *info, int flags)
{
	float ratios[] = {0.3f, 0.7f};
	int ret;

	ui_set_font(ctx, style->header_font);
	ret = nk_tree_push(ctx, NK_TREE_TAB, "Meltdown", flags);
	ui_set_font(ctx, style->font);

	if (!ret)
		return;

	nk_layout_row(ctx, NK_DYNAMIC, 20, 2, ratios);
	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Status:");
	ui_set_font(ctx, style->font);

	if (info->affected) {
		nk_label_colored_wrap(ctx, "Vulnerable", style->red);
	} else {
		nk_label_colored_wrap(ctx, "Not Affected", style->green);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "KPTI Present:");
	ui_set_font(ctx, style->font);

	if (info->kpti_present) {
		nk_label_colored_wrap(ctx, "Yes", style->green);
	} else {
		nk_label_colored_wrap(ctx, "No", info->affected ? style->red : style->black);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "KPTI Enabled:");
	ui_set_font(ctx, style->font);

	if (info->kpti_enabled) {
		nk_label_colored_wrap(ctx, "Yes", style->green);
	} else {
		nk_label_colored_wrap(ctx, "No", info->affected ? style->red : style->black);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "PCID Accelerated:");
	ui_set_font(ctx, style->font);

	if (info->has_pcid) {
		nk_label_wrap(ctx, "Yes");
	} else {
		nk_label_wrap(ctx, "No");
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "PCID Invalidation:");
	ui_set_font(ctx, style->font);

	if (info->has_pcid) {
		nk_label_wrap(ctx, "Yes");
	} else {
		nk_label_wrap(ctx, "No");
	}

	nk_tree_pop(ctx);
}
