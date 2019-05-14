#pragma once

struct ui_font;
struct ui_window;

struct ui_canvas *
ui_get_canvas(struct ui_window *window);
struct nk_context *
ui_get_context(struct ui_window *window);
struct ui_window *
ui_create_window(struct ui_font *font, const char *title, int x, int y,
	int width, int height);
void
ui_get_window_size(struct ui_window *window, unsigned *width,
	unsigned *height);
void
ui_show_window(struct ui_window *window);
int
ui_should_close(struct ui_window *window);
void
ui_render_window(struct ui_window *window, struct nk_color color);
