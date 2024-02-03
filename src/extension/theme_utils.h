#ifndef BRIDGE_THEME_UTILS_H
#define BRIDGE_THEME_UTILS_H 1

#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit-web-extension.h>

void ThemeUtils_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension);

#endif
