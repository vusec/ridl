#pragma once

void
ui_free_image(struct nk_image img);
struct nk_image
ui_load_image_from_memory(const void *buf, nk_uint size);
struct nk_image
ui_load_image_from_file(char const *path);
