#include <stdio.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_IMPLEMENTATION
#include <nuklear.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <ui/display.h>
#include <ui/font.h>
#include <ui/image.h>
#include <ui/window.h>

#include <vuln/l1tf.h>
#include <vuln/meltdown.h>
#include <vuln/ridl.h>
#include <vuln/spectre.h>

#include "style.h"

#include "../system.h"

#if defined(_WIN32)
#define FONT_NAME "Segoe UI"
#else
#define FONT_NAME "Vera Sans"
#endif

void
show_system_tab(struct nk_context *ctx, struct style *style, struct sys_info *info, int flags);
void
show_meltdown_tab(struct nk_context *ctx, struct style *style, struct meltdown_info *info, int flags);
void
show_l1tf_tab(struct nk_context *ctx, struct style *style, struct l1tf_info *info, int flags);
void
show_spectre_v1_tab(struct nk_context *ctx, struct style *style, struct spectre_info *info, int flags);
void
show_spectre_v2_tab(struct nk_context *ctx, struct style *style, struct spectre_info *info, int flags);
void
show_ssb_tab(struct nk_context *ctx, struct style *style, struct spectre_info *info, int flags);
void
show_ridl_tab(struct nk_context *ctx, struct style *style, struct ridl_info *info, int flags);

void
set_theme(struct nk_context *ctx)
{
	struct nk_color table[NK_COLOR_COUNT];

	table[NK_COLOR_TEXT] = nk_rgba(70, 70, 70, 255);
	table[NK_COLOR_WINDOW] = nk_rgba(240, 240, 240, 255);
	table[NK_COLOR_HEADER] = nk_rgba(175, 175, 175, 255);
	table[NK_COLOR_BORDER] = nk_rgba(0, 0, 0, 255);
	table[NK_COLOR_BUTTON] = nk_rgba(185, 185, 185, 255);
	table[NK_COLOR_BUTTON_HOVER] = nk_rgba(170, 170, 170, 255);
	table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(160, 160, 160, 255);
	table[NK_COLOR_TOGGLE] = nk_rgba(150, 150, 150, 255);
	table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(120, 120, 120, 255);
	table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(175, 175, 175, 255);
	table[NK_COLOR_SELECT] = nk_rgba(190, 190, 190, 255);
	table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(175, 175, 175, 255);
	table[NK_COLOR_SLIDER] = nk_rgba(190, 190, 190, 255);
	table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(80, 80, 80, 255);
	table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(70, 70, 70, 255);
	table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(60, 60, 60, 255);
	table[NK_COLOR_PROPERTY] = nk_rgba(175, 175, 175, 255);
	table[NK_COLOR_EDIT] = nk_rgba(150, 150, 150, 255);
	table[NK_COLOR_EDIT_CURSOR] = nk_rgba(0, 0, 0, 255);
	table[NK_COLOR_COMBO] = nk_rgba(175, 175, 175, 255);
	table[NK_COLOR_CHART] = nk_rgba(160, 160, 160, 255);
	table[NK_COLOR_CHART_COLOR] = nk_rgba(45, 45, 45, 255);
	table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
	table[NK_COLOR_SCROLLBAR] = nk_rgba(180, 180, 180, 255);
	table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(140, 140, 140, 255);
	table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(150, 150, 150, 255);
	table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(160, 160, 160, 255);
	//table[NK_COLOR_TAB_HEADER] = nk_rgba(180, 180, 180, 255);
	table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 105, 152, 255);

	nk_style_from_table(ctx, table);

	ctx->style.tab.text = nk_rgb(255, 255, 255);
}

int
main(void)
{
	struct style style;
	struct sys_info sys_info;
	struct spectre_info spectre_info;
	struct meltdown_info meltdown_info;
	struct l1tf_info l1tf_info;
	struct ridl_info ridl_info;
	struct ui_window *window;
	struct nk_context *ctx;
	unsigned width, height;

	if (ui_open_display() < 0) {
		fprintf(stderr, "error: unable to open the display\n");
		return -1;
	}

	style.font = ui_create_font(FONT_NAME, 14, 0);
	style.bold_font = ui_create_font(FONT_NAME, 14, UI_FONT_BOLD);
	style.header_font = ui_create_font(FONT_NAME, 14, UI_FONT_BOLD);

	style.red = nk_rgb(160, 44, 44);
	style.green = nk_rgb(102, 136, 0);
	style.yellow = nk_rgb(255, 102, 0);
	style.black = nk_rgb(0, 0, 0);

	window = ui_create_window(style.font, "MDS Tool", 0, 0, 640, 480);

	if (!window)
		return -1;

	ui_show_window(window);

	ctx = ui_get_context(window);
	set_theme(ctx);
	ctx->style.button.rounding = 0;

	query_sys_info(&sys_info);
	query_spectre_info(&spectre_info);
	query_meltdown_info(&meltdown_info);
	query_l1tf_info(&l1tf_info);
	query_ridl_info(&ridl_info);

	while (!ui_should_close(window)) {
		nk_input_begin(ctx);
		ui_process_events();
		nk_input_end(ctx);

		ui_get_window_size(window, &width, &height);

		if (nk_begin(ctx, "", nk_rect(0, 0, (float)width, (float)height + 11), 0)) {
			show_system_tab(ctx, &style, &sys_info, NK_MAXIMIZED);
			show_spectre_v1_tab(ctx, &style, &spectre_info, NK_MAXIMIZED);
			show_spectre_v2_tab(ctx, &style, &spectre_info, NK_MAXIMIZED);
			show_ssb_tab(ctx, &style, &spectre_info, NK_MAXIMIZED);
			show_meltdown_tab(ctx, &style, &meltdown_info, NK_MAXIMIZED);
			show_l1tf_tab(ctx, &style, &l1tf_info, NK_MAXIMIZED);
			show_ridl_tab(ctx, &style, &ridl_info, NK_MAXIMIZED);
		}

		nk_end(ctx);

		ui_render_window(window, nk_rgba(0, 0, 0, 0));
	}

	ui_close_display();

	return 0;
}
