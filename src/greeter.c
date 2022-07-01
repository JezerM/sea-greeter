#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <locale.h>
#include <gio/gmenu.h>
#include "config.h"
#include "settings.h"
#include "theme.h"
#include "logger.h"

static GdkWindow *root_window;
static GdkDisplay *default_display;

extern GreeterConfig *greeter_config;

static void
destroy_window_cb(GtkWidget* widget, GtkWidget* window) {
  (void) widget;
  (void) window;

  GApplication *app = g_application_get_default();
  GList *windows = gtk_application_get_windows(GTK_APPLICATION(app));
  guint windows_count = g_list_length(windows);

  if (windows_count <= 1) {
    g_application_quit(app);
  }
}
static void
show_window_cb(GtkWidget* widget, GtkWidget* window) {
  (void) widget;
  (void) window;
  logger_debug("Sea Greeter started");
}

/*
 * Destroy window when web-view is closed
 */
static gboolean
close_web_view_cb(WebKitWebView* web_view, GtkWidget* window) {
  (void) web_view;
  gtk_widget_destroy(window);
  return TRUE;
}

/*
 * Enable developer tools or web inspector
 */
static void
enable_developer_tools(WebKitWebView* web_view) {
  WebKitSettings *settings = webkit_web_view_get_settings (WEBKIT_WEB_VIEW(web_view));
  g_object_set (G_OBJECT(settings), "enable-developer-extras", TRUE, NULL);

  WebKitWebInspector *inspector = webkit_web_view_get_inspector(web_view);
  webkit_web_inspector_attach(inspector);
}

/*
 * Initialize web extensions
 */
static void
initialize_web_extensions(WebKitWebContext *context, gpointer user_data) {
  (void) user_data;
  /* Web Extensions get a different ID for each Web Process */
  static guint32 unique_id = 0;

  logger_debug("Extension initialized");

  webkit_web_context_set_web_extensions_directory (
     context, WEB_EXTENSIONS_DIR);
  webkit_web_context_set_web_extensions_initialization_user_data (
     context, g_variant_new_uint32 (unique_id++));
}

/*
 * Callback to be executed when a web-view user message is received
 */
static void
web_view_user_message_received(
    WebKitWebView *web_view,
    WebKitUserMessage *message,
    gpointer user_data)
{
  (void) user_data;
  /*GApplication *app = g_application_get_default();*/
  const char *name = webkit_user_message_get_name(message);

  if (g_strcmp0(name, "ready-to-show") == 0) {
    gtk_widget_grab_focus(GTK_WIDGET(web_view));

    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(web_view));
    gtk_widget_show_all(window);

    /*GList *windows = gtk_application_get_windows(GTK_APPLICATION(app));*/

    /*GList *curr = windows;*/
    /*while (curr != NULL) {*/
      /*GtkWindow *win = curr->data;*/
      /*gtk_widget_show_all(GTK_WIDGET(win));*/
      /*curr = curr->next;*/
    /*}*/
  }
}

/*
 * Set keybinding accelerators
 */
static void
set_keybindings(GtkApplicationWindow *window) {
  GVariant *window_id = g_variant_new_uint32(
      gtk_application_window_get_id(window)
      );
  gchar *toggle_inspector_name = g_action_print_detailed_name(
      "win.toggle-inspector",
      window_id
      );

  const struct accelerator {
    const gchar *action;
    const gchar *accelerators[9];
  } accels[] = {
    { "app.quit", { "<Control>Q", NULL } },
    { toggle_inspector_name, { "<shift><Primary>I", "F12", NULL } },
    { NULL, { NULL } }
  };

  GApplication *app = g_application_get_default();

  int accel_count = G_N_ELEMENTS(accels);
  for (int i = 0; i < accel_count; i++) {
    if (accels[i].action == NULL)
      break;
    gtk_application_set_accels_for_action(
        GTK_APPLICATION(app),
        accels[i].action,
        accels[i].accelerators
        );
  }
}

static void
print_info() {
  logger_debug("INFO");
}

/*
 * Quit application forcedly
 */
static void
app_quit_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  (void) action;
  (void) parameter;
  (void) user_data;
  GApplication *app = g_application_get_default();
  g_application_quit(app);
}

