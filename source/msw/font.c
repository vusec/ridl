#include <stdlib.h>
#include <string.h>

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include "gdiplus.h"

#include <ui/font.h>

#include <nuklear.h>

#include "canvas.h"
#include "display.h"
#include "font.h"

extern struct ui_display display;

struct ui_font *
ui_create_font(const char *name, int size, int flags)
{
	struct ui_font *font;
	GpFontFamily *family;
	WCHAR *wname;
	int wlen;
	enum FontStyle style;

	font = calloc(1, sizeof *font);

	if (!font)
		return NULL;

	wlen = MultiByteToWideChar(CP_UTF8, 0, name, -1, NULL, 0);
	wname = (WCHAR *)_alloca((wlen + 1) * sizeof *wname);
	MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, wlen);
	wname[wlen] = 0;

	if ((flags & (UI_FONT_BOLD | UI_FONT_ITALIC)) ==
		(UI_FONT_BOLD | UI_FONT_ITALIC)) {
		style = FontStyleBoldItalic;
	} else if (flags & UI_FONT_BOLD) {
		style = FontStyleBold;
	} else if (flags & UI_FONT_ITALIC) {
		style = FontStyleItalic;
	} else if (flags & UI_FONT_ULINE) {
		style = FontStyleUnderline;
	} else {
		style = FontStyleRegular;
	}

	GdipCreateFontFamilyFromName(wname, NULL, &family);
	GdipCreateFont(family, (REAL)size, style, UnitPixel,
		&font->ft);
	GdipDeleteFontFamily(family);

	font->handle.userdata = nk_handle_ptr(font);
	GdipGetFontSize(font->ft, &font->handle.height);
	font->handle.width = ui_get_text_width;

	return font;
}

void
ui_set_font(struct nk_context *ctx, struct ui_font *font)
{
	struct ui_canvas *canvas = ctx->userdata.ptr;

	font->canvas = canvas;

	nk_style_set_font(ctx, &font->handle);
}

float
ui_get_text_width(nk_handle handle, float height, const char *text, int len)
{
	struct ui_font *font = (struct ui_font *)handle.ptr;
	struct ui_canvas *canvas = font->canvas;
	RectF layout = { 0.0f, 0.0f, 65536.0f, 65536.0f };
	RectF bbox;
	WCHAR *wstr;
	int wlen;

	if (!font || !text)
		return 0;

	(void)height;
	wlen = MultiByteToWideChar(CP_UTF8, 0, text, len, NULL, 0);
	wstr = (WCHAR *)_alloca(wlen * sizeof *wstr);
	MultiByteToWideChar(CP_UTF8, 0, text, len, wstr, wlen);

	GdipMeasureString(canvas->memory, wstr, wlen, font->ft, &layout,
		canvas->format, &bbox, NULL, NULL);

	return bbox.Width;
}
