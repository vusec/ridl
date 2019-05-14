#include <nuklear.h>

#include <ui/display.h>
#include <ui/font.h>
#include <ui/window.h>

#include <vuln/spectre.h>

#include "style.h"

void
show_spectre_v1_tab(struct nk_context *ctx, struct style *style, struct spectre_info *info, int flags)
{
	float ratios[] = {0.3f, 0.7f};
	int ret;

	ui_set_font(ctx, style->header_font);
	ret = nk_tree_push(ctx, NK_TREE_TAB, "Direct Branch Speculation", flags);
	ui_set_font(ctx, style->font);

	if (!ret)
		return;

	nk_layout_row(ctx, NK_DYNAMIC, 20, 2, ratios);
	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Status:");
	ui_set_font(ctx, style->font);

	if (info->v1_affected) {
		nk_label_colored_wrap(ctx, "Vulnerable", style->red);
	} else {
		nk_label_colored_wrap(ctx, "Not Affected", style->green);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "__user pointer sanitization:");
	ui_set_font(ctx, style->font);

	if (info->uptr_san) {
		nk_label_colored_wrap(ctx, "Enabled", style->green);
	} else {
		nk_label_colored_wrap(ctx, "Disabled", info->v1_affected ? style->red : style->black);
	}

	nk_tree_pop(ctx);
}

void
show_spectre_v2_tab(struct nk_context *ctx, struct style *style, struct spectre_info *info, int flags)
{
	float ratios[] = {0.3f, 0.7f};
	int ret;

	ui_set_font(ctx, style->header_font);
	ret = nk_tree_push(ctx, NK_TREE_TAB, "Indirect Branch Speculation", flags);
	ui_set_font(ctx, style->font);

	if (!ret)
		return;

	nk_layout_row(ctx, NK_DYNAMIC, 20, 2, ratios);
	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Status:");
	ui_set_font(ctx, style->font);

	if (info->v2_affected) {
		nk_label_colored_wrap(ctx, "Vulnerable", style->red);
	} else {
		nk_label_colored_wrap(ctx, "Not Affected", style->green);
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "Retpoline:");
	ui_set_font(ctx, style->font);

	switch (info->retpol) {
	case RETPOL_ASM: nk_label_colored_wrap(ctx, "Assembly", style->green); break;
	case RETPOL_FULL: nk_label_colored_wrap(ctx, "Full", style->green); break;
	default: nk_label_colored_wrap(ctx, "Disabled", info->v2_affected ? style->red : style->black); break;
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "IBPB:");
	ui_set_font(ctx, style->font);

	switch (info->ibpb) {
	case IBPB_ALWAYS: nk_label_colored_wrap(ctx, "Always", style->green); break;
	case IBPB_COND: nk_label_colored_wrap(ctx, "Conditional", style->green); break;
	case IBPB_PRESENT: nk_label_colored_wrap(ctx, "Disabled", info->v2_affected ? style->red : style->black); break;
	default: nk_label_colored_wrap(ctx, "Not Available", style->black); break;
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "IBRS:");
	ui_set_font(ctx, style->font);

	switch (info->ibrs) {
	case IBRS_ENABLED: nk_label_colored_wrap(ctx, "Enabled", style->green); break;
	case IBRS_PRESENT: nk_label_colored_wrap(ctx, "Disabled", info->v2_affected ? style->red : style->black); break;
	default: nk_label_colored_wrap(ctx, "Not Available", style->black); break;
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "STIBP:");
	ui_set_font(ctx, style->font);

	/* TODO: only useful if SMT is actually present. */
	switch (info->stibp) {
	case STIBP_ENABLED: nk_label_colored_wrap(ctx, "Enabled", style->green); break;
	case STIBP_PRESENT: nk_label_colored_wrap(ctx, "Disabled", info->v2_affected ? style->red : style->black); break;
	default: nk_label_colored_wrap(ctx, "Not Available", style->black); break;
	}

	ui_set_font(ctx, style->bold_font);
	nk_label_wrap(ctx, "SMEP:");
	ui_set_font(ctx, style->font);

	switch (info->smep) {
	case SMEP_ENABLED: nk_label_colored_wrap(ctx, "Enabled", style->green); break;
	case SMEP_PRESENT: nk_label_colored_wrap(ctx, "Disabled", info->v2_affected ? style->red : style->black); break;
	default: nk_label_colored_wrap(ctx, "Not Available", style->black); break;
	}

	nk_tree_pop(ctx);
}
