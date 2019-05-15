#include <nuklear.h>

#include <ui/display.h>
#include <ui/font.h>
#include <ui/window.h>

#include <vuln/l1tf.h>

#include "style.h"

void
show_l1tf_tab(struct nk_context *ctx, struct style *style, struct l1tf_info *info, int flags)
{
	float ratios[] = {0.3f, 0.7f};
	int ret;

	ui_set_font(ctx, style->header_font);
	ret = nk_tree_push(ctx, NK_TREE_TAB, "L1 Terminal Fault", flags);
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
	nk_label_wrap(ctx, "L1TF Present:");
	ui_set_font(ctx, style->font);

	if (info->l1tf_present) {
		nk_label_colored_wrap(ctx, "Yes", style->green);
	} else {
		nk_label_colored_wrap(ctx, "No", info->affected ? style->red : style->black);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "PTE Inversion:");
	ui_set_font(ctx, style->font);

	if (info->pte_inv) {
		nk_label_colored_wrap(ctx, "Yes", style->green);
	} else {
		nk_label_colored_wrap(ctx, "No", info->affected ? style->red : style->black);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "SMT:");
	ui_set_font(ctx, style->font);

	if (info->smt_vuln) {
		nk_label_colored_wrap(ctx, "Vulnerable", style->red);
	} else {
		nk_label_colored_wrap(ctx, "Unaffected", info->affected ? style->green : style->black);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "L1d Flush Present:");
	ui_set_font(ctx, style->font);

	if (info->has_l1d_flush) {
		nk_label_wrap(ctx, "Yes");
	} else {
		nk_label_wrap(ctx, "No");
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "L1d Flush:");
	ui_set_font(ctx, style->font);

	switch (info->l1d_flush) {
	case L1D_FLUSH_ALWAYS: nk_label_colored_wrap(ctx, "Always", style->green); break;
	case L1D_FLUSH_COND: nk_label_colored_wrap(ctx, "Conditional", style->green); break;
	case L1D_FLUSH_AVAIL: nk_label_colored_wrap(ctx, "Available", style->yellow); break;
	default: nk_label_colored_wrap(ctx, "Never", info->affected ? style->red : style->black); break;
	}

	nk_tree_pop(ctx);
}
