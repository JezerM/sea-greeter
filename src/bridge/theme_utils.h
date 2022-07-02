#ifndef BRIDGE_THEME_UTILS_H
#define BRIDGE_THEME_UTILS_H 1

#include <webkit2/webkit-web-extension.h>
#include <lightdm-gobject-1/lightdm.h>

void
ThemeUtils_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension
);

#endif
