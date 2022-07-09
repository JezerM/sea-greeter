#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <string.h>
#include <unistd.h>

#include <jsc/jsc.h>
#include <webkit2/webkit2.h>

typedef struct {
  gboolean received;
  WebKitUserMessage *message;
} IPCMessage;

static void
send_message_cb(
    GObject *web_view,
    GAsyncResult *res,
    gpointer user_data)
{
  IPCMessage *ipc_message = user_data;
  ipc_message->received = true;
  ipc_message->message = webkit_web_view_send_message_to_page_finish(
      WEBKIT_WEB_VIEW(web_view),
      res,
      NULL);
}

/**
 * Sends a message to web_page synchronously
 *
 * @param web_page A WebKitWebPage
 * @param message The WebKitUserMessage to send
 * @Returns The received response from web_view
 */
WebKitUserMessage *
ipc_main_send_message_sync(
    WebKitWebView *web_view,
    WebKitUserMessage *message)
{
  IPCMessage *ipc_message = malloc(sizeof *ipc_message);
  ipc_message->received = false;
  ipc_message->message = NULL;

  webkit_web_view_send_message_to_page(web_view, message, NULL, send_message_cb, ipc_message);

  GMainContext *main_context = g_main_context_default();

  while (ipc_message->received == false) {
    g_main_context_iteration(main_context, true);
  }

  WebKitUserMessage *reply = ipc_message->message;
  g_free(ipc_message);

  return reply;
}

/**
 * Sends a message to web_page
 * This is just a wrap around webkit_web_view_send_message_to_page
 *
 * @param web_page A WebKitWebView
 * @param message The WebKitUserMessage to send
 * @param callback A callback
 */
void
ipc_renderer_send_message(
    WebKitWebView *web_view,
    WebKitUserMessage *message,
    GAsyncReadyCallback callback)
{
  webkit_web_view_send_message_to_page(web_view, message, NULL, callback, NULL);
}

WebKitUserMessage *
ipc_main_send_message_sync_with_arguments(
    WebKitWebView *web_view,
    JSCContext *jsc_context,
    const char *object,
    const char *target,
    GPtrArray *arguments)
{
  GVariant *parameters = jsc_parameters_to_g_variant_array(
      jsc_context,
      target,
      arguments);
  WebKitUserMessage *message = webkit_user_message_new(object, parameters);
  WebKitUserMessage *reply = ipc_main_send_message_sync(web_view, message);
  return reply;
}
