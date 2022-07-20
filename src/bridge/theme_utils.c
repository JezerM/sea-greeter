#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <jsc/jsc.h>
#include <webkit2/webkit2.h>

#include "bridge/lightdm-objects.h"
#include "bridge/utils.h"
#include "logger.h"
#include "settings.h"

static JSCVirtualMachine *VirtualMachine = NULL;
static JSCContext *Context = NULL;

static GPtrArray *allowed_dirs = NULL;
extern GString *shared_data_directory;

static JSCContext *
get_global_context()
{
  if (Context == NULL)
    Context = jsc_context_new_with_virtual_machine(VirtualMachine);
  return Context;
}

typedef struct _BridgeObject {
  GPtrArray *properties;
  GPtrArray *methods;
} BridgeObject;

static BridgeObject ThemeUtils_object;

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

static const char *
g_variant_to_string(GVariant *variant)
{
  if (!g_variant_is_of_type(variant, G_VARIANT_TYPE_STRING))
    return NULL;
  const gchar *value = g_variant_get_string(variant, NULL);
  return value;
}
static GPtrArray *
jsc_array_to_g_ptr_array(JSCValue *jsc_array)
{
  if (!jsc_value_is_array(jsc_array)) {
    return NULL;
  }
  GPtrArray *array = g_ptr_array_new();
  JSCValue *jsc_array_length = jsc_value_object_get_property(jsc_array, "length");

  int length = jsc_value_to_int32(jsc_array_length);

  for (int i = 0; i < length; i++) {
    g_ptr_array_add(array, jsc_value_object_get_property_at_index(jsc_array, i));
  }

  return array;
}

static void
handle_theme_utils_method(WebKitUserMessage *message, const gchar *method, GPtrArray *parameters)
{
  int i = 0;
  struct JSCClassMethod *current = ThemeUtils_object.methods->pdata[i];
  while (current->name != NULL) {
    /*printf("Current: %d - %s\n", i, current->name);*/
    if (g_strcmp0(current->name, method) == 0) {
      JSCValue *jsc_value = ((JSCValue * (*) (GPtrArray *) ) current->callback)(parameters);
      gchar *json_value = jsc_value_to_json(jsc_value, 0);
      /*printf("JSON value: '%s'\n", json_value);*/

      GVariant *value = g_variant_new_string(json_value);
      WebKitUserMessage *reply = webkit_user_message_new("reply", value);

      webkit_user_message_send_reply(message, reply);
      g_free(json_value);
      break;
    }
    i++;
    current = ThemeUtils_object.methods->pdata[i];
  }
}

void
handle_theme_utils_accessor(WebKitWebView *web_view, WebKitUserMessage *message)
{
  (void) web_view;
  const char *name = webkit_user_message_get_name(message);
  if (g_strcmp0(name, "theme_utils") != 0)
    return;

  g_autoptr(WebKitUserMessage) empty_msg = webkit_user_message_new("", NULL);
  GVariant *msg_param = webkit_user_message_get_parameters(message);

  if (!g_variant_is_of_type(msg_param, G_VARIANT_TYPE_ARRAY)) {
    webkit_user_message_send_reply(message, empty_msg);
    return;
  }
  int parameters_length = g_variant_n_children(msg_param);
  if (parameters_length == 0 || parameters_length > 2) {
    webkit_user_message_send_reply(message, empty_msg);
    return;
  }

  JSCContext *context = get_global_context();
  JSCValue *parameters = NULL;

  GVariant *method_var = g_variant_get_child_value(msg_param, 0);
  GVariant *params_var = g_variant_get_child_value(msg_param, 1);

  const gchar *method = g_variant_to_string(method_var);
  const gchar *json_params = g_variant_to_string(params_var);
  parameters = jsc_value_new_from_json(context, json_params);
  /*printf("Handling: '%s'\n", method);*/
  /*printf("JSON params: '%s'\n", json_params);*/

  g_variant_unref(method_var);
  g_variant_unref(params_var);
  if (method == NULL) {
    webkit_user_message_send_reply(message, empty_msg);
    return;
  }

  GPtrArray *g_array = jsc_array_to_g_ptr_array(parameters);

  handle_theme_utils_method(message, method, g_array);

  g_ptr_array_free(g_array, true);
}

void
ThemeUtils_destroy()
{
  g_ptr_array_free(ThemeUtils_object.properties, true);
  g_ptr_array_free(ThemeUtils_object.methods, true);

  g_ptr_array_free(allowed_dirs, true);
}

void
ThemeUtils_initialize()
{
  const struct JSCClassMethod ThemeUtils_methods[] = {
    { "dirlist", G_CALLBACK(ThemeUtils_dirlist_cb), G_TYPE_NONE },
    { NULL, NULL, 0 },
  };

  GPtrArray *ldm_methods = g_ptr_array_new_full(G_N_ELEMENTS(ThemeUtils_methods), NULL);
  for (gsize i = 0; i < G_N_ELEMENTS(ThemeUtils_methods); i++) {
    struct JSCClassMethod *method = malloc(sizeof *method);
    method->name = ThemeUtils_methods[i].name;
    method->return_type = ThemeUtils_methods[i].return_type;
    method->callback = ThemeUtils_methods[i].callback;
    g_ptr_array_add(ldm_methods, method);
  }

  VirtualMachine = jsc_virtual_machine_new();

  ThemeUtils_object.properties = NULL;
  ThemeUtils_object.methods = ldm_methods;

  allowed_dirs = g_ptr_array_new();

  char resolved_path[PATH_MAX];
  realpath(greeter_config->greeter->theme->str, resolved_path);
  char *theme_dir = g_path_get_dirname(resolved_path);

  g_ptr_array_add(allowed_dirs, greeter_config->app->theme_dir->str);
  g_ptr_array_add(allowed_dirs, greeter_config->branding->background_images_dir->str);
  g_ptr_array_add(allowed_dirs, shared_data_directory);
  g_ptr_array_add(allowed_dirs, theme_dir);
  g_ptr_array_add(allowed_dirs, g_strdup(g_get_tmp_dir()));
}
