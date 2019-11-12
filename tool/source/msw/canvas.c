#include <stdlib.h>
#include <string.h>

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include "gdiplus.h"

#include <ui/display.h>
#include <ui/window.h>

#include <nuklear.h>

#include "canvas.h"
#include "font.h"
#include "display.h"
#include "window.h"

extern struct ui_display display;

static ARGB
ui_convert_color(struct nk_color c)
{
	return (c.a << 24) | (c.r << 16) | (c.g << 8) | c.b;
}

struct ui_canvas *
ui_create_canvas(HWND target, struct ui_font *font, unsigned width,
	unsigned height)
{
	struct nk_user_font *ufont;
	struct ui_canvas *canvas;

	canvas = calloc(1, sizeof *canvas);

	if (!canvas)
		return NULL;

	canvas->width = width;
	canvas->height = height;

	GdipCreateFromHWND(target, &canvas->window);
	GdipCreateBitmapFromGraphics(width, height, canvas->window, &canvas->bitmap);
	GdipGetImageGraphicsContext(canvas->bitmap, &canvas->memory);
	GdipCreatePen1(0, 1.0f, UnitPixel, &canvas->pen);
	GdipCreateSolidFill(0, &canvas->brush);
	GdipStringFormatGetGenericTypographic(&canvas->format);
	GdipSetStringFormatFlags(canvas->format,
		StringFormatFlagsNoFitBlackBox |
		StringFormatFlagsMeasureTrailingSpaces |
		StringFormatFlagsNoWrap |
		StringFormatFlagsNoClip);

	font->canvas = canvas;
	ufont = &font->handle;

	nk_init_default(&canvas->ctx, ufont);
	nk_set_user_data(&canvas->ctx, nk_handle_ptr(canvas));

	return canvas;
}

void
ui_destroy_canvas(struct ui_canvas *canvas)
{
	GdipDeleteGraphics(canvas->window);
	GdipDeleteGraphics(canvas->memory);
	GdipDisposeImage(canvas->bitmap);
	GdipDeletePen(canvas->pen);
	GdipDeleteBrush(canvas->brush);
	GdipDeleteStringFormat(canvas->format);
	free(canvas);
}

void
ui_invalidate_canvas(struct ui_canvas *canvas)
{
}

void
ui_resize_canvas(struct ui_window *window, struct ui_canvas *canvas, unsigned width, unsigned height)
{
	if (!canvas)
		return;

	canvas->width = width;
	canvas->height = height;

	GdipDeleteGraphics(canvas->window);
	GdipDeleteGraphics(canvas->memory);
	GdipDisposeImage(canvas->bitmap);
	GdipCreateFromHWND(window->win, &canvas->window);
	GdipCreateBitmapFromGraphics(width, height, canvas->window, &canvas->bitmap);
	GdipGetImageGraphicsContext(canvas->bitmap, &canvas->memory);
}

void
ui_clear_canvas(struct ui_canvas *canvas, struct nk_color color)
{
	GdipGraphicsClear(canvas->memory, ui_convert_color(color));
	GdipSetTextRenderingHint(canvas->memory,
		TextRenderingHintSystemDefault);
	GdipSetSmoothingMode(canvas->memory,
		SmoothingModeDefault);
}

void
ui_blit_canvas(GpGraphics *graphics, struct ui_canvas *canvas)
{
	GdipDrawImageI(graphics, canvas->bitmap, 0, 0);
}

void
ui_scissor_canvas(struct ui_canvas *canvas, float x, float y, float width,
	float height)
{
	GdipSetClipRectI(canvas->memory, (INT)x, (INT)y,
		(INT)(width + 1), (INT)(height + 1), CombineModeReplace);
}

void
ui_stroke_line(struct ui_canvas *canvas, short x0, short y0, short x1, short y1,
	unsigned line_thickness, struct nk_color color)
{
	GdipSetPenWidth(canvas->pen, (REAL)line_thickness);
	GdipSetPenColor(canvas->pen, ui_convert_color(color));
	GdipDrawLineI(canvas->memory, canvas->pen, x0, y0, x1, y1);
}

void
ui_stroke_rect(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, unsigned short r, unsigned short line_thickness,
	struct nk_color color)
{
	GdipSetPenWidth(canvas->pen, (REAL)line_thickness);
	GdipSetPenColor(canvas->pen, ui_convert_color(color));

	if (r == 0) {
		GdipDrawRectangleI(canvas->memory, canvas->pen, x, y, w, h);
	} else {
		INT d = 2 * r;
		GdipDrawArcI(canvas->memory, canvas->pen, x, y, d, d, 180, 90);
		GdipDrawLineI(canvas->memory, canvas->pen, x + r, y, x + w - r, y);
		GdipDrawArcI(canvas->memory, canvas->pen, x + w - d, y, d, d, 270, 90);
		GdipDrawLineI(canvas->memory, canvas->pen, x + w, y + r, x + w, y + h - r);
		GdipDrawArcI(canvas->memory, canvas->pen, x + w - d, y + h - d, d, d, 0, 90);
		GdipDrawLineI(canvas->memory, canvas->pen, x, y + r, x, y + h - r);
		GdipDrawArcI(canvas->memory, canvas->pen, x, y + h - d, d, d, 90, 90);
		GdipDrawLineI(canvas->memory, canvas->pen, x + r, y + h, x + w - r, y + h);
	}
}

