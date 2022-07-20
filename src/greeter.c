#include <gtk/gtk.h>
#include <locale.h>
#include <stdlib.h>
#include <webkit2/webkit2.h>

#include "config.h"
#include "logger.h"
#include "settings.h"
#include "theme.h"

#include "bridge/greeter_config.h"
#include "bridge/lightdm.h"
#include "browser.h"

static GdkWindow *root_window;
static GdkDisplay *default_display;

extern GreeterConfig *greeter_config;

GPtrArray *greeter_browsers = NULL;

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
 * Set keybinding accelerators
 */
static void
set_keybindings()
{
  const struct accelerator {
    const gchar *action;
    const gchar *accelerators[9];
  } accels[] = {
    { "app.quit", { "<Control>Q", NULL } },

    { "win.toggle-inspector", { "<Shift><Primary>I", "F12", NULL } },

    { "win.undo", { "<Primary>Z", NULL } },
    { "win.redo", { "<Shift><Primary>Z", NULL } },

    { "win.copy", { "<Primary>C", NULL } },
    { "win.cut", { "<Primary>X", NULL } },
    { "win.paste", { "<Primary>V", NULL } },
    { "win.paste-plain", { "<Shift><Primary>V", NULL } },
    { "win.select-all", { "<Primary>A", NULL } },

    { "win.zoom-normal", { "<Primary>0", "<Primary>KP_0", NULL } },
    { "win.zoom-in", { "<Primary>plus", "<Primary>KP_Add", "<Primary>equal", "ZoomIn", NULL } },
    { "win.zoom-out", { "<Primary>minus", "<Primary>KP_Subtract", "ZoomOut", NULL } },
    { "win.fullscreen", { "F11", NULL } },

    { "win.reload", { "<Primary>R", "F5", "Refresh", "Reload", NULL } },
    { "win.force-reload", { "<Shift><Primary>R", "<Shift>F5", NULL } },

    { "win.close", { "<Primary>W", NULL } },
    { "win.minimize", { "<Primary>M", NULL } },

    { NULL, { NULL } },
  };

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
 * Initialize app actions
 */
static void
initialize_actions(GtkApplication *app)
{
  static const GActionEntry app_entries[] = {
    { "info", print_info, NULL, NULL, NULL, { 0 } },
    { "quit", app_quit_cb, NULL, NULL, NULL, { 0 } },
  };

  g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);
}

/*
 * Callback to be executed when app is activated.
 * Occurs after ":startup"
 */
static void
app_activate_cb(GtkApplication *app, gpointer user_data)
{
  (void) user_data;

  root_window = gdk_get_default_root_window();
  default_display = gdk_display_get_default();

  gdk_window_set_cursor(root_window, gdk_cursor_new_for_display(default_display, GDK_LEFT_PTR));

  LightDM_initialize();
  GreeterConfig_initialize();

  g_signal_connect(
      webkit_web_context_get_default(),
      "initialize-web-extensions",
      G_CALLBACK(initialize_web_extensions),
      NULL);

  greeter_browsers = g_ptr_array_new();

  GdkDisplay *display = gdk_display_get_default();
  int n_monitors = gdk_display_get_n_monitors(display);
  gboolean debug_mode = greeter_config->greeter->debug_mode;

  /*n_monitors++;*/
  for (int i = 0; i < n_monitors; i++) {
    GdkMonitor *monitor = gdk_display_get_monitor(display, i);
    /*GdkMonitor *monitor = gdk_display_get_monitor(display, 0);*/

    Browser *browser = browser_new_debug(app, monitor, debug_mode);
    g_ptr_array_add(greeter_browsers, browser);

    load_theme(browser->web_view);
  }

  initialize_actions(app);
  set_keybindings();
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

int
main(int argc, char **argv)
{
  GtkApplication *app = gtk_application_new("com.github.jezerm.sea-greeter", G_APPLICATION_FLAGS_NONE);

  setlocale(LC_ALL, "");

  load_configuration();
  /*print_greeter_config();*/

  WebKitApplicationInfo *web_info = webkit_application_info_new();
  webkit_application_info_set_name(web_info, "com.github.jezerm.sea-greeter");

  g_signal_connect(app, "activate", G_CALLBACK(app_activate_cb), NULL);
  g_signal_connect(app, "startup", G_CALLBACK(app_startup_cb), NULL);

  g_application_parse_args(&argc, &argv);

  g_application_run(G_APPLICATION(app), argc, argv);

  webkit_application_info_unref(web_info);
  LightDM_destroy();
  free_greeter_config();
  return 0;
}
