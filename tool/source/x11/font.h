#pragma once

#include <nuklear.h>

struct ui_font {
	int ascent, descent, height;
	XftFont *ft;
	struct nk_user_font handle;
};

float
ui_get_text_width(nk_handle handle, float height, const char *text, int len);
