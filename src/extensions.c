#include <webkit2/webkit-web-extension.h>

#include "lightdm-extension.h"
#include "logger.h"

#include "extension/greeter_config.h"

guint64 page_id;

static void
web_page_document_loaded(WebKitWebPage *web_page, gpointer user_data)
{
  (void) user_data;
  /*printf("Web Page %lu loaded\n", webkit_web_page_get_id(web_page));*/

  WebKitUserMessage *message = webkit_user_message_new("ready-to-show", NULL);
  webkit_web_page_send_message_to_view(web_page, message, NULL, NULL, NULL);
}

static gboolean
web_page_send_request_cb(
    WebKitWebPage *web_page,
    WebKitURIRequest *request,
    WebKitURIResponse *redirected_response,
    gpointer user_data)
{
  (void) web_page;
  (void) redirected_response;
  (void) user_data;

  const char *uri = webkit_uri_request_get_uri(request);
  char *scheme = g_uri_parse_scheme(uri);

  /*printf("Page %lu URI: %s - %s\n", webkit_web_page_get_id(web_page), uri, scheme);*/

  gboolean not_local_file = g_strcmp0(scheme, "file") != 0;
  gboolean not_data_uri = g_strcmp0(scheme, "data") != 0;
  gboolean not_webg_uri = g_strcmp0(scheme, "web-greeter") != 0;

  gboolean deny_request = not_local_file && not_data_uri && not_webg_uri;

  /*printf("Allow: %d\n", deny_request);*/

  return deny_request;
}

#define WEB_PAGE_LOG() fprintf(stderr, "%s [ %s ] %s %d: %s\n", timestamp, type, source_id, line, message);

static void
web_page_send_console_message_to_view(
    WebKitWebPage *web_page,
    const char *type,
    const char *message,
    const char *source_id,
    guint line)
{
  GVariant *params = g_variant_new("(sssu)", type, message, source_id, line);
  WebKitUserMessage *user_message = webkit_user_message_new("console", params);
  webkit_web_page_send_message_to_view(web_page, user_message, NULL, NULL, NULL);
}

static void
web_page_console_message_sent(WebKitWebPage *web_page, WebKitConsoleMessage *console_message, gpointer user_data)
{
  (void) web_page;
  (void) user_data;
  GDateTime *now = g_date_time_new_now_local();
  char *timestamp = g_date_time_format(now, "%Y-%m-%d %H:%M:%S");

  char *type = "";
  const gchar *message = webkit_console_message_get_text(console_message);
  const gchar *source_id = webkit_console_message_get_source_id(console_message);
  guint line = webkit_console_message_get_line(console_message);

  switch (webkit_console_message_get_level(console_message)) {
    case WEBKIT_CONSOLE_MESSAGE_LEVEL_ERROR:
      type = "ERROR";
      WEB_PAGE_LOG();
      web_page_send_console_message_to_view(web_page, type, message, source_id, line);
      break;
    case WEBKIT_CONSOLE_MESSAGE_LEVEL_WARNING:
      type = "WARNING";
      WEB_PAGE_LOG();
      break;
    default:
      return;
  }
}

static void
web_page_created_callback(WebKitWebExtension *extension, WebKitWebPage *web_page, gpointer user_data)
{
  (void) extension;

  gboolean secure_mode = false;
  g_variant_get(user_data, "(b)", &secure_mode, NULL);

  page_id = webkit_web_page_get_id(web_page);

  g_signal_connect(web_page, "document-loaded", G_CALLBACK(web_page_document_loaded), NULL);

  g_signal_connect(web_page, "console-message-sent", G_CALLBACK(web_page_console_message_sent), NULL);

  if (secure_mode) {
    g_signal_connect(web_page, "send-request", G_CALLBACK(web_page_send_request_cb), NULL);
  }

  /*g_variant_unref(user_data);*/
}

G_MODULE_EXPORT void
webkit_web_extension_initialize_with_user_data(WebKitWebExtension *extension, GVariant *user_data)
{
  g_variant_ref(user_data);

  g_signal_connect(extension, "page-created", G_CALLBACK(web_page_created_callback), user_data);
  web_page_initialize(extension);
}
