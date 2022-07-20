#include <webkit2/webkit2.h>

#include "bridge/greeter_config.h"
#include "bridge/lightdm.h"
#include "bridge/theme_utils.h"
#include "browser-web-view.h"
#include "logger.h"

struct _BrowserWebView {
  WebKitWebView parent_instance;
};
typedef struct {
  gboolean loaded;
} BrowserWebViewPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(BrowserWebView, browser_web_view, WEBKIT_TYPE_WEB_VIEW)

static void
browser_web_view_class_init(BrowserWebViewClass *klass)
{
  (void) klass;
}
static void
browser_web_view_init(BrowserWebView *self)
{
  BrowserWebViewPrivate *priv = browser_web_view_get_instance_private(self);
  priv->loaded = false;
}

/*
 * Callback to be executed when a web-view user message is received
 */
static void
browser_web_view_user_message_received_cb(BrowserWebView *web_view, WebKitUserMessage *message, gpointer user_data)
{
  (void) user_data;
  const char *name = webkit_user_message_get_name(message);

  if (g_strcmp0(name, "ready-to-show") == 0) {
    BrowserWebViewPrivate *priv = browser_web_view_get_instance_private(web_view);
    if (priv->loaded)
      return;

    gtk_widget_grab_focus(GTK_WIDGET(web_view));

    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(web_view));
    gtk_widget_show_all(window);
    priv->loaded = true;
    logger_debug("Sea greeter started win: %d", gtk_application_window_get_id(GTK_APPLICATION_WINDOW(window)));
    return;
  }

  handle_lightdm_accessor(WEBKIT_WEB_VIEW(web_view), message);
  handle_greeter_config_accessor(WEBKIT_WEB_VIEW(web_view), message);
  handle_theme_utils_accessor(WEBKIT_WEB_VIEW(web_view), message);
}
static gboolean
browser_web_view_context_menu_cb(
    WebKitWebView *webView,
    WebKitContextMenu *context_menu,
    GdkEvent *event,
    WebKitHitTestResult *hit_test_result,
    gpointer user_data)
{
  (void) webView;
  (void) context_menu;
  (void) event;
  (void) hit_test_result;
  (void) user_data;
  return false;
}

/*
 * Enable/disable developer tools or web inspector
 */
void
browser_web_view_set_developer_tools(BrowserWebView *web_view, gboolean value)
{
  WebKitSettings *settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(web_view));
  g_object_set(G_OBJECT(settings), "enable-developer-extras", value, NULL);

  WebKitWebInspector *inspector = webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(web_view));
  if (value) {
    webkit_web_inspector_attach(inspector);
  } else {
    webkit_web_inspector_detach(inspector);
  }
}

BrowserWebView *
browser_web_view_new()
{
  BrowserWebView *web_view = g_object_new(BROWSER_WEB_VIEW_TYPE, NULL);
  WebKitSettings *settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(web_view));
  g_object_set(G_OBJECT(settings), "allow-universal-access-from-file-urls", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "allow-file-access-from-file-urls", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "enable-page-cache", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "enable-offline-web-application-cache", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "enable-html5-local-storage", TRUE, NULL);

  WebKitWebContext *context = webkit_web_view_get_context(WEBKIT_WEB_VIEW(web_view));
  webkit_web_context_set_cache_model(context, WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);

  GdkRGBA *rgba = malloc(sizeof *rgba);
  gdk_rgba_parse(rgba, "#000000");
  webkit_web_view_set_background_color(WEBKIT_WEB_VIEW(web_view), rgba);
  g_free(rgba);

  g_signal_connect(web_view, "user-message-received", G_CALLBACK(browser_web_view_user_message_received_cb), NULL);
  g_signal_connect(web_view, "context-menu", G_CALLBACK(browser_web_view_context_menu_cb), NULL);

  return web_view;
}
