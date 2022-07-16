#include <gtk/gtk.h>
#include <locale.h>
#include <stdlib.h>
#include <webkit2/webkit2.h>

#include "config.h"
#include "logger.h"
#include "settings.h"
#include "theme.h"

#include "bridge/lightdm.h"
#include "greeter.h"

static GdkWindow *root_window;
static GdkDisplay *default_display;

extern GreeterConfig *greeter_config;

GPtrArray *greeter_browsers = NULL;

static void
destroy_window_cb(GtkWidget *widget, GtkWidget *window)
{
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
show_window_cb(GtkWidget *widget, GtkWidget *window)
{
  (void) widget;
  (void) window;
  logger_debug("Sea Greeter started");
}

/*
 * Destroy window when web-view is closed
 */
static gboolean
close_web_view_cb(WebKitWebView *web_view, GtkBrowser *browser)
{
  (void) web_view;
  gtk_widget_destroy(GTK_WIDGET(browser->window));
  g_ptr_array_remove(greeter_browsers, browser);
  g_free(browser);
  return TRUE;
}

/*
 * Enable developer tools or web inspector
 */
static void
enable_developer_tools(WebKitWebView *web_view)
{
  WebKitSettings *settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(web_view));
  g_object_set(G_OBJECT(settings), "enable-developer-extras", TRUE, NULL);

  WebKitWebInspector *inspector = webkit_web_view_get_inspector(web_view);
  webkit_web_inspector_attach(inspector);
}

/*
 * Initialize web extensions
 */
static void
initialize_web_extensions(WebKitWebContext *context, gpointer user_data)
{
  (void) user_data;
  /* Web Extensions get a different ID for each Web Process */
  static guint32 unique_id = 0;

  logger_debug("Extension initialized");

  webkit_web_context_set_web_extensions_directory(context, WEB_EXTENSIONS_DIR);
  webkit_web_context_set_web_extensions_initialization_user_data(context, g_variant_new_uint32(unique_id++));
}

/*
 * Callback to be executed when a web-view user message is received
 */
static void
web_view_user_message_received(WebKitWebView *web_view, WebKitUserMessage *message, gpointer user_data)
{
  (void) user_data;
  const char *name = webkit_user_message_get_name(message);

  /*printf("Got message: '%s'\n", name);*/

  if (g_strcmp0(name, "ready-to-show") == 0) {
    gtk_widget_grab_focus(GTK_WIDGET(web_view));

    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(web_view));
    gtk_widget_show_all(window);
    return;
  }
  handle_lightdm_accessor(web_view, message);
}

/*
 * Set keybinding accelerators
 */
static void
set_keybindings()
{
  const struct accelerator {
    const gchar *action;
    const gchar *accelerators[9];
  } accels[] = { { "app.quit", { "<Control>Q", NULL } },
                 { "win.toggle-inspector", { "<shift><Primary>I", "F12", NULL } },
                 { NULL, { NULL } } };

  GApplication *app = g_application_get_default();

  int accel_count = G_N_ELEMENTS(accels);
  for (int i = 0; i < accel_count; i++) {
    if (accels[i].action == NULL)
      break;
    gtk_application_set_accels_for_action(GTK_APPLICATION(app), accels[i].action, accels[i].accelerators);
  }
}

static void
print_info()
{
  logger_debug("INFO");
}

/*
 * Quit application forcedly
 */
