#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <ui/display.h>

#include "display.h"

struct ui_display display;

int
ui_open_display(void)
{
	display.dpy = XOpenDisplay(NULL);

	if (!display.dpy)
		return -1;

	display.root = DefaultRootWindow(display.dpy);
	display.screen = XDefaultScreen(display.dpy);
	display.vis = XDefaultVisual(display.dpy, display.screen);
	display.cmap = XCreateColormap(display.dpy, display.root, display.vis,
		AllocNone);
	display.ctx = XUniqueContext();
	display.wm_delete_window = XInternAtom(display.dpy, "WM_DELETE_WINDOW", False);

	return 0;
}

void
ui_close_display(void)
{
	XFreeColormap(display.dpy, display.cmap);
	XCloseDisplay(display.dpy);
}
