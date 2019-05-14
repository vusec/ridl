#include <nuklear.h>

#include <ui/display.h>
#include <ui/font.h>
#include <ui/window.h>

#include <vuln/ssb.h>

#include "style.h"

void
show_ssb_tab(struct nk_context *ctx, struct style *style, struct ssb_info *info, int flags)
{
	float ratios[] = {0.3f, 0.7f};
	int ret;

	ui_set_font(ctx, style->header_font);
	ret = nk_tree_push(ctx, NK_TREE_TAB, "Speculative Store Bypass", flags);
	ui_set_font(ctx, style->font);

	if (!ret)
		return;

	nk_layout_row(ctx, NK_DYNAMIC, 20, 2, ratios);
	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Speculative Store Bypass");
	ui_set_font(ctx, style->font);

	if (info->affected) {
		nk_label_colored_wrap(ctx, "Vulnerable", style->red);
	} else {
		nk_label_colored_wrap(ctx, "Not Affected", style->green);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Speculative Store Bypass Disable:");
	ui_set_font(ctx, style->font);

	if (info->affected) {
		switch (info->ssbd) {
		case SSBD_OS_SUPPORT: nk_label_colored_wrap(ctx, "OS Support", style->green); break;
		case SSBD_PRESENT: nk_label_colored_wrap(ctx, "Available", style->yellow); break;
		default: nk_label_colored_wrap(ctx, "Not Available", style->red);
		}
	} else {
		nk_label_colored_wrap(ctx, "Not Required", style->green);
	}

	nk_tree_pop(ctx);
}
