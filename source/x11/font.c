#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>

#include <asprintf.h>

#include <ui/font.h>

#include <nuklear.h>

#include "display.h"
#include "font.h"

extern struct ui_display display;

struct ui_font *
ui_create_font(const char *name, int size, int flags)
{
	struct ui_font *font;
	char *s = NULL;
	char *fname;

	font = calloc(1, sizeof *font);

	if (!font)
		return NULL;

	asprintf(&s, "%s:pixelsize=%d", name, size);

	if (flags & UI_FONT_BOLD) {
		fname = s;
		s = NULL;
		asprintf(&s, "%s:weight=bold", fname);
		free(fname);
	}

	if (flags & UI_FONT_ITALIC) {
		fname = s;
		s = NULL;
		asprintf(&s, "%s:slant=italic", fname);
		free(fname);
	}

	font->ft = XftFontOpenName(display.dpy, 0, s);
	free(s);

	if (!font->ft)
		goto err_free;

	font->ascent = font->ft->ascent;
	font->descent = font->ft->descent;
	font->height = font->ft->height;

	font->handle.userdata = nk_handle_ptr(font);
	font->handle.height = (float)font->height;
	font->handle.width = ui_get_text_width;

	return font;

err_free:
	free(font);
	return NULL;
}

void
ui_set_font(struct nk_context *ctx, struct ui_font *font)
{
	nk_style_set_font(ctx, &font->handle);
}

float
ui_get_text_width(nk_handle handle, float height, const char *text, int len)
{
	struct ui_font *font = (struct ui_font *)handle.ptr;
	XGlyphInfo info;

	if (!font || !text)
		return 0;

	XftTextExtents8(display.dpy, font->ft, (FcChar8 *)text, len, &info);
	return info.width;
}
