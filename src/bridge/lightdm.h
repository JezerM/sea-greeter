#ifndef BRIDGE_LIGHTDM_H
#define BRIDGE_LIGHTDM_H 1

#include <webkit2/webkit2.h>
#include <lightdm-gobject-1/lightdm.h>

void
LightDM_initialize();
void
LightDM_destroy();
void
handle_lightdm_accessor(
    WebKitWebView *web_view,
    WebKitUserMessage *message);

#endif
