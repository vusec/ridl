#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>

#include <ui/display.h>

#include "canvas.h"
#include "display.h"
#include "window.h"

struct ui_display display;

void
ui_process_event(XEvent *ev)
{
	struct ui_window *window;
	struct nk_context *ctx;

	if (XFindContext(display.dpy, ev->xany.window, display.ctx,
		(XPointer *)&window) != 0)
		return;

	ctx = window->ctx;

	XFilterEvent(ev, window->win);

	switch (ev->type) {
	case Expose:
	case ConfigureNotify: {
		unsigned width, height;
		XWindowAttributes attr;
		XGetWindowAttributes(display.dpy, window->win, &attr);
		width = (unsigned)attr.width;
		height = (unsigned)attr.height;
		ui_resize_canvas(window->canvas, width, height);
		ui_invalidate_canvas(window->canvas);
		window->should_update = 1;
	} break;
	case KeymapNotify: {
		XRefreshKeyboardMapping(&ev->xmapping);
	} break;
	case ButtonPress:
	case ButtonRelease: {
		int down = (ev->type == ButtonPress);
		const int x = ev->xbutton.x;
		const int y = ev->xbutton.y;

		if (ev->xbutton.button == Button1) {
			nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
		} else if (ev->xbutton.button == Button2) {
			nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
		} else if (ev->xbutton.button == Button3) {
			nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
		} else if (ev->xbutton.button == Button4) {
			nk_input_scroll(ctx, nk_vec2(0, 1.0f));
		} else if (ev->xbutton.button == Button5) {
			nk_input_scroll(ctx, nk_vec2(0, -1.0f));
		}

		window->should_update = 1;
	} break;
	case MotionNotify: {
		const int x = ev->xmotion.x;
		const int y = ev->xmotion.y;

		nk_input_motion(ctx, x, y);
		window->should_update = 1;
	} break;
	case ClientMessage: {
		window->should_close = 1;
	} break;
	default: break;
	}
}

void
ui_process_events(void)
{
	XEvent ev;

	do {
		XNextEvent(display.dpy, &ev);
		ui_process_event(&ev);
	} while (XPending(display.dpy));
}
