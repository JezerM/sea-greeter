#ifndef BRIDGE_THEME_UTILS_H
#define BRIDGE_THEME_UTILS_H 1

#include <webkit2/webkit2.h>
#include <lightdm-gobject-1/lightdm.h>

void
ThemeUtils_initialize();
void
ThemeUtils_destroy();
void
handle_theme_utils_accessor(
    WebKitWebView *web_view,
    WebKitUserMessage *message);

#endif
