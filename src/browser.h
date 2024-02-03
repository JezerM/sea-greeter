#ifndef BROWSER_H
#define BROWSER_H 1

#include <gtk/gtk.h>

#include "browser-web-view.h"

typedef struct {
  int minX;
  int maxX;
  int minY;
  int maxY;
} OverallBoundary;

typedef struct {
  guint64 id;
  gboolean is_primary;
  GdkRectangle geometry;
  OverallBoundary overall_boundary;
} WindowMetadata;

G_BEGIN_DECLS

#define BROWSER_TYPE browser_get_type()
G_DECLARE_FINAL_TYPE(Browser, browser, BROWSER, WINDOW, GtkApplicationWindow)

struct _Browser {
  GtkApplicationWindow parent_instance;

  BrowserWebView *web_view;
  GdkMonitor *monitor;
  gboolean debug_mode;
  gboolean is_primary;
  WindowMetadata meta;
};

Browser *browser_new(GtkApplication *app, GdkMonitor *monitor);
Browser *browser_new_full(GtkApplication *app, GdkMonitor *monitor, gboolean debug_mode, gboolean is_primary);
void browser_show_menu_bar(Browser *browser, gboolean show);

G_END_DECLS

#endif
