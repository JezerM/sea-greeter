#ifndef BRIDGE_LIGHTDM_H
#define BRIDGE_LIGHTDM_H 1

#include "browser-web-view.h"
#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit2.h>

void LightDM_initialize(void);
void LightDM_destroy(void);
void handle_lightdm_accessor(BrowserWebView *web_view, WebKitUserMessage *message);

#endif
