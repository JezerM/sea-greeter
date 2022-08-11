#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <jsc/jsc.h>
#include <webkit2/webkit2.h>

#include "bridge/bridge-object.h"
#include "bridge/lightdm-objects.h"
#include "bridge/utils.h"

#include "logger.h"
#include "settings.h"
#include "utils/utils.h"

static GPtrArray *allowed_dirs = NULL;
extern GString *shared_data_directory;

static BridgeObject *ThemeUtils_object = NULL;

static void *
ThemeUtils_dirlist_cb(GPtrArray *arguments)
{
  JSCContext *context = get_global_context();
  if (arguments->len < 2) {
    return NULL;
  }

  JSCValue *jsc_path = arguments->pdata[0];
  JSCValue *jsc_only_images = arguments->pdata[1];

  JSCValue *value = jsc_value_new_array(context, G_TYPE_NONE);

  g_autofree gchar *path = js_value_to_string_or_null(jsc_path);
  path = g_strstrip(path);
  if (path == NULL || g_strcmp0(path, "") == 0) {
    return value;
  }

  if (g_strcmp0(path, "/") == 0) {
    return value;
  }
  if (g_strcmp0(g_utf8_substring(path, 0, 1), "./") == 0) {
    return value;
  }

  char resolved_path[PATH_MAX];
  if (realpath(path, resolved_path) == NULL) {
    /*printf("Path normalize error: '%s'\n", strerror(errno));*/
    return value;
  }

  struct stat path_stat;
  stat(resolved_path, &path_stat);
  if (!g_path_is_absolute(resolved_path) || !(S_ISDIR(path_stat.st_mode))) {
    /*printf("Not absolute nor a directory\n");*/
    return value;
  }

  gboolean allowed = false;

  for (guint i = 0; i < allowed_dirs->len; i++) {
    char *allowed_dir = allowed_dirs->pdata[i];
    if (strncmp(resolved_path, allowed_dir, strlen(allowed_dir)) == 0) {
      allowed = true;
      break;
    }
  }

  if (!allowed) {
    logger_error("Path \"%s\" is not allowed", resolved_path);
    return value;
  }

  DIR *dir;
  struct dirent *ent;
  dir = opendir(resolved_path);
  if (dir == NULL) {
    printf("Opendir error: '%s'\n", strerror(errno));
    return value;
  }

  GPtrArray *files = g_ptr_array_new();

  GRegex *regex = g_regex_new(".+\\.(jpe?g|png|gif|bmp|webp)", G_REGEX_CASELESS, 0, NULL);

  while ((ent = readdir(dir)) != NULL) {
    char *file_name = ent->d_name;
    if (g_strcmp0(file_name, ".") == 0 || g_strcmp0(file_name, "..") == 0) {
      continue;
    }

    char *file_path = g_build_path("/", resolved_path, file_name, NULL);
    /*printf("-> '%s'\n", file_name);*/

    if (jsc_value_is_boolean(jsc_only_images) && jsc_value_to_boolean(jsc_only_images)) {
      struct stat file_stat;
      stat(file_path, &file_stat);
      if (S_ISREG(file_stat.st_mode) && g_regex_match(regex, file_name, 0, NULL)) {
        /*printf("\tFile match!\n");*/
        g_ptr_array_add(files, jsc_value_new_string(context, file_path));
      }
    } else {
      g_ptr_array_add(files, jsc_value_new_string(context, file_path));
    }
    g_free(file_path);
  }
  closedir(dir);

  value = jsc_value_new_array_from_garray(context, files);
  g_ptr_array_free(files, true);

  return value;
}

void
handle_theme_utils_accessor(WebKitWebView *web_view, WebKitUserMessage *message)
{
  bridge_object_handle_accessor(ThemeUtils_object, web_view, message);
}

void
ThemeUtils_destroy()
{
  g_object_unref(ThemeUtils_object);
  g_ptr_array_free(allowed_dirs, true);
}

void
ThemeUtils_initialize()
{
  const struct JSCClassMethod ThemeUtils_methods[] = {
    { "dirlist", G_CALLBACK(ThemeUtils_dirlist_cb), G_TYPE_NONE },
  };

  ThemeUtils_object
      = bridge_object_new_full("theme_utils", NULL, 0, ThemeUtils_methods, G_N_ELEMENTS(ThemeUtils_methods));

  allowed_dirs = g_ptr_array_new();

  char resolved_path[PATH_MAX];
  realpath(greeter_config->greeter->theme, resolved_path);
  char *theme_dir = g_path_get_dirname(resolved_path);

  g_ptr_array_add(allowed_dirs, greeter_config->app->theme_dir);
  g_ptr_array_add(allowed_dirs, greeter_config->branding->background_images_dir);
  g_ptr_array_add(allowed_dirs, shared_data_directory);
  g_ptr_array_add(allowed_dirs, theme_dir);
  g_ptr_array_add(allowed_dirs, g_strdup(g_get_tmp_dir()));
}
