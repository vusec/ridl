#include <stdlib.h>
#include <string.h>

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include "gdiplus.h"

#include <ui/display.h>

#include "canvas.h"
#include "display.h"
#include "events.h"
#include "window.h"

extern struct ui_display display;

LRESULT CALLBACK
ui_handle_event(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
	struct ui_window *window;
	struct ui_canvas *canvas;

	window = (struct ui_window *)GetWindowLongPtr(win, GWLP_USERDATA);

	if (!window)
		return DefWindowProc(win, msg, wparam, lparam);

	canvas = window->canvas;

	switch (msg) {
	case WM_CLOSE: {
		window->should_close = 1;
	} break;
	case WM_SIZE: {
		unsigned width = LOWORD(lparam);
		unsigned height = HIWORD(lparam);
		ui_resize_canvas(window, canvas, width, height);
	} break;
	case WM_PAINT: {
		PAINTSTRUCT paint;
		HDC dc;
		GpGraphics *graphics;

		dc = BeginPaint(win, &paint);
		GdipCreateFromHDC(dc, &graphics);
		ui_blit_canvas(graphics, canvas);
		GdipDeleteGraphics(graphics);
		EndPaint(win, &paint);
	} break;
	case WM_LBUTTONDOWN:
		nk_input_button(&canvas->ctx, NK_BUTTON_LEFT, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
		SetCapture(win);
		return 1;
	case WM_LBUTTONUP:
		nk_input_button(&canvas->ctx, NK_BUTTON_DOUBLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
		nk_input_button(&canvas->ctx, NK_BUTTON_LEFT, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
		ReleaseCapture();
		return 1;
	case WM_RBUTTONDOWN:
		nk_input_button(&canvas->ctx, NK_BUTTON_RIGHT, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
		SetCapture(win);
		return 1;
	case WM_RBUTTONUP:
		nk_input_button(&canvas->ctx, NK_BUTTON_RIGHT, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
		ReleaseCapture();
		return 1;
	case WM_MBUTTONDOWN:
		nk_input_button(&canvas->ctx, NK_BUTTON_MIDDLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
		SetCapture(win);
		return 1;
	case WM_MBUTTONUP:
		nk_input_button(&canvas->ctx, NK_BUTTON_MIDDLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
		ReleaseCapture();
		return 1;
	case WM_MOUSEWHEEL:
		nk_input_scroll(&canvas->ctx, nk_vec2(0,(float)(short)HIWORD(wparam) / WHEEL_DELTA));
		return 1;
	case WM_MOUSEMOVE:
		nk_input_motion(&canvas->ctx, (short)LOWORD(lparam), (short)HIWORD(lparam));
		return 1;
	case WM_LBUTTONDBLCLK:
		nk_input_button(&canvas->ctx, NK_BUTTON_DOUBLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
return 1;
	default: break;
	}

	return DefWindowProc(win, msg, wparam, lparam);
}

void
ui_process_events(void)
{
	MSG msg;

	if (!GetMessage(&msg, NULL, 0, 0))
		return;

	do {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	} while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE));
}
