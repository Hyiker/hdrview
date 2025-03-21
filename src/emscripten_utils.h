#pragma once

void setup_imgui_clipboard();
bool host_is_apple();
bool host_is_safari();

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>

extern "C"
{
    EMSCRIPTEN_KEEPALIVE int hdrview_loadfile(const char *filename, const char *buffer, size_t buffer_size);
    EMSCRIPTEN_KEEPALIVE void hdrview_loadurl(const char *url);
} // extern "C"

#endif
