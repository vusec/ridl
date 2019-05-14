#pragma once

struct ui_font;
struct nk_context;

#define UI_FONT_BOLD   (1 << 0)
#define UI_FONT_ITALIC (1 << 1)
#define UI_FONT_ULINE  (1 << 2)

struct ui_font *
ui_create_font(const char *name, int size, int flags);
void
ui_set_font(struct nk_context *ctx, struct ui_font *font);
