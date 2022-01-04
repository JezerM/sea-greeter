#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <locale.h>
#include "config.h"
#include "settings.h"
#include "theme.h"
#include "logger.h"

static WebKitWebView *web_view;
static GtkWidget *main_window;
static GdkWindow *root_window;
static GdkDisplay *default_display;

extern GreeterConfig *greeter_config;

static void
destroyWindowCb(GtkWidget* widget, GtkWidget* window) {
  (void) widget;
  (void) window;
  gtk_main_quit();
}
static void
showWindowCb(GtkWidget* widget, GtkWidget* window) {
  (void) widget;
  (void) window;
  logger_debug("Sea Greeter started");
}

static gboolean
closeWebViewCb(WebKitWebView* webView, GtkWidget* window) {
  (void) webView;
  gtk_widget_destroy(window);
  return TRUE;
}

static void
enable_developer_tools(WebKitWebView* webView) {
  WebKitSettings *settings = webkit_web_view_get_settings (WEBKIT_WEB_VIEW(webView));
  g_object_set (G_OBJECT(settings), "enable-developer-extras", TRUE, NULL);
  WebKitWebInspector *inspector = webkit_web_view_get_inspector(webView);
  webkit_web_inspector_attach(inspector);
}

static void
initialize_web_extensions(WebKitWebContext *context, gpointer user_data) {
  (void) user_data;
  /* Web Extensions get a different ID for each Web Process */
  static guint32 unique_id = 0;

  webkit_web_context_set_web_extensions_directory (
     context, WEB_EXTENSIONS_DIR);
  webkit_web_context_set_web_extensions_initialization_user_data (
     context, g_variant_new_uint32 (unique_id++));
}

static void
web_view_user_message_received(WebKitWebView *webView,
    WebKitUserMessage *message,
    gpointer user_data)
{
  (void) webView;
  (void) user_data;
  const char *name = webkit_user_message_get_name(message);
  if (g_strcmp0(name, "ready-to-show") == 0) {
    gtk_widget_grab_focus(GTK_WIDGET(web_view));
    gtk_widget_show_all(main_window);
  }
}

int main(int argc, char** argv) {
  gtk_init(&argc, &argv);

  setlocale(LC_ALL, "");

	root_window = gdk_get_default_root_window();
	default_display = gdk_display_get_default();

  gdk_window_set_cursor(root_window, gdk_cursor_new_for_display(default_display, GDK_LEFT_PTR));

  load_configuration();
  /*print_greeter_config();*/

  WebKitApplicationInfo *web_info = webkit_application_info_new();
  webkit_application_info_ref(web_info);
  webkit_application_info_set_name(web_info, "com.github.jezerm.sea-greeter");

  main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);

  g_signal_connect (webkit_web_context_get_default(),
                   "initialize-web-extensions",
                    G_CALLBACK (initialize_web_extensions),
                    NULL);

  web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
  WebKitSettings *settings = webkit_web_view_get_settings (WEBKIT_WEB_VIEW(web_view));
  g_object_set(G_OBJECT(settings), "allow-universal-access-from-file-urls", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "allow-file-access-from-file-urls", TRUE, NULL);

  WebKitWebContext *context = webkit_web_view_get_context(web_view);
  webkit_web_context_set_cache_model(context, WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);

  g_signal_connect (web_view,
                   "user-message-received",
                    G_CALLBACK (web_view_user_message_received),
                    NULL);
  load_theme(web_view);

  gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(web_view));

  enable_developer_tools(web_view);

  g_signal_connect(main_window, "destroy", G_CALLBACK(destroyWindowCb), NULL);
  g_signal_connect(main_window, "show", G_CALLBACK(showWindowCb), NULL);
  g_signal_connect(web_view, "close", G_CALLBACK(closeWebViewCb), main_window);

  /*webkit_web_view_load_uri(web_view, greeter_config->greeter->theme->str);*/
  /*webkit_web_view_load_uri(web_view, "https://jezerm.github.io/lightdm-void-theme/#/home");*/

  gtk_main();

  webkit_application_info_unref(web_info);
  free_greeter_config();
  return 0;
}
