#include <glib-object.h>
#include <glib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <webkit2/webkit-web-extension.h>

#include "utils/utils.h"

typedef struct {
  gboolean received;
  WebKitUserMessage *message;
} IPCMessage;

static void
send_message_cb(GObject *web_page, GAsyncResult *res, gpointer user_data)
{
  IPCMessage *ipc_message = user_data;
  ipc_message->received = true;
  ipc_message->message = webkit_web_page_send_message_to_view_finish(WEBKIT_WEB_PAGE(web_page), res, NULL);
}

/**
 * Sends a message to web_view synchronously
 *
 * @param web_page A WebKitWebPage
 * @param message The WebKitUserMessage to send
 * @Returns The received response from web_view
 */
WebKitUserMessage *
ipc_renderer_send_message_sync(WebKitWebPage *web_page, WebKitUserMessage *message)
{
  IPCMessage *ipc_message = malloc(sizeof *ipc_message);
  ipc_message->received = false;
  ipc_message->message = NULL;

  webkit_web_page_send_message_to_view(web_page, message, NULL, send_message_cb, ipc_message);

  GMainContext *main_context = g_main_context_default();

  while (ipc_message->received == false) {
    g_main_context_iteration(main_context, true);
  }

  WebKitUserMessage *reply = ipc_message->message;
  g_free(ipc_message);

  return reply;
}

/**
 * Sends a message to web_view asynchronously
 * This is just a wrap around webkit_web_page_send_message_to_view
 *
 * @param web_page A WebKitWebPage
 * @param message The WebKitUserMessage to send
 * @param callback A callback
 */
void
ipc_renderer_send_message(WebKitWebPage *web_page, WebKitUserMessage *message, GAsyncReadyCallback callback)
{
  webkit_web_page_send_message_to_view(web_page, message, NULL, callback, NULL);
}

/**
 * Sends a message to web_view synchronously with arguments
 *
 * @param web_page A WebKitWebPage
 * @param jsc_context The JSCContext object
 * @param object The backend object to access
 * @param target The target property/method to call
 * @param arguments A GPtrArray of JSCValue parameters
 * @Returns The received response from web_view
 */
WebKitUserMessage *
ipc_renderer_send_message_sync_with_arguments(
    WebKitWebPage *web_page,
    JSCContext *jsc_context,
    const char *object,
    const char *target,
    GPtrArray *arguments)
{
  if (!JSC_IS_CONTEXT(jsc_context))
    return NULL;
  GVariant *parameters = jsc_parameters_to_g_variant_array(jsc_context, target, arguments);
  WebKitUserMessage *message = webkit_user_message_new(object, parameters);
  WebKitUserMessage *reply = ipc_renderer_send_message_sync(web_page, message);
  return reply;
}

/**
 * Sends a message to web_view with arguments
 *
 * @param web_page A WebKitWebPage
 * @param jsc_context The JSCContext object
 * @param object The backend object to access
 * @param target The target property/method to call
 * @param arguments A GPtrArray of JSCValue parameters
 * @param callback A callback
 * @param callback Extra data
 */
void
ipc_renderer_send_message_with_arguments(
    WebKitWebPage *web_page,
    JSCContext *jsc_context,
    const char *object,
    const char *target,
    GPtrArray *arguments,
    GAsyncReadyCallback callback,
    gpointer user_data)
{

  if (!JSC_IS_CONTEXT(jsc_context))
    return;

  GVariant *parameters = jsc_parameters_to_g_variant_array(jsc_context, target, arguments);
  WebKitUserMessage *message = webkit_user_message_new(object, parameters);
  webkit_web_page_send_message_to_view(web_page, message, NULL, callback, user_data);
}
