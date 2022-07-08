#ifndef EXTENSION_LIGHTDM_H
#define EXTENSION_LIGHTDM_H 1

#include <webkit2/webkit-web-extension.h>
#include <lightdm-gobject-1/lightdm.h>

void
LightDM_Test_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension);

#endif
