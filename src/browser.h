#ifndef BROWSER_H
#define BROWSER_H 1

#include <gtk/gtk.h>

#include "browser-web-view.h"

G_BEGIN_DECLS

#define BROWSER_TYPE browser_get_type()
G_DECLARE_FINAL_TYPE(Browser, browser, BROWSER, WINDOW, GtkApplicationWindow)

struct _Browser {
  GtkApplicationWindow parent_instance;

  BrowserWebView *web_view;
};

Browser *browser_new(GtkApplication *app, GdkMonitor *monitor);
Browser * browser_new_debug(GtkApplication *app, GdkMonitor *monitor, gboolean debug_mode);

G_END_DECLS

#endif