static void
app_quit_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
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
toggle_inspector_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  (void) user_data;
  GApplication *app = g_application_get_default();

  GtkApplicationWindow *window_origin = NULL;

  GList *windows = gtk_application_get_windows(GTK_APPLICATION(app));

  GList *curr = windows;
  while (curr != NULL) {
    if (gtk_window_has_toplevel_focus(curr->data)) {
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

  WebKitWebInspector *inspector = webkit_web_view_get_inspector(web_view);
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
initialize_actions(GtkApplication *app)
{
  static const GActionEntry app_entries[] = {
    { "info", print_info, NULL, NULL, NULL, { 0 } },
    { "quit", app_quit_cb, NULL, NULL, NULL, { 0 } },
  };
  static const GActionEntry win_entries[] = {
    { "toggle-inspector", toggle_inspector_cb, NULL, NULL, NULL, { 0 } },
  };

  g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);

  GList *windows = gtk_application_get_windows(app);

  GList *curr = windows;
  while (curr != NULL) {
    GtkWindow *win = curr->data;
    g_action_map_add_action_entries(G_ACTION_MAP(win), win_entries, G_N_ELEMENTS(win_entries), win);
    curr = curr->next;
  }
}

/*
 * Create menu bar Model
 */
static GMenu *
initialize_menu_bar()
{
  GMenu *menu_model = g_menu_new();

  GMenu *file_model = g_menu_new();

  g_menu_append(file_model, "Info", "app.info");
  g_menu_append(file_model, "Quit", "app.quit");
  g_menu_append_submenu(menu_model, "File", G_MENU_MODEL(file_model));

  GMenu *about_model = g_menu_new();

  g_menu_append_submenu(menu_model, "About", G_MENU_MODEL(about_model));

  GMenu *view_model = g_menu_new();

  g_menu_append(view_model, "Toggle Developer Tools", "win.toggle-inspector");
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
    gpointer user_data)
{
  (void) webView;
  (void) context_menu;
  (void) event;
  (void) hit_test_result;
  (void) user_data;
  return false;
}

static WebKitWebView *
create_web_view()
{
  WebKitWebView *web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());

  WebKitSettings *settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(web_view));
  g_object_set(G_OBJECT(settings), "allow-universal-access-from-file-urls", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "allow-file-access-from-file-urls", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "enable-page-cache", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "enable-offline-web-application-cache", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "enable-html5-local-storage", TRUE, NULL);

  WebKitWebContext *context = webkit_web_view_get_context(web_view);
  webkit_web_context_set_cache_model(context, WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);

  GdkRGBA *rgba = malloc(sizeof *rgba);
  gdk_rgba_parse(rgba, "#000000");
  webkit_web_view_set_background_color(web_view, rgba);
  g_free(rgba);

  g_signal_connect(web_view, "user-message-received", G_CALLBACK(web_view_user_message_received), NULL);
  g_signal_connect(web_view, "context-menu", G_CALLBACK(web_view_context_menu), NULL);
  return web_view;
}

static GtkBrowser *
create_browser(GtkApplication *app, WebKitWebView *web_view, GdkMonitor *monitor)
{
  GtkApplicationWindow *window = GTK_APPLICATION_WINDOW(gtk_application_window_new(app));

  GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(window));
  GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
  gtk_widget_set_visual(GTK_WIDGET(window), visual);
  gtk_widget_set_app_paintable(GTK_WIDGET(window), true);

  GdkRectangle geometry;
  gdk_monitor_get_geometry(monitor, &geometry);

  gtk_window_set_default_size(GTK_WINDOW(window), geometry.width, geometry.height);

  g_signal_connect(window, "destroy", G_CALLBACK(destroy_window_cb), NULL);
  g_signal_connect(window, "show", G_CALLBACK(show_window_cb), NULL);

  GtkWidget *center_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_end(GTK_BOX(center_vbox), GTK_WIDGET(web_view), true, true, 0);

  gtk_container_add(GTK_CONTAINER(window), center_vbox);

  GtkBrowser *browser = malloc(sizeof *browser);
  browser->window = window;
  browser->web_view = web_view;

  g_signal_connect(web_view, "close", G_CALLBACK(close_web_view_cb), browser);

  if (greeter_config->greeter->debug_mode) {
    GMenuModel *menu_model = G_MENU_MODEL(initialize_menu_bar());
    GtkWidget *menu_bar = gtk_menu_bar_new_from_model(menu_model);

    gtk_box_pack_start(GTK_BOX(center_vbox), GTK_WIDGET(menu_bar), false, false, 0);
    enable_developer_tools(web_view);

    g_object_unref(menu_model);
  } else {
    gtk_window_fullscreen(GTK_WINDOW(window));
  }

  return browser;
}

/*
 * Callback to be executed when app is activated.
 * Occurs after ":startup"
 */
static void
app_activate_cb(GtkApplication *app, gpointer user_data)
{
  (void) user_data;

  LightDM_initialize();

  g_signal_connect(
      webkit_web_context_get_default(),
      "initialize-web-extensions",
      G_CALLBACK(initialize_web_extensions),
      NULL);

  greeter_browsers = g_ptr_array_new();

  WebKitWebView *web_view = create_web_view();
  /*WebKitWebView *web_view1 = create_web_view();*/

  GdkDisplay *display = gdk_display_get_default();

  int n_monitors = gdk_display_get_n_monitors(display);

  for (int i = 0; i < n_monitors; i++) {
    GdkMonitor *monitor = gdk_display_get_monitor(display, i);
    GtkBrowser *browser = create_browser(app, web_view, monitor);

    g_ptr_array_add(greeter_browsers, browser);
  }

  /*GtkBrowser *browser1 = create_browser(app, web_view1);*/

  /*g_ptr_array_add(greeter_browsers, browser1);*/

  initialize_actions(app);
  set_keybindings();

  load_theme(web_view);
  /*load_theme(web_view1);*/
}