/*
 * Toggle web view inspector
 */
static void
toggle_inspector_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  (void) action;
  (void) parameter;
  (void) user_data;
  GApplication *app = g_application_get_default();

  guint32 window_id = g_variant_get_uint32(parameter);
  GtkApplicationWindow *window_origin = NULL;

  GList *windows = gtk_application_get_windows(GTK_APPLICATION(app));

  GList *curr = windows;
  while (curr != NULL) {
    if (gtk_application_window_get_id(curr->data) == window_id) {
      window_origin = curr->data;
      break;
    }
    curr = curr->next;
  }
  if (window_origin == NULL)
    return;

  GList *window_children = gtk_container_get_children(GTK_CONTAINER(window_origin));
  if (g_list_length(window_children) == 0)
    return;

  window_children = gtk_container_get_children(GTK_CONTAINER(window_children[0].data));

  WebKitWebView *web_view = g_list_last(window_children)->data;
  if (!WEBKIT_IS_WEB_VIEW(web_view))
    return;

  WebKitWebInspector *inspector = webkit_web_view_get_inspector (web_view);
  WebKitWebViewBase *inspector_web_view = webkit_web_inspector_get_web_view(inspector);

  if (inspector_web_view != NULL) {
    webkit_web_inspector_close(WEBKIT_WEB_INSPECTOR(inspector));
  } else {
    webkit_web_inspector_show(WEBKIT_WEB_INSPECTOR(inspector));
  }
}

/*
 * Initialize app actions
 */
static void
initialize_actions (GtkApplication *app)
{
  static const GActionEntry app_entries[] = {
    { "info", print_info, NULL, NULL, NULL, {0}},
    { "quit", app_quit_cb, NULL, NULL, NULL , {0}},
  };
  static const GActionEntry win_entries[] = {
    { "toggle-inspector", toggle_inspector_cb, "u", NULL, NULL , {0}},
  };

  g_action_map_add_action_entries(
      G_ACTION_MAP(app),
      app_entries,
      G_N_ELEMENTS(app_entries),
      app
      );

  GList *windows = gtk_application_get_windows(app);

  GList *curr = windows;
  while (curr != NULL) {
    GtkWindow *win = curr->data;
    g_action_map_add_action_entries(
        G_ACTION_MAP(win),
        win_entries,
        G_N_ELEMENTS(win_entries),
        win
        );
    curr = curr->next;
  }
}

/*
 * Create menu bar Model
 */
static GMenu *
initialize_menu_bar(GtkApplicationWindow *window) {
  GMenu *menu_model = g_menu_new();

  GMenu *file_model = g_menu_new();

  g_menu_append(file_model, "Info", "app.info");
  g_menu_append(file_model, "Quit", "app.quit");
  g_menu_append_submenu(menu_model, "File", G_MENU_MODEL(file_model));

  GMenu *about_model = g_menu_new();

  g_menu_append_submenu(menu_model, "About", G_MENU_MODEL(about_model));

  GMenu *view_model = g_menu_new();

  GMenuItem *item_toggle_inspector = g_menu_item_new("Toggle Developer Tools", NULL);
  g_menu_item_set_action_and_target_value(
      item_toggle_inspector,
      "win.toggle-inspector",
      g_variant_new_uint32(gtk_application_window_get_id(window))
      );
  g_menu_append_item(view_model, item_toggle_inspector);
  /*g_menu_append(view_model, "Toggle Developer Tools", "win.toggle-inspector");*/
  g_menu_append_submenu(menu_model, "View", G_MENU_MODEL(view_model));

  g_object_unref(file_model);
  g_object_unref(about_model);
  g_object_unref(view_model);

  return menu_model;
}

static gboolean
web_view_context_menu(
    WebKitWebView *webView,
    WebKitContextMenu *context_menu,
    GdkEvent *event,
    WebKitHitTestResult *hit_test_result,
    gpointer user_data
    )
{
  (void) webView;
  (void) context_menu;
  (void) event;
  (void) hit_test_result;
  (void) user_data;
  return false;
}

