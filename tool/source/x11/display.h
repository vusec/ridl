#pragma once

struct ui_display {
	Display *dpy;
	Window root;
	int screen;
	Visual *vis;
	Colormap cmap;
	XContext ctx;
	Atom wm_delete_window;
};
