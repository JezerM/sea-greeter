#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <jsc/jsc.h>

#include "bridge/lightdm-objects.h"
#include "bridge/utils.h"
#include "logger.h"
#include "settings.h"

#include "utils/ipc-renderer.h"

static WebKitWebPage *WebPage = NULL;
ldm_object *ThemeUtils_object = NULL;

static void *
jsc_callback_call(JSCContext *context, JSCValue *callback, JSCValue *value)
{
  JSCValue *jsc_console = jsc_context_get_value(context, "console");
  if (!jsc_value_is_function(callback)) {
    (void) jsc_value_object_invoke_method(
        jsc_console,
        "error",
        G_TYPE_STRING,
        "theme_utils.dirlist(): callback is not a function",
        G_TYPE_NONE);
    return NULL;
  }

  (void) jsc_value_function_call(callback, JSC_TYPE_VALUE, value, G_TYPE_NONE);

  return NULL;
}

static void *
ThemeUtils_dirlist_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;
  if (arguments->len < 3) {
    return NULL;
  }
  JSCValue *jsc_console = jsc_context_get_value(context, "console");

  JSCValue *jsc_path = arguments->pdata[0];
  JSCValue *jsc_only_images = arguments->pdata[1];
  JSCValue *jsc_callback = arguments->pdata[2];

  JSCValue *value = jsc_value_new_array(context, G_TYPE_NONE);

  gchar *path = js_value_to_string_or_null(jsc_path);
  if (path == NULL) {
    (void) jsc_value_object_invoke_method(
        jsc_console,
        "error",
        G_TYPE_STRING,
        "theme_utils.dirlist(): path must be a non-empty string!",
        G_TYPE_NONE);
    return jsc_callback_call(context, jsc_callback, value);
  }

  if (g_strcmp0(path, "/") == 0) {
    return jsc_callback_call(context, jsc_callback, value);
  }
  if (g_strcmp0(g_utf8_substring(path, 0, 1), "./") == 0) {
    return jsc_callback_call(context, jsc_callback, value);
  }

  char resolved_path[PATH_MAX];
  if (realpath(path, resolved_path) == NULL) {
    /*printf("Path normalize error: '%s'\n", strerror(errno));*/
    return jsc_callback_call(context, jsc_callback, value);
  }

  struct stat path_stat;
  stat(resolved_path, &path_stat);
  if (!g_path_is_absolute(resolved_path) || !(S_ISDIR(path_stat.st_mode))) {
    /*printf("Not absolute nor a directory\n");*/
    return jsc_callback_call(context, jsc_callback, value);
  }

  // if (allowed)

  DIR *dir;
  struct dirent *ent;
  dir = opendir(resolved_path);
  if (dir == NULL) {
    printf("Opendir error: '%s'\n", strerror(errno));
    return jsc_callback_call(context, jsc_callback, value);
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

  jsc_callback_call(context, jsc_callback, value);
  return NULL;
}

char *time_language = NULL;

static JSCValue *
ThemeUtils_get_current_localized_date_cb(ldm_object *instance, GPtrArray *arguments)
{
  (void) arguments;
  JSCContext *context = instance->context;

  JSCValue *Intl = jsc_context_get_value(context, "Intl");
  JSCValue *DateTimeFormat = jsc_value_object_get_property(Intl, "DateTimeFormat");

  GPtrArray *locales = g_ptr_array_new();

  if (time_language == NULL) {
    JSCValue *jsc_time_language = jsc_context_evaluate(context, "greeter_config.greeter.time_language", 36);
    time_language = jsc_value_to_string(jsc_time_language);
  }

  if (g_strcmp0(time_language, "") != 0) {
    g_ptr_array_add(locales, jsc_value_new_string(context, time_language));
  }

  JSCValue *jsc_locales = jsc_value_new_array_from_garray(context, locales);

  JSCValue *two_digit = jsc_value_new_string(context, "2-digit");
  JSCValue *options_date = jsc_value_new_object(context, NULL, NULL);
  jsc_value_object_set_property(options_date, "day", two_digit);
  jsc_value_object_set_property(options_date, "month", two_digit);
  jsc_value_object_set_property(options_date, "year", two_digit);

  JSCValue *fmtDate
      = jsc_value_function_call(DateTimeFormat, JSC_TYPE_VALUE, jsc_locales, JSC_TYPE_VALUE, options_date, G_TYPE_NONE);

  JSCValue *Now = jsc_context_evaluate(context, "new Date()", 10);

  JSCValue *date = jsc_value_object_invoke_method(fmtDate, "format", JSC_TYPE_VALUE, Now, G_TYPE_NONE);

  return date;
}

