#include <nuklear.h>

#include <ui/display.h>
#include <ui/font.h>
#include <ui/window.h>

#include <vuln/ridl.h>

#include "style.h"

void
show_ridl_tab(struct nk_context *ctx, struct style *style, struct ridl_info *info, int flags)
{
	float ratios[] = {0.3f, 0.7f};
	int ret;

	ui_set_font(ctx, style->header_font);
	ret = nk_tree_push(ctx, NK_TREE_TAB, "Micro-architectural Data Sampling", flags);
	ui_set_font(ctx, style->font);

	if (!ret)
		return;

	nk_layout_row(ctx, NK_DYNAMIC, 20, 2, ratios);
	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Line Fill Buffers (MFBDS):");
	ui_set_font(ctx, style->font);

	if (info->mfbds) {
		nk_label_colored_wrap(ctx, "Vulnerable", style->red);
	} else {
		nk_label_colored_wrap(ctx, "Not Affected", style->green);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Store Buffers (MSBDS):");
	ui_set_font(ctx, style->font);

	if (info->msbds) {
		nk_label_colored_wrap(ctx, "Vulnerable", style->red);
	} else {
		nk_label_colored_wrap(ctx, "Not Affected", style->green);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Load Ports (MLPDS):");
	ui_set_font(ctx, style->font);

	if (info->mlpds) {
		nk_label_colored_wrap(ctx, "Vulnerable", style->red);
	} else {
		nk_label_colored_wrap(ctx, "Not Affected", style->green);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Uncached Memory (MDSUM)");
	ui_set_font(ctx, style->font);

	if (info->mdsum) {
		nk_label_colored_wrap(ctx, "Vulnerable", style->red);
	} else {
		nk_label_colored_wrap(ctx, "Not Affected", style->green);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "SMT:");
	ui_set_font(ctx, style->font);

	if ((info->mfbds || info->msbds || info->mlpds) && info->smt_vuln) {
		nk_label_colored_wrap(ctx, "Vulnerable", style->red);
	} else {
		nk_label_colored_wrap(ctx, "Unaffected",
			(info->mfbds || info->msbds || info->mlpds) ? style->green : style->black);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "MD_CLEAR:");
	ui_set_font(ctx, style->font);

	if (info->mfbds || info->msbds || info->mlpds) {
		if (info->md_clear) {
			nk_label_colored_wrap(ctx, "Available", style->green);
		} else {
			nk_label_colored_wrap(ctx, "Not Available", style->red);
		}
	} else {
		nk_label_colored_wrap(ctx, "Not Required", style->green);
	}

	nk_tree_pop(ctx);
}
