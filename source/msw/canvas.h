#pragma once

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include "gdiplus.h"

#include <nuklear.h>

struct ui_font;

struct ui_canvas {
	struct nk_context ctx;
	GpGraphics *window, *memory;
	GpImage *bitmap;
	GpPen *pen;
	GpSolidFill *brush;
	GpStringFormat *format;
	unsigned width, height;
};

struct ui_canvas *
ui_create_canvas(HWND target, struct ui_font *font, unsigned width,
	unsigned height);
void
ui_destroy_canvas(struct ui_canvas *canvas);
void
ui_invalidate_canvas(struct ui_canvas *canvas);
void
ui_resize_canvas(struct ui_window *window, struct ui_canvas *canvas, unsigned width, unsigned height);
void
ui_clear_canvas(struct ui_canvas *canvas, struct nk_color color);
void
ui_blit_canvas(GpGraphics *graphics, struct ui_canvas *canvas);
void
ui_scissor_canvas(struct ui_canvas *canvas, float x, float y, float width,
	float height);
void
ui_stroke_line(struct ui_canvas *canvas, short x0, short y0, short x1, short y1,
	unsigned line_thickness, struct nk_color color);
void
ui_stroke_rect(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, unsigned short r, unsigned short line_thickness,
	struct nk_color color);
void
ui_fill_rect(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, unsigned short r, struct nk_color color);
void
ui_fill_triangle(struct ui_canvas *canvas, short x0, short y0, short x1,
	short y1, short x2, short y2, struct nk_color color);
void
ui_stroke_triangle(struct ui_canvas *canvas, short x0, short y0, short x1,
	short y1, short x2, short y2, unsigned short line_thickness,
	struct nk_color color);
void
ui_fill_polygon(struct ui_canvas *canvas,  const struct nk_vec2i *pnts, int count,
	struct nk_color color);
void
ui_stroke_polygon(struct ui_canvas *canvas, const struct nk_vec2i *pnts, int count,
	unsigned short line_thickness, struct nk_color color);
void
ui_stroke_polyline(struct ui_canvas *canvas, const struct nk_vec2i *pnts,
	int count, unsigned short line_thickness, struct nk_color color);
void
ui_fill_circle(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, struct nk_color color);
void
ui_stroke_circle(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, unsigned short line_thickness, struct nk_color color);
void
ui_stroke_curve(struct ui_canvas *canvas, struct nk_vec2i p1,
	struct nk_vec2i p2, struct nk_vec2i p3, struct nk_vec2i p4,
	unsigned int num_segments, unsigned short line_thickness,
	struct nk_color color);
void
ui_draw_text(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, const char *text, int len, struct ui_font *font,
	struct nk_color cbg, struct nk_color cfg);
