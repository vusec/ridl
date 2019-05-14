#pragma once

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include "gdiplus.h"

#include <nuklear.h>

struct ui_font {
	GpFont *ft;
	struct nk_user_font handle;
	struct ui_canvas *canvas;
};

float
ui_get_text_width(nk_handle handle, float height, const char *text, int len);
