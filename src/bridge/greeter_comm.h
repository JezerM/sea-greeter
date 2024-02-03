#ifndef BRIDGE_GREETER_COMM_H
#define BRIDGE_GREETER_COMM_H 1

#include "browser-web-view.h"
#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit2.h>

void GreeterComm_initialize(void);
void GreeterComm_destroy(void);
void handle_greeter_comm_accessor(BrowserWebView *web_view, WebKitUserMessage *message);

#endif
