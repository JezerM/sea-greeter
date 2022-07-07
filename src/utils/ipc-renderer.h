#ifndef IPC_RENDERER_H
#define IPC_RENDERER_H 1

#include <webkit2/webkit-web-extension.h>

WebKitUserMessage *
ipc_renderer_send_message_sync(
    WebKitWebPage *web_page,
    WebKitUserMessage *message
);
void
ipc_renderer_send_message(
    WebKitWebPage *web_page,
    WebKitUserMessage *message,
    GAsyncReadyCallback callback
);

#endif
