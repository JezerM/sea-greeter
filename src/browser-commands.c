#include <webkit2/webkit2.h>

#include "browser-commands.h"
#include "browser.h"

/*
 * Toggle web view inspector
 */
void
browser_toggle_inspector_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;

  BrowserWebView *web_view = browser->web_view;

  WebKitWebInspector *inspector = webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(web_view));
  WebKitWebViewBase *inspector_web_view = webkit_web_inspector_get_web_view(inspector);

  if (inspector_web_view != NULL) {
    webkit_web_inspector_close(WEBKIT_WEB_INSPECTOR(inspector));
  } else {
    webkit_web_inspector_show(WEBKIT_WEB_INSPECTOR(inspector));
  }
}

static void
browser_set_zoom(Browser *browser, int zoom)
{
  WebKitWebView *web_view = WEBKIT_WEB_VIEW(browser->web_view);
  double current_zoom = webkit_web_view_get_zoom_level(web_view);
  if (zoom == BROWSER_ZOOM_IN) {
    current_zoom += 0.1;
  } else if (zoom == BROWSER_ZOOM_OUT) {
    current_zoom -= 0.1;
  }
  webkit_web_view_set_zoom_level(web_view, current_zoom);
}
void
browser_zoom_in_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  browser_set_zoom(browser, BROWSER_ZOOM_IN);
}
void
browser_zoom_out_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  browser_set_zoom(browser, BROWSER_ZOOM_OUT);
}

void
browser_copy_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  GtkWidget *focused = gtk_window_get_focus(GTK_WINDOW(browser));

  if (GTK_IS_EDITABLE(focused)) {
    gtk_editable_copy_clipboard(GTK_EDITABLE(focused));
  } else {
    webkit_web_view_execute_editing_command(WEBKIT_WEB_VIEW(browser->web_view), WEBKIT_EDITING_COMMAND_COPY);
  }
}
void
browser_cut_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  GtkWidget *focused = gtk_window_get_focus(GTK_WINDOW(browser));

  if (GTK_IS_EDITABLE(focused)) {
    gtk_editable_cut_clipboard(GTK_EDITABLE(focused));
  } else {
    webkit_web_view_execute_editing_command(WEBKIT_WEB_VIEW(browser->web_view), WEBKIT_EDITING_COMMAND_CUT);
  }
}
void
browser_paste_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  GtkWidget *focused = gtk_window_get_focus(GTK_WINDOW(browser));

  if (GTK_IS_EDITABLE(focused)) {
    gtk_editable_paste_clipboard(GTK_EDITABLE(focused));
  } else {
    webkit_web_view_execute_editing_command(WEBKIT_WEB_VIEW(browser->web_view), WEBKIT_EDITING_COMMAND_PASTE);
  }
}
void
browser_paste_plain_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  GtkWidget *focused = gtk_window_get_focus(GTK_WINDOW(browser));

  if (GTK_IS_EDITABLE(focused)) {
    gtk_editable_paste_clipboard(GTK_EDITABLE(focused));
  } else {
    webkit_web_view_execute_editing_command(
        WEBKIT_WEB_VIEW(browser->web_view),
        WEBKIT_EDITING_COMMAND_PASTE_AS_PLAIN_TEXT);
  }
}
