#ifndef EXTENSION_GREETER_CONFIG_H
#define EXTENSION_GREETER_CONFIG_H 1

#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit-web-extension.h>

void GreeterConfig_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension);

#endif
