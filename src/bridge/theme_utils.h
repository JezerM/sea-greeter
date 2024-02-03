#ifndef BRIDGE_THEME_UTILS_H
#define BRIDGE_THEME_UTILS_H 1

#include "browser-web-view.h"
#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit2.h>

void ThemeUtils_initialize(void);
void ThemeUtils_destroy(void);
void handle_theme_utils_accessor(BrowserWebView *web_view, WebKitUserMessage *message);

#endif
