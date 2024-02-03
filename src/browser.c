#include <webkit2/webkit2.h>

#include "browser-commands.h"
#include "browser-web-view.h"
#include "browser.h"

extern GPtrArray *greeter_browsers;

typedef struct {
  guint64 id;
  GtkBox *main_box;
  GtkMenuBar *menu_bar;
} BrowserPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(Browser, browser, GTK_TYPE_APPLICATION_WINDOW)

typedef enum {
  PROP_ID = 1,
  PROP_MONITOR,
  PROP_DEBUG_MODE,
  PROP_IS_PRIMARY,
  N_PROPERTIES,
} BrowserProperty;

static GParamSpec *browser_properties[N_PROPERTIES] = { NULL };

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

static guint64
browser_gen_id(Browser *self)
{
  const char *manufacturer = gdk_monitor_get_manufacturer(self->monitor);
  const char *model = gdk_monitor_get_model(self->monitor);

  guint64 manufacturer_hash = manufacturer == NULL ? 0 : g_str_hash(manufacturer);
  guint64 model_hash = model == NULL ? 0 : g_str_hash(model);

  return (manufacturer_hash << 24) | (model_hash << 8);
}

static void
browser_initiate_metadata(Browser *self)
{
  BrowserPrivate *priv = browser_get_instance_private(self);

  self->meta.id = priv->id;
  self->meta.is_primary = self->is_primary;

  gtk_window_get_position(GTK_WINDOW(self), &self->meta.geometry.x, &self->meta.geometry.y);
  gtk_window_get_size(GTK_WINDOW(self), &self->meta.geometry.width, &self->meta.geometry.height);
}

void
browser_set_overall_boundary(GPtrArray *browsers)
{
  OverallBoundary overall_boundary;

  overall_boundary.maxX = -INT_MAX;
  overall_boundary.maxY = -INT_MAX;
  overall_boundary.minX = INT_MAX;
  overall_boundary.minY = INT_MAX;

  for (guint i = 0; i < browsers->len; i++) {
    Browser *browser = browsers->pdata[i];
    GdkRectangle geometry = browser->meta.geometry;
    overall_boundary.minX = MIN(overall_boundary.minX, geometry.x);
    overall_boundary.minY = MIN(overall_boundary.minY, geometry.y);
    overall_boundary.maxX = MAX(overall_boundary.maxX, geometry.x + geometry.width);
    overall_boundary.maxY = MAX(overall_boundary.maxY, geometry.y + geometry.height);
  }
  for (guint i = 0; i < browsers->len; i++) {
    Browser *browser = browsers->pdata[i];
    browser->meta.overall_boundary = overall_boundary;
  }
}

