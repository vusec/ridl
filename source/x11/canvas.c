#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <X11/extensions/Xrender.h>

#include <nuklear.h>

#include <ui/display.h>
#include <ui/window.h>

#include "canvas.h"
#include "font.h"
#include "display.h"

extern struct ui_display display;

unsigned long
color_from_byte(const nk_byte *c)
{
	unsigned long ret = 0;

	ret |= (unsigned long)c[0] << 16;
	ret |= (unsigned long)c[1] << 8;
	ret |= (unsigned long)c[2] << 0;

	return ret;
}

struct ui_canvas *
ui_create_canvas(Drawable target, struct ui_font *font, unsigned width,
	unsigned height)
{
	struct nk_user_font *ufont;
	struct ui_canvas *canvas;
	XRenderPictFormat *pic_format;

	canvas = calloc(1, sizeof *canvas);

	if (!canvas)
		return NULL;

	canvas->width = width;
	canvas->height = height;
	canvas->gc = XCreateGC(display.dpy, display.root, 0, NULL);
	XSetLineAttributes(display.dpy, canvas->gc, 1, LineSolid, CapButt,
		JoinMiter);
	canvas->drawable = XCreatePixmap(display.dpy, target, width, height,
		(unsigned int)DefaultDepth(display.dpy, display.screen));

	ufont = &font->handle;

	nk_init_default(&canvas->ctx, ufont);

	canvas->ftdraw = XftDrawCreate(display.dpy, canvas->drawable,
		display.vis, display.cmap);

	return canvas;
}

void
ui_destroy_canvas(struct ui_canvas *canvas)
{
	XFreePixmap(display.dpy, canvas->drawable);
	XFreeGC(display.dpy, canvas->gc);
	free(canvas);
}

void
ui_invalidate_canvas(struct ui_canvas *canvas)
{
	canvas->ftdraw = XftDrawCreate(display.dpy, canvas->drawable,
		display.vis, display.cmap);
}

void
ui_resize_canvas(struct ui_canvas *canvas, unsigned width, unsigned height)
{
	if (!canvas)
		return;

	if (canvas->width == width && canvas->height == height)
		return;

	canvas->width = width;
	canvas->height = height;

	if (canvas->drawable)
		XFreePixmap(display.dpy, canvas->drawable);

	canvas->drawable = XCreatePixmap(display.dpy, display.root, width, height,
		(unsigned)DefaultDepth(display.dpy, display.screen));
}

void
ui_clear_canvas(struct ui_canvas *canvas, struct nk_color color)
{
	XSetForeground(display.dpy, canvas->gc, color_from_byte(&color.r));
	XFillRectangle(display.dpy, canvas->drawable, canvas->gc, 0, 0,
		canvas->width, canvas->height);
}

void
ui_blit_canvas(Drawable target, struct ui_canvas *canvas, unsigned width,
	unsigned height)
{
	XCopyArea(display.dpy, canvas->drawable, target, canvas->gc, 0, 0,
		width, height, 0, 0);
}

void
ui_scissor_canvas(struct ui_canvas *canvas, float x, float y, float width,
	float height)
{
	XRectangle rect;

	rect.x = (short)(x - 1);
	rect.y = (short)(y - 1);
	rect.width = (unsigned short)(width + 2);
	rect.height = (unsigned short)(height + 2);

	XSetClipRectangles(display.dpy, canvas->gc, 0, 0, &rect, 1, Unsorted);
}

void
ui_stroke_line(struct ui_canvas *canvas, short x0, short y0, short x1, short y1,
	unsigned line_thickness, struct nk_color color)
{
	unsigned long c = color_from_byte(&color.r);

	XSetForeground(display.dpy, canvas->gc, c);
	XSetLineAttributes(display.dpy, canvas->gc, line_thickness, LineSolid,
		CapButt, JoinMiter);
	XDrawLine(display.dpy, canvas->drawable, canvas->gc, (int)x0, (int)y0,
		(int)x1, (int)y1);
	XSetLineAttributes(display.dpy, canvas->gc, 1, LineSolid, CapButt,
		JoinMiter);
}

