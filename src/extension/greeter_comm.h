#ifndef EXTENSION_GREETER_COMM_H
#define EXTENSION_GREETER_COMM_H 1

#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit-web-extension.h>

void GreeterComm_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension);

#endif
