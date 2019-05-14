#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <X11/extensions/Xrender.h>

#include <nuklear.h>
#include <stb_image.h>

#include <ui/display.h>
#include <ui/window.h>

#include "canvas.h"
#include "display.h"
#include "image.h"

extern struct ui_display display;

struct nk_image
ui_alloc_image(unsigned char *data, int width, int height, int channels)
{
	struct nk_image img;
	struct ui_image *image;
	XRenderPictFormat *pic_format;
	Pixmap pixmap;
	GC gc;
	long i;
	long isize = width * height * channels;
	int depth = DefaultDepth(display.dpy, display.screen);
	int format;

	image = calloc(1, sizeof *image);

	if (!data)
		return nk_image_id(0);

	if (!image)
		return nk_image_id(0);

	depth = 32;

	switch (depth) {
	case 32: format = PictStandardARGB32; break;
	case 24: format = PictStandardRGB24; break;
	case 8: format = PictStandardA8; break;
	default: return nk_image_id(0);
	}

	pic_format = XRenderFindStandardFormat(display.dpy, format);

	for (size_t i = 0; i < width * height; ++i) {
		char r = data[i * 4];
		char g = data[i * 4 + 1];
		char b = data[i * 4 + 2];
		char a = data[i * 4 + 3];
		data[i * 4 + 0] = b;
		data[i * 4 + 1] = g;
		data[i * 4 + 2] = r;
		data[i * 4 + 3] = a;
	}

	image->img = XCreateImage(display.dpy, CopyFromParent, depth, ZPixmap,
		0, (char *)data, width, height, 4 * 8, 4 * width);
	pixmap = XCreatePixmap(display.dpy, display.root, width, height, 4 * 8);
	gc = XCreateGC(display.dpy, pixmap, 0, NULL);
	XPutImage(display.dpy, pixmap, gc, image->img, 0, 0, 0, 0, width, height);
	image->pic = XRenderCreatePicture(display.dpy, pixmap, pic_format, 0, NULL);
	XFreeGC(display.dpy, gc);
	XFreePixmap(display.dpy, pixmap);

	img = nk_image_ptr((void *)image);
	img.w = width;
	img.h = height;

	return img;
}

void
ui_free_image(struct nk_image img)
{
	struct ui_image *image = img.handle.ptr;

	XRenderFreePicture(display.dpy, image->pic);
}

struct nk_image
ui_load_image_from_memory(const void *buf, nk_uint size)
{
	unsigned char *data;
	int x, y, n = 4;

	data = stbi_load_from_memory(buf, size, &x, &y, &n, 0);

	return ui_alloc_image(data, x, y, n);
}

struct nk_image
ui_load_image_from_file(char const *path)
{
	unsigned char *data;
	int x, y, n = 4;

	data = stbi_load(path, &x, &y, &n, 0);

	return ui_alloc_image(data, x, y, n);
}

void
ui_draw_image(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, struct nk_image img, struct nk_color color)
{
	struct ui_image *image = img.handle.ptr;

	if (!image)
		return;

	XRenderPictFormat *pformat = XRenderFindStandardFormat(display.dpy, PictStandardRGB24);
	canvas->pic = XRenderCreatePicture(display.dpy, canvas->drawable, pformat, 0, 0);
	XRenderComposite(display.dpy, PictOpOver, image->pic, image->pic, canvas->pic,
		0, 0, 0, 0, x, y, w, h);
	XRenderFreePicture(display.dpy, canvas->pic);
}
