#ifndef BRIDGE_GREETER_CONFIG_H
#define BRIDGE_GREETER_CONFIG_H 1

#include "browser-web-view.h"
#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit2.h>

void GreeterConfig_destroy(void);
void GreeterConfig_initialize(void);
void handle_greeter_config_accessor(BrowserWebView *web_view, WebKitUserMessage *message);

#endif
