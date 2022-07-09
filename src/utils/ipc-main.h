#ifndef IPC_MAIN_H
#define IPC_MAIN_H 1

#include <webkit2/webkit2.h>

WebKitUserMessage *
ipc_renderer_send_message_sync(
    WebKitWebView *web_view,
    WebKitUserMessage *message
);
void
ipc_renderer_send_message(
    WebKitWebView *web_view,
    WebKitUserMessage *message,
    GAsyncReadyCallback callback
);
WebKitUserMessage *
ipc_renderer_send_message_sync_with_arguments(
    WebKitWebView *web_view,
    JSCContext *jsc_context,
    const char *object,
    const char *target,
    GPtrArray *arguments
);

#endif
