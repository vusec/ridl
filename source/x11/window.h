#pragma once

#include <nuklear.h>

struct ui_window {
	struct nk_context *ctx;
	struct ui_canvas *canvas;
	Window win;
	int should_close;
	int should_update;
};
