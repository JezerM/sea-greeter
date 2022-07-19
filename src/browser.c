#include <webkit2/webkit2.h>

#include "browser-commands.h"
#include "browser-web-view.h"
#include "browser.h"

extern GPtrArray *greeter_browsers;

typedef struct {
  GtkBox *main_box;
  GtkMenuBar *menu_bar;
} BrowserPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(Browser, browser, GTK_TYPE_APPLICATION_WINDOW)

static void
browser_dispose(GObject *gobject)
{
  G_OBJECT_CLASS(browser_parent_class)->dispose(gobject);
}
static void
browser_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(browser_parent_class)->finalize(gobject);
}
static void
browser_destroy(GtkWidget *self)
{
  GTK_WIDGET_CLASS(browser_parent_class)->destroy(GTK_WIDGET(self));
  g_ptr_array_remove(greeter_browsers, self);
}

static const GActionEntry win_entries[] = {
  { "undo", browser_undo_cb, NULL, NULL, NULL, { 0 } },
  { "redo", browser_redo_cb, NULL, NULL, NULL, { 0 } },

  { "copy", browser_copy_cb, NULL, NULL, NULL, { 0 } },
  { "cut", browser_cut_cb, NULL, NULL, NULL, { 0 } },
  { "paste", browser_paste_cb, NULL, NULL, NULL, { 0 } },
  { "paste-plain", browser_paste_plain_cb, NULL, NULL, NULL, { 0 } },
  { "select-all", browser_select_all_cb, NULL, NULL, NULL, { 0 } },

  { "zoom-normal", browser_zoom_normal_cb, NULL, NULL, NULL, { 0 } },
  { "zoom-in", browser_zoom_in_cb, NULL, NULL, NULL, { 0 } },
  { "zoom-out", browser_zoom_out_cb, NULL, NULL, NULL, { 0 } },

  { "reload", browser_reload_cb, NULL, NULL, NULL, { 0 } },
  { "force-reload", browser_force_reload_cb, NULL, NULL, NULL, { 0 } },
};
static const GActionEntry win_debug_entries[] = {
  { "toggle-inspector", browser_toggle_inspector_cb, NULL, NULL, NULL, { 0 } },

  { "fullscreen", browser_fullscreen_cb, NULL, NULL, NULL, { 0 } },

  { "close", browser_close_cb, NULL, NULL, NULL, { 0 } },
  { "minimize", browser_minimize_cb, NULL, NULL, NULL, { 0 } },
};

void
browser_show_menu_bar(Browser *browser, gboolean show)
{
  BrowserPrivate *priv = browser_get_instance_private(browser);
  if (!GTK_IS_WIDGET(priv->menu_bar))
    return;
  GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(priv->menu_bar));
  if (show && parent == NULL) {
    gtk_box_pack_start(priv->main_box, GTK_WIDGET(priv->menu_bar), false, true, 0);
    g_object_unref(priv->menu_bar);
  } else if (parent != NULL) {
    g_object_ref(priv->menu_bar);
    gtk_container_remove(GTK_CONTAINER(priv->main_box), GTK_WIDGET(priv->menu_bar));
  }
}

static void
browser_constructed(GObject *object)
{
  G_OBJECT_CLASS(browser_parent_class)->constructed(object);
  Browser *browser = BROWSER_WINDOW(object);

  g_action_map_add_action_entries(G_ACTION_MAP(browser), win_entries, G_N_ELEMENTS(win_entries), browser);

  if (browser->debug_mode) {
    g_action_map_add_action_entries(G_ACTION_MAP(browser), win_debug_entries, G_N_ELEMENTS(win_debug_entries), browser);

    GtkBuilder *builder = gtk_builder_new_from_resource("/com/github/jezerm/sea_greeter/resources/menu_bar.ui");
    GMenuModel *menu = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
    priv->menu_bar = GTK_MENU_BAR(gtk_menu_bar_new_from_model(menu));

    g_object_unref(builder);
  }
}

static void
browser_class_init(BrowserClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = browser_dispose;
  object_class->finalize = browser_finalize;
  object_class->constructed = browser_constructed;

  widget_class->destroy = browser_destroy;
}

static void
browser_init(Browser *self)
{
  BrowserPrivate *priv = browser_get_instance_private(self);

  self->web_view = browser_web_view_new();

  priv->menu_bar = NULL;
  priv->main_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  gtk_widget_set_name(GTK_WIDGET(box), "box_container");

  /*gtk_box_pack_start(priv->main_box, GTK_WIDGET(priv->menu_bar), false, true, 0);*/
  gtk_box_pack_start(box, GTK_WIDGET(self->web_view), true, true, 0);
  gtk_box_pack_end(priv->main_box, GTK_WIDGET(box), true, true, 0);

  gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(priv->main_box));

  g_object_unref(builder);
}

Browser *
browser_new(GtkApplication *app, GdkMonitor *monitor)
{
  Browser *browser = g_object_new(BROWSER_TYPE, "application", app, NULL);
  GdkRectangle geometry;
  gdk_monitor_get_geometry(monitor, &geometry);

  gtk_window_set_default_size(GTK_WINDOW(browser), geometry.width, geometry.height);

  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(provider, "/com/github/jezerm/sea_greeter/resources/style.css");

  GdkDisplay *display = gdk_monitor_get_display(monitor);
  GdkScreen *screen = gdk_display_get_default_screen(display);
  gtk_style_context_add_provider_for_screen(
      screen,
      GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  g_object_unref(provider);

  return browser;
}
Browser *
browser_new_debug(GtkApplication *app, GdkMonitor *monitor, gboolean debug_mode)
{
  Browser *browser = browser_new(app, monitor);
  BrowserPrivate *priv = browser_get_instance_private(browser);
  if (debug_mode) {
    browser_web_view_set_developer_tools(browser->web_view, true);
    gtk_box_pack_start(priv->main_box, GTK_WIDGET(priv->menu_bar), false, true, 0);
  } else {
    browser_web_view_set_developer_tools(browser->web_view, false);
    gtk_window_fullscreen(GTK_WINDOW(browser));
  }
  return browser;
}
