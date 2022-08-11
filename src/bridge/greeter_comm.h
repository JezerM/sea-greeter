#ifndef BRIDGE_GREETER_COMM_H
#define BRIDGE_GREETER_COMM_H 1

#include <webkit2/webkit2.h>
#include <lightdm-gobject-1/lightdm.h>

void
GreeterComm_initialize();
void
GreeterComm_destroy();
void
handle_greeter_comm_accessor(
    WebKitWebView *web_view,
    WebKitUserMessage *message);

#endif
