#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>

#include <nuklear.h>

#include <ui/display.h>
#include <ui/window.h>

#include "canvas.h"
#include "display.h"
#include "image.h"
#include "window.h"

extern struct ui_display display;

struct ui_canvas *
ui_get_canvas(struct ui_window *window)
{
	return window->canvas;
}

struct nk_context *
ui_get_context(struct ui_window *window)
{
	return window->ctx;
}

struct ui_window *
ui_create_window(struct ui_font *font, const char *title, int x, int y,
	int width, int height)
{
	XSetWindowAttributes attr;
	struct ui_window *window;

	window = calloc(1, sizeof *window);

	if (!window)
		return NULL;

	attr.colormap = display.cmap;
	attr.event_mask =
		ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPress |
		ButtonReleaseMask | ButtonMotionMask | Button1MotionMask |
		Button3MotionMask | Button4MotionMask | Button5MotionMask |
		PointerMotionMask | KeymapStateMask;

	window->win = XCreateWindow(display.dpy, display.root, x, y, width,
		height, 0, XDefaultDepth(display.dpy, display.screen), InputOutput,
		display.vis, CWEventMask | CWColormap, &attr);

	XStoreName(display.dpy, window->win, title);
	XSetWMProtocols(display.dpy, window->win, &display.wm_delete_window, 1);

	window->canvas = ui_create_canvas(window->win, font, width, height);
	window->ctx = &window->canvas->ctx;

	XSaveContext(display.dpy, window->win, display.ctx, (XPointer)window);

	return window;
}

void
ui_get_window_size(struct ui_window *window, unsigned *width,
	unsigned *height)
{
	XWindowAttributes attr;

	XGetWindowAttributes(display.dpy, window->win, &attr);

	*width = (unsigned)attr.width;
	*height = (unsigned)attr.height;
}

void
ui_show_window(struct ui_window *window)
{
	XMapWindow(display.dpy, window->win);
}

int
ui_should_close(struct ui_window *window)
{
	return window->should_close;
}

void
ui_render_window(struct ui_window *window, struct nk_color color)
{
	const struct nk_command *cmd;
	struct ui_canvas *canvas = window->canvas;

	if (!window->should_update) {
		nk_clear(window->ctx);
		return;
	}

	window->should_update = 0;

	XClearWindow(display.dpy, window->win);
	ui_clear_canvas(window->canvas, color);

	nk_foreach(cmd, window->ctx) {
		switch (cmd->type) {
		case NK_COMMAND_NOP: break;
		case NK_COMMAND_SCISSOR: {
			const struct nk_command_scissor *sub =
				(const struct nk_command_scissor *)cmd;
			ui_scissor_canvas(canvas, sub->x, sub->y, sub->w, sub->h);
		} break;
		case NK_COMMAND_LINE: {
			const struct nk_command_line *sub =
				(const struct nk_command_line *)cmd;
			ui_stroke_line(canvas, sub->begin.x, sub->begin.y, sub->end.x,
				sub->end.y, sub->line_thickness, sub->color);
		} break;
		case NK_COMMAND_RECT: {
			const struct nk_command_rect *sub =
				(const struct nk_command_rect *)cmd;
			ui_stroke_rect(canvas, sub->x, sub->y,
				NK_MAX(sub->w - sub->line_thickness, 0),
				NK_MAX(sub->h - sub->line_thickness, 0),
				(unsigned short)sub->rounding,
				sub->line_thickness, sub->color);
		} break;
		case NK_COMMAND_RECT_FILLED: {
			const struct nk_command_rect_filled *sub =
				(const struct nk_command_rect_filled *)cmd;
			ui_fill_rect(canvas, sub->x, sub->y, sub->w, sub->h,
				(unsigned short)sub->rounding, sub->color);
		} break;
		case NK_COMMAND_CIRCLE: {
			const struct nk_command_circle *sub =
				(const struct nk_command_circle *)cmd;
			ui_stroke_circle(canvas, sub->x, sub->y, sub->w, sub->h,
				sub->line_thickness, sub->color);
		} break;
		case NK_COMMAND_CIRCLE_FILLED: {
			const struct nk_command_circle_filled *sub =
				(const struct nk_command_circle_filled *)cmd;
			ui_fill_circle(canvas, sub->x, sub->y, sub->w, sub->h,
				sub->color);
		} break;
		case NK_COMMAND_TRIANGLE: {
			const struct nk_command_triangle *sub =
				(const struct nk_command_triangle*)cmd;
			ui_stroke_triangle(canvas, sub->a.x, sub->a.y,
				sub->b.x, sub->b.y, sub->c.x, sub->c.y,
				sub->line_thickness, sub->color);
		} break;
		case NK_COMMAND_TRIANGLE_FILLED: {
			const struct nk_command_triangle_filled *sub =
				(const struct nk_command_triangle_filled *)cmd;
			ui_fill_triangle(canvas, sub->a.x, sub->a.y, sub->b.x, sub->b.y,
				sub->c.x, sub->c.y, sub->color);
		} break;
		case NK_COMMAND_POLYGON: {
			const struct nk_command_polygon *pcmd =
				(const struct nk_command_polygon*)cmd;
			ui_stroke_polygon(canvas, pcmd->points, pcmd->point_count,
				pcmd->line_thickness, pcmd->color);
		} break;
		case NK_COMMAND_POLYGON_FILLED: {
			const struct nk_command_polygon_filled *pcmd =
				(const struct nk_command_polygon_filled *)cmd;
			ui_fill_polygon(canvas, pcmd->points, pcmd->point_count,
				pcmd->color);
		} break;
		case NK_COMMAND_POLYLINE: {
			const struct nk_command_polyline *pcmd =
				(const struct nk_command_polyline *)cmd;
			ui_stroke_polyline(canvas, pcmd->points, pcmd->point_count,
				pcmd->line_thickness, pcmd->color);
		} break;
		case NK_COMMAND_TEXT: {
			const struct nk_command_text *sub =
				(const struct nk_command_text*)cmd;
			ui_draw_text(canvas, sub->x, sub->y, sub->w, sub->h,
				(const char *)sub->string, sub->length,
				(struct ui_font *)sub->font->userdata.ptr,
				sub->background, sub->foreground);
		} break;
		case NK_COMMAND_CURVE: {
			const struct nk_command_curve *sub =
				(const struct nk_command_curve *)cmd;
			ui_stroke_curve(canvas, sub->begin, sub->ctrl[0], sub->ctrl[1],
				sub->end, 22, sub->line_thickness, sub->color);
		} break;
		case NK_COMMAND_IMAGE: {
			const struct nk_command_image *sub =
				(const struct nk_command_image *)cmd;
			ui_draw_image(canvas, sub->x, sub->y, sub->w, sub->h, sub->img,
				sub->col);
		} break;
		case NK_COMMAND_RECT_MULTI_COLOR:
		case NK_COMMAND_ARC:
		case NK_COMMAND_ARC_FILLED:
		case NK_COMMAND_CUSTOM:
		default: break;
		}
	}

	nk_clear(window->ctx);
	ui_blit_canvas(window->win, window->canvas,
		window->canvas->width, window->canvas->height);
	XFlush(display.dpy);
}