static WebKitWebView *
create_web_view() {
  WebKitWebView *web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());

  WebKitSettings *settings = webkit_web_view_get_settings (WEBKIT_WEB_VIEW(web_view));
  g_object_set(G_OBJECT(settings), "allow-universal-access-from-file-urls", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "allow-file-access-from-file-urls", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "enable-page-cache", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "enable-offline-web-application-cache", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "enable-html5-local-storage", TRUE, NULL);

  WebKitWebContext *context = webkit_web_view_get_context(web_view);
  webkit_web_context_set_cache_model(context, WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);

  g_signal_connect (web_view,
                   "user-message-received",
                    G_CALLBACK (web_view_user_message_received),
                    NULL);
  g_signal_connect (web_view,
                   "context-menu",
                    G_CALLBACK (web_view_context_menu),
                    NULL);
  return web_view;
}

static GtkApplicationWindow*
create_browser(GtkApplication *app, WebKitWebView *web_view) {
  GtkApplicationWindow *window = GTK_APPLICATION_WINDOW(gtk_application_window_new(app));
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

  g_signal_connect(window, "destroy", G_CALLBACK(destroy_window_cb), NULL);
  g_signal_connect(window, "show", G_CALLBACK(show_window_cb), NULL);

  GtkWidget *center_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *menu_bar = gtk_menu_bar_new_from_model(G_MENU_MODEL(initialize_menu_bar(window)));

  gtk_box_pack_start(GTK_BOX(center_vbox), GTK_WIDGET(menu_bar), false, false, 0);
  gtk_box_pack_end(GTK_BOX(center_vbox), GTK_WIDGET(web_view), true, true, 0);

  gtk_container_add(GTK_CONTAINER(window), center_vbox);

  return window;
}

/*
 * Callback to be executed when app is activated.
 * Occurs after ":startup"
 */
static void
app_activate_cb(GtkApplication *app, gpointer user_data) {
  (void) user_data;

  g_signal_connect(webkit_web_context_get_default(),
                   "initialize-web-extensions",
                    G_CALLBACK(initialize_web_extensions),
                    NULL);

  WebKitWebView *web_view = create_web_view();
  /*WebKitWebView *web_view1 = create_web_view();*/

  GtkApplicationWindow *main_window = create_browser(app, web_view);
  /*GtkApplicationWindow *main_window1 = create_browser(app, web_view1);*/

  g_signal_connect(web_view, "close", G_CALLBACK(close_web_view_cb), main_window);
  /*g_signal_connect(web_view1, "close", G_CALLBACK(close_web_view_cb), main_window1);*/
  enable_developer_tools(web_view);
  /*enable_developer_tools(web_view1);*/

  initialize_actions(app);

  set_keybindings(main_window);

  load_theme(web_view);
  /*webkit_web_view_load_uri(web_view1, "google.com");*/
  /*load_theme(web_view1);*/
}

/*
 * Callback to be executed when app is started.
 * Occurs before ":activate"
 */
static void
app_startup_cb(GtkApplication *app, gpointer user_data) {
  (void) app;
  (void) user_data;
}

int main(int argc, char** argv) {
  gtk_init(&argc, &argv);

  GtkApplication *app = gtk_application_new("com.github.jezerm.sea-greeter", G_APPLICATION_FLAGS_NONE);

  setlocale(LC_ALL, "");

  root_window = gdk_get_default_root_window();
  default_display = gdk_display_get_default();

  gdk_window_set_cursor(root_window, gdk_cursor_new_for_display(default_display, GDK_LEFT_PTR));

  load_configuration();
  /*print_greeter_config();*/

  WebKitApplicationInfo *web_info = webkit_application_info_new();
  webkit_application_info_ref(web_info);
  webkit_application_info_set_name(web_info, "com.github.jezerm.sea-greeter");

  /*g_signal_connect (webkit_web_context_get_default(),*/
                   /*"initialize-web-extensions",*/
                    /*G_CALLBACK (initialize_web_extensions),*/
                    /*NULL);*/

  g_signal_connect(app, "activate", G_CALLBACK(app_activate_cb), NULL);
  g_signal_connect(app, "startup", G_CALLBACK(app_startup_cb), NULL);

  g_application_run(G_APPLICATION(app), argc, argv);

  webkit_application_info_unref(web_info);
  free_greeter_config();
  return 0;
}
