#include "lightdm-extension.h"
#include "logger.h"
#include <webkit2/webkit-web-extension.h>

guint64 page_id;

static void
web_page_document_loaded(WebKitWebPage *web_page, gpointer user_data)
{
  (void) user_data;
  WebKitUserMessage *message = webkit_user_message_new("ready-to-show", NULL);
  webkit_web_page_send_message_to_view(web_page, message, NULL, NULL, NULL);
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
    case WEBKIT_CONSOLE_MESSAGE_LEVEL_DEBUG:
      type = "DEBUG";
      break;
    case WEBKIT_CONSOLE_MESSAGE_LEVEL_ERROR:
      type = "ERROR";
      break;
    case WEBKIT_CONSOLE_MESSAGE_LEVEL_INFO:
      type = "INFO";
      break;
    case WEBKIT_CONSOLE_MESSAGE_LEVEL_WARNING:
      type = "WARNING";
      break;
    case WEBKIT_CONSOLE_MESSAGE_LEVEL_LOG:
      type = "LOG";
      break;
  }

  fprintf(stderr, "%s [ %s ] %s %d: %s\n", timestamp, type, source_id, line, message);
}

static void
web_page_created_callback(WebKitWebExtension *extension, WebKitWebPage *web_page, gpointer user_data)
{
  (void) extension;
  (void) user_data;
  page_id = webkit_web_page_get_id(web_page);

  g_signal_connect(web_page, "document-loaded", G_CALLBACK(web_page_document_loaded), NULL);

  g_signal_connect(web_page, "console-message-sent", G_CALLBACK(web_page_console_message_sent), NULL);
}

G_MODULE_EXPORT void
webkit_web_extension_initialize(WebKitWebExtension *extension)
{
  g_signal_connect(extension, "page-created", G_CALLBACK(web_page_created_callback), NULL);
  web_page_initialize(extension);
}
