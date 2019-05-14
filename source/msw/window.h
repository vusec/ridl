#pragma once

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

#include <nuklear.h>

struct ui_window {
	struct nk_context *ctx;
	struct ui_canvas *canvas;
	HWND win;
	int should_close;
};