void
ui_stroke_rect(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, unsigned short r, unsigned short line_thickness,
	struct nk_color color)
{
	unsigned long c = color_from_byte(&color.r);

	XSetForeground(display.dpy, canvas->gc, c);
	XSetLineAttributes(display.dpy, canvas->gc, line_thickness, LineSolid,
		CapButt, JoinMiter);

	if (r == 0) {
		XDrawRectangle(display.dpy, canvas->drawable, canvas->gc, x, y, w, h);
		return;
	}

	{
		short xc = x + r;
		short yc = y + r;
		short wc = (short)(w - 2 * r);
		short hc = (short)(h - 2 * r);

		XDrawLine(display.dpy, canvas->drawable, canvas->gc, xc, y, xc + wc, y);
		XDrawLine(display.dpy, canvas->drawable, canvas->gc, x + w, yc, x + w,
			yc + hc);
		XDrawLine(display.dpy, canvas->drawable, canvas->gc, xc, y + h, xc + wc,
			y + h);
		XDrawLine(display.dpy, canvas->drawable, canvas->gc, x, yc, x, yc + hc);

		XDrawArc(display.dpy, canvas->drawable, canvas->gc, xc + wc - r, y,
			(unsigned)r * 2, (unsigned)r * 2, 0 * 64, 90 * 64);
		XDrawArc(display.dpy, canvas->drawable, canvas->gc, x, y,
			(unsigned)r * 2, (unsigned)r * 2, 90 * 64, 90 * 64);
		XDrawArc(display.dpy, canvas->drawable, canvas->gc, x, yc + hc - r,
			(unsigned)r * 2, (unsigned)2 * r, 180 * 64, 90 * 64);
		XDrawArc(display.dpy, canvas->drawable, canvas->gc, xc + wc - r, yc + hc - r,
			(unsigned)r * 2, (unsigned)2 * r, -90 * 64, 90 * 64);
	}

	XSetLineAttributes(display.dpy, canvas->gc, 1, LineSolid, CapButt, JoinMiter);
}

void
ui_fill_rect(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, unsigned short r, struct nk_color color)
{
	unsigned long c = color_from_byte(&color.r);

	XSetForeground(display.dpy, canvas->gc, c);

	if (r == 0) {
		XFillRectangle(display.dpy, canvas->drawable, canvas->gc, x, y, w, h);
		return;
	}

	{
		short xc = x + r;
		short yc = y + r;
		short wc = (short)(w - 2 * r);
		short hc = (short)(h - 2 * r);

		XPoint pnts[12];
		pnts[0].x = x;
		pnts[0].y = yc;
		pnts[1].x = xc;
		pnts[1].y = yc;
		pnts[2].x = xc;
		pnts[2].y = y;

		pnts[3].x = xc + wc;
		pnts[3].y = y;
		pnts[4].x = xc + wc;
		pnts[4].y = yc;
		pnts[5].x = x + w;
		pnts[5].y = yc;

		pnts[6].x = x + w;
		pnts[6].y = yc + hc;
		pnts[7].x = xc + wc;
		pnts[7].y = yc + hc;
		pnts[8].x = xc + wc;
		pnts[8].y = y + h;

		pnts[9].x = xc;
		pnts[9].y = y + h;
		pnts[10].x = xc;
		pnts[10].y = yc + hc;
		pnts[11].x = x;
		pnts[11].y = yc + hc;

		XFillPolygon(display.dpy, canvas->drawable, canvas->gc, pnts, 12,
			Convex, CoordModeOrigin);
		XFillArc(display.dpy, canvas->drawable, canvas->gc, xc + wc - r, y,
			(unsigned)r * 2, (unsigned)r * 2, 0 * 64, 90 * 64);
		XFillArc(display.dpy, canvas->drawable, canvas->gc, x, y,
			(unsigned)r * 2, (unsigned)r * 2, 90 * 64, 90 * 64);
		XFillArc(display.dpy, canvas->drawable, canvas->gc, x, yc + hc - r,
			(unsigned)r * 2, (unsigned)2 * r, 180 * 64, 90 * 64);
		XFillArc(display.dpy, canvas->drawable, canvas->gc, xc + wc - r,
			yc + hc - r, (unsigned)r * 2, (unsigned)2 * r, -90 * 64, 90 * 64);
	}
}

