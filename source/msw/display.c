#include <stdlib.h>
#include <string.h>

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include "gdiplus.h"

#include <ui/display.h>

#include "display.h"
#include "events.h"

struct ui_display display;

int
ui_open_display(void)
{
	WNDCLASS wc = {0};

	GdiplusStartupInput startup = { 1, NULL, FALSE, TRUE };

	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = ui_handle_event;
	wc.hInstance = GetModuleHandle(0);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"window";

	RegisterClass(&wc);

	GdiplusStartup(&display.token, &startup, NULL);

	return 0;
}

void
ui_close_display(void)
{
	GdiplusShutdown(display.token);
	UnregisterClass(L"window", GetModuleHandle(0));
}
