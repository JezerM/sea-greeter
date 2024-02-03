#ifndef BROWSER_COMMANDS_H
#define BROWSER_COMMANDS_H 1

#include <gio/gio.h>
#include <glib.h>

enum {
  BROWSER_ZOOM_IN,
  BROWSER_ZOOM_OUT,
};

void browser_toggle_inspector_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_zoom_in_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_zoom_out_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_zoom_normal_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_undo_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_redo_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_copy_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_cut_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_paste_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_paste_plain_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_select_all_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_reload_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_force_reload_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_fullscreen_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_close_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void browser_minimize_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);

#endif