void
ui_fill_triangle(struct ui_canvas *canvas, short x0, short y0, short x1,
	short y1, short x2, short y2, struct nk_color color)
{
	XPoint pnts[3];
	unsigned long c = color_from_byte(&color.r);

	pnts[0].x = (short)x0;
	pnts[0].y = (short)y0;
	pnts[1].x = (short)x1;
	pnts[1].y = (short)y1;
	pnts[2].x = (short)x2;
	pnts[2].y = (short)y2;

	XSetForeground(display.dpy, canvas->gc, c);
	XFillPolygon(display.dpy, canvas->drawable, canvas->gc, pnts, 3, Convex, CoordModeOrigin);
}

void
ui_stroke_triangle(struct ui_canvas *canvas, short x0, short y0, short x1,
	short y1, short x2, short y2, unsigned short line_thickness,
	struct nk_color color)
{
	unsigned long c = color_from_byte(&color.r);

	XSetForeground(display.dpy, canvas->gc, c);
	XSetLineAttributes(display.dpy, canvas->gc, line_thickness, LineSolid,
		CapButt, JoinMiter);
	XDrawLine(display.dpy, canvas->drawable, canvas->gc, x0, y0, x1, y1);
	XDrawLine(display.dpy, canvas->drawable, canvas->gc, x1, y1, x2, y2);
	XDrawLine(display.dpy, canvas->drawable, canvas->gc, x2, y2, x0, y0);
	XSetLineAttributes(display.dpy, canvas->gc, 1, LineSolid, CapButt, JoinMiter);
}

void
ui_fill_polygon(struct ui_canvas *canvas,  const struct nk_vec2i *pnts, int count,
	struct nk_color color)
{
	int i = 0;
	#define MAX_POINTS 128
	XPoint xpnts[MAX_POINTS];
	unsigned long c = color_from_byte(&color.r);

	XSetForeground(display.dpy, canvas->gc, c);

	for (i = 0; i < count && i < MAX_POINTS; ++i) {
		xpnts[i].x = pnts[i].x;
		xpnts[i].y = pnts[i].y;
	}

	XFillPolygon(display.dpy, canvas->drawable, canvas->gc, xpnts, count, Convex, CoordModeOrigin);
	#undef MAX_POINTS
}

void
ui_stroke_polygon(struct ui_canvas *canvas, const struct nk_vec2i *pnts, int count,
	unsigned short line_thickness, struct nk_color color)
{
	int i = 0;
	unsigned long c = color_from_byte(&color.r);

	XSetForeground(display.dpy, canvas->gc, c);
	XSetLineAttributes(display.dpy, canvas->gc, line_thickness, LineSolid, CapButt, JoinMiter);

	for (i = 1; i < count; ++i) {
		XDrawLine(display.dpy, canvas->drawable, canvas->gc, pnts[i-1].x, pnts[i-1].y, pnts[i].x, pnts[i].y);
	}

	XDrawLine(display.dpy, canvas->drawable, canvas->gc, pnts[count-1].x, pnts[count-1].y, pnts[0].x, pnts[0].y);
	XSetLineAttributes(display.dpy, canvas->gc, 1, LineSolid, CapButt, JoinMiter);
}