/*
 * Callback to be executed when app is started.
 * Occurs before ":activate"
 */
static void
app_startup_cb(GtkApplication *app, gpointer user_data)
{
  (void) app;
  (void) user_data;
}

static void
g_application_parse_args(gint *argc, gchar ***argv)
{
  GOptionContext *context = g_option_context_new(NULL);

  gboolean version = false;

  gchar *mode_str = NULL;
  gboolean debug = false;
  gboolean normal = false;

  gchar *theme = NULL;
  gboolean list = false;

  GOptionEntry entries[] = {
    { "version", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &version, "Version", NULL },

    { "mode", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &mode_str, "Mode", NULL },
    { "debug", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &debug, "Debug mode", NULL },
    { "normal", 'n', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &normal, "Normal mode", NULL },

    { "theme", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &theme, "Theme", NULL },
    { "list", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &list, "List installed themes", NULL },
    { NULL, 0, 0, 0, NULL, NULL, NULL },
  };

  g_option_context_add_main_entries(context, entries, NULL);
  GOptionGroup *gtk_group = gtk_get_option_group(false);
  g_option_context_add_group(context, gtk_group);
  g_option_context_set_help_enabled(context, true);

  g_option_context_parse(context, argc, argv, NULL);
  g_option_context_free(context);

  if (version) {
    printf("%s\n", VERSION);
    exit(0);
  }
  if (list) {
    printf("List\n"); // TODO
    exit(0);
  }
  if (theme) {
    greeter_config->greeter->theme = g_string_new(theme);
    g_free(theme);
  }

  if (mode_str && debug && normal) {
    fprintf(stderr, "Conflict arguments: \"--mode\", \"--debug\" and \"--normal\"\n");
    exit(1);
  } else if (mode_str && debug) {
    fprintf(stderr, "Conflict arguments: \"--mode\" and \"--debug\"\n");
    exit(1);
  } else if (mode_str && normal) {
    fprintf(stderr, "Conflict arguments: \"--mode\" and \"--normal\"\n");
    exit(1);
  } else if (debug && normal) {
    fprintf(stderr, "Conflict arguments: \"--debug\" and \"--normal\"\n");
    exit(1);
  }

  if (mode_str && g_strcmp0(mode_str, "debug") == 0) {
    debug = true;
  } else if (mode_str && g_strcmp0(mode_str, "normal") == 0) {
    normal = true;
  } else if (mode_str) {
    fprintf(stderr, "Argument --mode should be: \"debug\" or \"normal\"\n");
    exit(1);
  }
  if (mode_str)
    g_free(mode_str);

  if (debug) {
    greeter_config->greeter->debug_mode = true;
  } else if (normal) {
    greeter_config->greeter->debug_mode = false;
  }
}
static int
gtk_application_on_command_line(GtkApplication *app, GApplicationCommandLine *command_line)
{
  gint argc;
  gchar **argv;
  argv = g_application_command_line_get_arguments(command_line, &argc);

  g_application_parse_args(&argc, &argv);

  g_application_activate(G_APPLICATION(app));
  return 0;
}

int
main(int argc, char **argv)
{
  gtk_init(&argc, &argv);

  GtkApplication *app = gtk_application_new("com.github.jezerm.sea-greeter", G_APPLICATION_HANDLES_COMMAND_LINE);

  setlocale(LC_ALL, "");

  root_window = gdk_get_default_root_window();
  default_display = gdk_display_get_default();

  gdk_window_set_cursor(root_window, gdk_cursor_new_for_display(default_display, GDK_LEFT_PTR));

  load_configuration();
  /*print_greeter_config();*/

  WebKitApplicationInfo *web_info = webkit_application_info_new();
  webkit_application_info_ref(web_info);
  webkit_application_info_set_name(web_info, "com.github.jezerm.sea-greeter");

  g_signal_connect(app, "activate", G_CALLBACK(app_activate_cb), NULL);
  g_signal_connect(app, "startup", G_CALLBACK(app_startup_cb), NULL);
  g_signal_connect(app, "command_line", G_CALLBACK(gtk_application_on_command_line), NULL);

  g_application_run(G_APPLICATION(app), argc, argv);

  webkit_application_info_unref(web_info);
  free_greeter_config();
  return 0;
}
