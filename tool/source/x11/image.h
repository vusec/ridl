#pragma once

struct ui_image {
	XImage *img;
	GC gc;
	Pixmap mask;
	Picture pic;
};

void
ui_draw_image(struct ui_canvas *canvas, short x, short y, unsigned short w,
	unsigned short h, struct nk_image img, struct nk_color color);