static void
browser_constructed(GObject *object)
{
  G_OBJECT_CLASS(browser_parent_class)->constructed(object);
  Browser *browser = BROWSER_WINDOW(object);
  BrowserPrivate *priv = browser_get_instance_private(browser);

  GdkRectangle geometry;
  gdk_monitor_get_geometry(browser->monitor, &geometry);

  gtk_window_set_default_size(GTK_WINDOW(browser), geometry.width, geometry.height);

  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(provider, "/com/github/jezerm/sea_greeter/resources/style.css");
  GdkDisplay *display = gdk_monitor_get_display(browser->monitor);
  GdkScreen *screen = gdk_display_get_default_screen(display);
  gtk_style_context_add_provider_for_screen(
      screen,
      GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  g_object_unref(provider);

  g_action_map_add_action_entries(G_ACTION_MAP(browser), win_entries, G_N_ELEMENTS(win_entries), browser);

  priv->id = browser_gen_id(browser);

  browser_initiate_metadata(browser);

  if (browser->debug_mode) {
    g_action_map_add_action_entries(G_ACTION_MAP(browser), win_debug_entries, G_N_ELEMENTS(win_debug_entries), browser);
    browser_web_view_set_developer_tools(browser->web_view, true);

    GtkBuilder *builder = gtk_builder_new_from_resource("/com/github/jezerm/sea_greeter/resources/menu_bar.ui");
    GMenuModel *menu = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
    priv->menu_bar = GTK_MENU_BAR(gtk_menu_bar_new_from_model(menu));
    gtk_box_pack_start(priv->main_box, GTK_WIDGET(priv->menu_bar), false, true, 0);

    g_object_unref(builder);
  } else {
    browser_web_view_set_developer_tools(browser->web_view, false);
    gtk_window_fullscreen(GTK_WINDOW(browser));
  }
}

static void
browser_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  (void) pspec;
  Browser *self = BROWSER_WINDOW(object);
  switch ((BrowserProperty) property_id) {
    case PROP_MONITOR:
      self->monitor = g_value_dup_object(value);
      break;
    case PROP_DEBUG_MODE:
      self->debug_mode = g_value_get_boolean(value);
      break;
    case PROP_IS_PRIMARY:
      self->is_primary = g_value_get_boolean(value);
      break;
    default:
      break;
  }
}
static void
browser_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  (void) pspec;
  Browser *self = BROWSER_WINDOW(object);
  BrowserPrivate *priv = browser_get_instance_private(self);

  switch ((BrowserProperty) property_id) {
    case PROP_ID:
      g_value_set_int(value, priv->id);
      break;
    case PROP_MONITOR:
      g_value_set_object(value, self->monitor);
      break;
    case PROP_DEBUG_MODE:
      g_value_set_boolean(value, self->debug_mode);
      break;
    case PROP_IS_PRIMARY:
      g_value_set_boolean(value, self->is_primary);
      break;
    default:
      break;
  }
}

static void
browser_class_init(BrowserClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  object_class->set_property = browser_set_property;
  object_class->get_property = browser_get_property;

  object_class->dispose = browser_dispose;
  object_class->finalize = browser_finalize;
  object_class->constructed = browser_constructed;

  widget_class->destroy = browser_destroy;

  browser_properties[PROP_ID] = g_param_spec_int("id", "ID", "The window internal id", 0, INT_MAX, 0, G_PARAM_READABLE);

  browser_properties[PROP_MONITOR] = g_param_spec_object(
      "monitor",
      "Monitor",
      "Monitor where browser should be placed",
      GDK_TYPE_MONITOR,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  browser_properties[PROP_DEBUG_MODE] = g_param_spec_boolean(
      "debug_mode",
      "DebugMode",
      "Whether the greeter is in debug mode or not",
      false,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  browser_properties[PROP_IS_PRIMARY] = g_param_spec_boolean(
      "is_primary",
      "IsPrimary",
      "Whether the browser is in a primary monitor or not",
      true,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties(object_class, N_PROPERTIES, browser_properties);
}

static void
browser_init(Browser *self)
{
  BrowserPrivate *priv = browser_get_instance_private(self);

  self->web_view = browser_web_view_new();
  self->is_primary = true;

  priv->menu_bar = NULL;
  priv->main_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  gtk_widget_set_name(GTK_WIDGET(box), "box_container");

  /*gtk_box_pack_start(priv->main_box, GTK_WIDGET(priv->menu_bar), false, true, 0);*/
  gtk_box_pack_start(box, GTK_WIDGET(self->web_view), true, true, 0);
  gtk_box_pack_end(priv->main_box, GTK_WIDGET(box), true, true, 0);

  gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(priv->main_box));
}

Browser *
browser_new(GtkApplication *app, GdkMonitor *monitor)
{
  Browser *browser = g_object_new(BROWSER_TYPE, "application", app, "monitor", monitor, NULL);
  return browser;
}
Browser *
browser_new_full(GtkApplication *app, GdkMonitor *monitor, gboolean debug_mode, gboolean is_primary)
{
  Browser *browser = g_object_new(
      BROWSER_TYPE,
      "application",
      app,
      "monitor",
      monitor,
      "debug_mode",
      debug_mode,
      "is_primary",
      is_primary,
      NULL);
  return browser;
}
