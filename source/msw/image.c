#include <stdlib.h>
#include <string.h>

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include "gdiplus.h"

#include <ui/display.h>
#include <ui/window.h>

#include <nuklear.h>

#include "canvas.h"
#include "display.h"
#include "image.h"

extern struct ui_display display;

struct nk_image
ui_alloc_image(GpImage *image)
{
	struct nk_image img;
	UINT width, height;

	img = nk_image_ptr((void *)image);

	GdipGetImageWidth(image, &width);
	GdipGetImageHeight(image, &height);

	img.w = width;
	img.h = height;

	return img;
}

struct nk_image
ui_load_image_from_memory(const void *buf, nk_uint size)
{
	GpImage *image;
	GpStatus status;
	IStream *stream;

	stream = SHCreateMemStream((const BYTE *)buf, size);

	if (!stream)
		return nk_image_id(0);

	status = GdipLoadImageFromStream(stream, &image);
	stream->lpVtbl->Release(stream);

	if (status)
		return nk_image_id(0);

	return ui_alloc_image(image);
}

struct nk_image
ui_load_image_from_file(char const *path)
{
	GpImage *image;
	WCHAR *wpath;
	int wlen;
	int len = strlen(path);

	wlen = MultiByteToWideChar(CP_UTF8, 0, path, len, NULL, 0);
	wpath = (WCHAR *)_alloca(wlen * sizeof *wpath);
	MultiByteToWideChar(CP_UTF8, 0, path, len, wpath, wlen);

	if (GdipLoadImageFromFile(wpath, &image))
		return nk_image_id(0);

	return ui_alloc_image(image);
}

void
ui_draw_image(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, struct nk_image img, struct nk_color color)
{
	GpImage *image = img.handle.ptr;

	if (!image)
		return;

	GdipDrawImageRectI(canvas->memory, image, x, y, w, h);
}