void
ui_fill_rect(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, unsigned short r, struct nk_color color)
{
	GdipSetSolidFillColor(canvas->brush, ui_convert_color(color));

	if (r == 0) {
		GdipFillRectangleI(canvas->memory, canvas->brush, x, y, w, h);
	} else {
		INT d = 2 * r;

		GdipFillRectangleI(canvas->memory, canvas->brush, x + r, y, w - d, h);
		GdipFillRectangleI(canvas->memory, canvas->brush, x, y + r, r, h - d);
		GdipFillRectangleI(canvas->memory, canvas->brush, x + w - r, y + r, r, h - d);
		GdipFillPieI(canvas->memory, canvas->brush, x, y, d, d, 180, 90);
		GdipFillPieI(canvas->memory, canvas->brush, x + w - d, y, d, d, 270, 90);
		GdipFillPieI(canvas->memory, canvas->brush, x + w - d, y + h - d, d, d, 0, 90);
		GdipFillPieI(canvas->memory, canvas->brush, x, y + h - d, d, d, 90, 90);
	}
}

void
ui_fill_triangle(struct ui_canvas *canvas, short x0, short y0, short x1,
	short y1, short x2, short y2, struct nk_color color)
{
	POINT points[] = {
		{ x0, y0 },
		{ x1, y1 },
		{ x2, y2 },
	};

	GdipSetSolidFillColor(canvas->brush, ui_convert_color(color));
	GdipFillPolygonI(canvas->memory, canvas->brush, points, 3, FillModeAlternate);
}

void
ui_stroke_triangle(struct ui_canvas *canvas, short x0, short y0, short x1,
	short y1, short x2, short y2, unsigned short line_thickness,
	struct nk_color color)
{
	POINT points[] = {
		{ x0, y0 },
		{ x1, y1 },
		{ x2, y2 },
		{ x0, y0 },
	};

	GdipSetPenWidth(canvas->pen, (REAL)line_thickness);
	GdipSetPenColor(canvas->pen, ui_convert_color(color));
	GdipDrawPolygonI(canvas->memory, canvas->pen, points, 4);
}

void
ui_fill_polygon(struct ui_canvas *canvas,  const struct nk_vec2i *pnts, int count,
	struct nk_color color)
{
	int i = 0;

	#define MAX_POINTS 64
	POINT points[MAX_POINTS];
	GdipSetSolidFillColor(canvas->brush, ui_convert_color(color));

	for (i = 0; i < count && i < MAX_POINTS; ++i) {
		points[i].x = pnts[i].x;
		points[i].y = pnts[i].y;
	}

	GdipFillPolygonI(canvas->memory, canvas->brush, points, i, FillModeAlternate);
	#undef MAX_POINTS
}

void
ui_stroke_polygon(struct ui_canvas *canvas, const struct nk_vec2i *pnts, int count,
	unsigned short line_thickness, struct nk_color color)
{
	GdipSetPenWidth(canvas->pen, (REAL)line_thickness);
	GdipSetPenColor(canvas->pen, ui_convert_color(color));

	if (count > 0) {
		int i;
		for (i = 1; i < count; ++i)
			GdipDrawLineI(canvas->memory, canvas->pen, pnts[i-1].x, pnts[i-1].y, pnts[i].x, pnts[i].y);
		GdipDrawLineI(canvas->memory, canvas->pen, pnts[count-1].x, pnts[count-1].y, pnts[0].x, pnts[0].y);
	}
}

void
ui_stroke_polyline(struct ui_canvas *canvas, const struct nk_vec2i *pnts,
	int count, unsigned short line_thickness, struct nk_color color)
{
	GdipSetPenWidth(canvas->pen, (REAL)line_thickness);
	GdipSetPenColor(canvas->pen, ui_convert_color(color));

	if (count > 0) {
		int i;

		for (i = 1; i < count; ++i) {
			GdipDrawLineI(canvas->memory, canvas->pen, pnts[i-1].x, pnts[i-1].y, pnts[i].x, pnts[i].y);
		}
	}
}

void
ui_fill_circle(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, struct nk_color color)
{
	GdipSetSolidFillColor(canvas->brush, ui_convert_color(color));
	GdipFillEllipseI(canvas->memory, canvas->brush, x, y, w, h);
}

void
ui_stroke_circle(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, unsigned short line_thickness, struct nk_color color)
{
	GdipSetPenWidth(canvas->pen, (REAL)line_thickness);
	GdipSetPenColor(canvas->pen, ui_convert_color(color));
	GdipDrawEllipseI(canvas->memory, canvas->pen, x, y, w, h);
}

void
ui_stroke_curve(struct ui_canvas *canvas, struct nk_vec2i p1,
	struct nk_vec2i p2, struct nk_vec2i p3, struct nk_vec2i p4,
	unsigned int num_segments, unsigned short line_thickness,
	struct nk_color color)
{
	GdipSetPenWidth(canvas->pen, (REAL)line_thickness);
	GdipSetPenColor(canvas->pen, ui_convert_color(color));
	GdipDrawBezierI(canvas->memory, canvas->pen, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
}

void
ui_draw_text(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, const char *text, int len, struct ui_font *font,
	struct nk_color cbg, struct nk_color cfg)
{
	int wsize;
	WCHAR* wstr;
	RectF layout = { (FLOAT)x, (FLOAT)y, (FLOAT)w, (FLOAT)h };

	if(!text || !font || !len) return;

	wsize = MultiByteToWideChar(CP_UTF8, 0, text, len, NULL, 0);
	wstr = (WCHAR*)_alloca(wsize * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, text, len, wstr, wsize);

	GdipSetSolidFillColor(canvas->brush, ui_convert_color(cfg));
	GdipDrawString(canvas->memory, wstr, wsize, font->ft, &layout, canvas->format, canvas->brush);
}
