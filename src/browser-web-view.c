#include <webkit2/webkit2.h>

#include "bridge/greeter_comm.h"
#include "bridge/greeter_config.h"
#include "bridge/lightdm.h"
#include "bridge/theme_utils.h"
#include "browser-web-view.h"
#include "browser.h"
#include "logger.h"
#include "settings.h"
#include "theme.h"

extern GreeterConfig *greeter_config;
extern GPtrArray *greeter_browsers;

struct _BrowserWebView {
  WebKitWebView parent_instance;
};
typedef struct {
  gboolean loaded;
} BrowserWebViewPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(BrowserWebView, browser_web_view, WEBKIT_TYPE_WEB_VIEW)

typedef enum {
  ERROR_PROMPT_CANCEL,
  ERROR_PROMPT_DEFAULT_THEME,
  ERROR_PROMPT_RELOAD_THEME,
} ErrorPromptResponseType;

static void
show_console_error_prompt(BrowserWebView *web_view, WebKitUserMessage *user_message)
{
  GVariant *params = webkit_user_message_get_parameters(user_message);

  char *type = NULL;
  char *message = NULL;
  char *source_id = NULL;
  guint line = 0;

  g_variant_get(params, "(sssu)", &type, &message, &source_id, &line);

  GtkWidget *root_window = gtk_widget_get_toplevel(GTK_WIDGET(web_view));

  GtkDialog *dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(
      "An error ocurred",
      GTK_WINDOW(root_window),
      GTK_DIALOG_MODAL,
      "_Cancel",
      ERROR_PROMPT_CANCEL,
      "_Use default theme",
      ERROR_PROMPT_DEFAULT_THEME,
      "_Reload theme",
      ERROR_PROMPT_RELOAD_THEME,
      NULL));

  gtk_dialog_set_default_response(dialog, ERROR_PROMPT_DEFAULT_THEME);

  GtkBox *content_area = GTK_BOX(gtk_dialog_get_content_area(dialog));
  GtkWidget *error_occurred = gtk_label_new("An error ocurred. Do you want to change to default theme? (gruvbox)");

  char *error_message = g_strdup_printf("%s %d: %s", source_id, line, message);
  GtkWidget *label = gtk_label_new(error_message);

  gtk_container_add(GTK_CONTAINER(content_area), error_occurred);
  gtk_container_add(GTK_CONTAINER(content_area), label);
  gtk_widget_show_all(GTK_WIDGET(content_area));

  gtk_widget_set_name(GTK_WIDGET(dialog), "error-prompt");
  int response = gtk_dialog_run(dialog);
  gtk_widget_destroy(GTK_WIDGET(dialog));

  gboolean stop_prompts = false;

  switch ((ErrorPromptResponseType) response) {
    case ERROR_PROMPT_CANCEL:
      break;
    case ERROR_PROMPT_DEFAULT_THEME:
      if (!BROWSER_IS_WINDOW(root_window))
        break;
      stop_prompts = true;
      g_free(greeter_config->greeter->theme);
      greeter_config->greeter->theme = g_strdup("gruvbox");
      for (guint i = 0; i < greeter_browsers->len; i++) {
        Browser *browser = greeter_browsers->pdata[i];
        load_theme(browser);
      }
      break;
    case ERROR_PROMPT_RELOAD_THEME:
      stop_prompts = true;
      for (guint i = 0; i < greeter_browsers->len; i++) {
        Browser *browser = greeter_browsers->pdata[i];
        BrowserWebView *web_v = browser->web_view;
        webkit_web_view_reload(WEBKIT_WEB_VIEW(web_v));
      }
      break;
    default:
      break;
  }
  g_autoptr(GVariant) reply_params = g_variant_new("(b)", stop_prompts);
  WebKitUserMessage *reply = webkit_user_message_new("console-done", g_steal_pointer(&reply_params));
  webkit_user_message_send_reply(user_message, reply);
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

  if (g_strcmp0(name, "console") == 0) {
    show_console_error_prompt(web_view, message);
    return;
  }

  handle_lightdm_accessor(WEBKIT_WEB_VIEW(web_view), message);
  handle_greeter_config_accessor(WEBKIT_WEB_VIEW(web_view), message);
  handle_theme_utils_accessor(WEBKIT_WEB_VIEW(web_view), message);
  handle_greeter_comm_accessor(WEBKIT_WEB_VIEW(web_view), message);
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

static void
browser_web_view_constructed(GObject *object)
{
  G_OBJECT_CLASS(browser_web_view_parent_class)->constructed(object);
  BrowserWebView *web_view = BROWSER_WEB_VIEW(object);

  WebKitSettings *settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(web_view));
  g_object_set(G_OBJECT(settings), "allow-universal-access-from-file-urls", true, NULL);
  g_object_set(G_OBJECT(settings), "allow-file-access-from-file-urls", true, NULL);
  g_object_set(G_OBJECT(settings), "enable-page-cache", true, NULL);
  g_object_set(G_OBJECT(settings), "enable-offline-web-application-cache", true, NULL);
  g_object_set(G_OBJECT(settings), "enable-html5-local-storage", true, NULL);
  g_object_set(G_OBJECT(settings), "enable-webgl", true, NULL);
  g_object_set(G_OBJECT(settings), "hardware-acceleration-policy", WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS, NULL);

  WebKitWebContext *context = webkit_web_view_get_context(WEBKIT_WEB_VIEW(web_view));
  webkit_web_context_set_cache_model(context, WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);

  g_autoptr(GdkRGBA) rgba = malloc(sizeof *rgba);
  gdk_rgba_parse(rgba, "#000000");
  webkit_web_view_set_background_color(WEBKIT_WEB_VIEW(web_view), rgba);

  g_signal_connect(web_view, "user-message-received", G_CALLBACK(browser_web_view_user_message_received_cb), NULL);
  g_signal_connect(web_view, "context-menu", G_CALLBACK(browser_web_view_context_menu_cb), NULL);
}

static void
browser_web_view_class_init(BrowserWebViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->constructed = browser_web_view_constructed;
}
static void
browser_web_view_init(BrowserWebView *self)
{
  BrowserWebViewPrivate *priv = browser_web_view_get_instance_private(self);
  priv->loaded = false;
}

BrowserWebView *
browser_web_view_new()
{
  BrowserWebView *web_view = g_object_new(BROWSER_WEB_VIEW_TYPE, NULL);
  return web_view;
}
