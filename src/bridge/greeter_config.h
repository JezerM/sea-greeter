#ifndef BRIDGE_GREETER_CONFIG_H
#define BRIDGE_GREETER_CONFIG_H 1

#include <webkit2/webkit2.h>
#include <lightdm-gobject-1/lightdm.h>

void
GreeterConfig_initialize();
void
handle_greeter_config_accessor(
    WebKitWebView *web_view,
    WebKitUserMessage *message);

#endif

