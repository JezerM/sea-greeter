#ifndef EXTENSION_GREETER_COMM_H
#define EXTENSION_GREETER_COMM_H 1

#include <webkit2/webkit-web-extension.h>
#include <lightdm-gobject-1/lightdm.h>

void
GreeterComm_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension);

#endif
