#ifndef BROWSER_WEB_VIEW_H
#define BROWSER_WEB_VIEW_H 1

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

G_BEGIN_DECLS

#define BROWSER_WEB_VIEW_TYPE browser_web_view_get_type()
G_DECLARE_FINAL_TYPE(BrowserWebView, browser_web_view, BROWSER, WEB_VIEW, WebKitWebView)

BrowserWebView *browser_web_view_new();
void browser_web_view_set_developer_tools(BrowserWebView *web_view, gboolean value);

G_END_DECLS

#endif