static JSCValue *
ThemeUtils_get_current_localized_time_cb(ldm_object *instance, GPtrArray *arguments)
{
  (void) arguments;
  JSCContext *context = instance->context;

  JSCValue *Intl = jsc_context_get_value(context, "Intl");
  JSCValue *DateTimeFormat = jsc_value_object_get_property(Intl, "DateTimeFormat");

  GPtrArray *locales = g_ptr_array_new();

  if (time_language == NULL) {
    JSCValue *jsc_time_language = jsc_context_evaluate(context, "greeter_config.greeter.time_language", 36);
    time_language = jsc_value_to_string(jsc_time_language);
  }
  /*printf("Time language: '%s'\n", time_language);*/

  if (g_strcmp0(time_language, "") != 0) {
    g_ptr_array_add(locales, jsc_value_new_string(context, time_language));
  }

  JSCValue *jsc_locales = jsc_value_new_array_from_garray(context, locales);

  JSCValue *two_digit = jsc_value_new_string(context, "2-digit");
  JSCValue *options_date = jsc_value_new_object(context, NULL, NULL);
  jsc_value_object_set_property(options_date, "hour", two_digit);
  jsc_value_object_set_property(options_date, "minute", two_digit);

  JSCValue *fmtDate
      = jsc_value_function_call(DateTimeFormat, JSC_TYPE_VALUE, jsc_locales, JSC_TYPE_VALUE, options_date, G_TYPE_NONE);

  JSCValue *Now = jsc_context_evaluate(context, "new Date()", 10);

  JSCValue *date = jsc_value_object_invoke_method(fmtDate, "format", JSC_TYPE_VALUE, Now, G_TYPE_NONE);

  return date;
}

/**
 * ThemeUtils Class constructor, should be called only once in sea-greeter's life
 */
static JSCValue *
ThemeUtils_constructor(JSCContext *context)
{
  return jsc_value_new_null(context);
}

void
ThemeUtils_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension)
{
  WebPage = web_page;
  (void) extension;

  JSCContext *js_context = webkit_frame_get_js_context_for_script_world(web_frame, world);
  JSCValue *global_object = jsc_context_get_global_object(js_context);

  if (ThemeUtils_object != NULL) {
    jsc_value_object_set_property(global_object, "theme_utils", ThemeUtils_object->value);
    return;
  }

  JSCClass *ThemeUtils_class = jsc_context_register_class(js_context, "__GreeterConfig", NULL, NULL, NULL);

  JSCValue *gc_constructor = jsc_class_add_constructor(
      ThemeUtils_class,
      NULL,
      G_CALLBACK(ThemeUtils_constructor),
      js_context,
      NULL,
      JSC_TYPE_VALUE,
      0,
      NULL);

  const struct JSCClassMethod ThemeUtils_methods[] = {
    { "dirlist", G_CALLBACK(ThemeUtils_dirlist_cb), G_TYPE_NONE },
    { "get_current_localized_date", G_CALLBACK(ThemeUtils_get_current_localized_date_cb), JSC_TYPE_VALUE },
    { "get_current_localized_time", G_CALLBACK(ThemeUtils_get_current_localized_time_cb), JSC_TYPE_VALUE },
    { NULL, NULL, 0 },
  };

  initialize_class_methods(ThemeUtils_class, ThemeUtils_methods);

  JSCValue *value = jsc_value_constructor_callv(gc_constructor, 0, NULL);
  ThemeUtils_object = malloc(sizeof *ThemeUtils_object);
  ThemeUtils_object->value = value;
  ThemeUtils_object->context = js_context;

  JSCValue *theme_utils_object = jsc_value_new_object(js_context, ThemeUtils_object, ThemeUtils_class);
  ThemeUtils_object->value = theme_utils_object;

  jsc_value_object_set_property(global_object, "theme_utils", theme_utils_object);
}
