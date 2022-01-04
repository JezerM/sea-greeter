#include <webkit2/webkit-web-extension.h>
#include "logger.h"

static void
web_page_document_loaded(WebKitWebPage *web_page, gpointer user_data) {
  (void) user_data;
  WebKitUserMessage *message = webkit_user_message_new("ready-to-show", NULL);
  webkit_web_page_send_message_to_view(web_page, message, NULL, NULL, NULL);
}

static void
web_page_created_callback(WebKitWebExtension *extension,
    WebKitWebPage *web_page,
    gpointer user_data)
{
  (void) extension;
  (void) user_data;
  g_signal_connect(web_page, "document-loaded",
      G_CALLBACK(web_page_document_loaded),
      NULL);
}

G_MODULE_EXPORT void
webkit_web_extension_initialize (WebKitWebExtension *extension) {
  g_signal_connect(extension, "page-created",
      G_CALLBACK (web_page_created_callback),
      NULL);
}
