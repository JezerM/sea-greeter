#ifndef BROWSER_COMMANDS_H
#define BROWSER_COMMANDS_H 1

#include <glib.h>
#include <gio/gio.h>

enum {
  BROWSER_ZOOM_IN,
  BROWSER_ZOOM_OUT,
};

void
browser_toggle_inspector_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void
browser_zoom_in_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void
browser_zoom_out_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void
browser_copy_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void
browser_cut_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void
browser_paste_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void
browser_paste_plain_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);

#endif