void
ui_stroke_polyline(struct ui_canvas *canvas, const struct nk_vec2i *pnts,
	int count, unsigned short line_thickness, struct nk_color color)
{
	int i = 0;
	unsigned long c = color_from_byte(&color.r);

	XSetLineAttributes(display.dpy, canvas->gc, line_thickness, LineSolid, CapButt, JoinMiter);
	XSetForeground(display.dpy, canvas->gc, c);

	for (i = 0; i < count-1; ++i) {
		XDrawLine(display.dpy, canvas->drawable, canvas->gc, pnts[i].x, pnts[i].y, pnts[i+1].x, pnts[i+1].y);
	}

	XSetLineAttributes(display.dpy, canvas->gc, 1, LineSolid, CapButt, JoinMiter);
}

void
ui_fill_circle(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, struct nk_color color)
{
	unsigned long c = color_from_byte(&color.r);

	XSetForeground(display.dpy, canvas->gc, c);
	XFillArc(display.dpy, canvas->drawable, canvas->gc, (int)x, (int)y,
		(unsigned)w, (unsigned)h, 0, 360 * 64);
}

void
ui_stroke_circle(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, unsigned short line_thickness, struct nk_color color)
{
	unsigned long c = color_from_byte(&color.r);

	XSetLineAttributes(display.dpy, canvas->gc, line_thickness, LineSolid,
		CapButt, JoinMiter);
	XSetForeground(display.dpy, canvas->gc, c);
	XDrawArc(display.dpy, canvas->drawable, canvas->gc, (int)x, (int)y,
		(unsigned)w, (unsigned)h, 0, 360 * 64);
	XSetLineAttributes(display.dpy, canvas->gc, 1, LineSolid, CapButt,
		JoinMiter);
}

void
ui_stroke_curve(struct ui_canvas *canvas, struct nk_vec2i p1,
	struct nk_vec2i p2, struct nk_vec2i p3, struct nk_vec2i p4,
	unsigned int num_segments, unsigned short line_thickness,
	struct nk_color color)
{
	unsigned int i_step;
	float t_step;
	struct nk_vec2i last = p1;

	XSetLineAttributes(display.dpy, canvas->gc, line_thickness, LineSolid,
		CapButt, JoinMiter);
	num_segments = NK_MAX(num_segments, 1);
	t_step = 1.0f / (float)num_segments;

	for (i_step = 1; i_step <= num_segments; ++i_step) {
		float t = t_step * (float)i_step;
		float u = 1.0f - t;
		float w1 = u * u * u;
		float w2 = 3 * u * u * t;
		float w3 = 3 * u * t * t;
		float w4 = t * t * t;
		float x = w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x;
		float y = w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y;

		ui_stroke_line(canvas, last.x, last.y, (short)x, (short)y,
			line_thickness, color);

		last.x = (short)x;
		last.y = (short)y;
	}

	XSetLineAttributes(display.dpy, canvas->gc, 1, LineSolid, CapButt,
		JoinMiter);
}

void
ui_draw_text(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, const char *text, int len, struct ui_font *font,
	struct nk_color cbg, struct nk_color cfg)
{
	int tx, ty;
	unsigned long bg = color_from_byte(&cbg.r);
	unsigned long fg = color_from_byte(&cfg.r);

	XSetForeground(display.dpy, canvas->gc, bg);
	XFillRectangle(display.dpy, canvas->drawable, canvas->gc, (int)x, (int)y,
		(unsigned)w, (unsigned)h);

	if (!text || !font || !len)
		return;

	tx = (int)x;
	ty = (int)y + font->ascent;

	{
		XRenderColor xrc;
		XftColor color;

		xrc.red = cfg.r * 257;
		xrc.green = cfg.g * 257;
		xrc.blue = cfg.b * 257;
		xrc.alpha = cfg.a * 257;

		XftColorAllocValue(display.dpy, display.vis, display.cmap, &xrc,
			&color);
		XftDrawString8(canvas->ftdraw, &color, font->ft, tx, ty,
			(FcChar8*)text, len);
	}
}
