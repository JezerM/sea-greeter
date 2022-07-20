#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit-web-extension.h>

#include "bridge/lightdm-objects.h"
#include "bridge/utils.h"

#include "logger.h"

#include "utils/ipc-renderer.h"
#include "utils/utils.h"

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
  JSCValue *jsc_callback = arguments->pdata[2];

  JSCValue *empty_value = jsc_value_new_array(context, G_TYPE_NONE);

  g_autofree gchar *path = js_value_to_string_or_null(jsc_path);
  path = g_strstrip(path);
  if (path == NULL || g_strcmp0(path, "") == 0) {
    (void) jsc_value_object_invoke_method(
        jsc_console,
        "error",
        G_TYPE_STRING,
        "theme_utils.dirlist(): path must be a non-empty string!",
        G_TYPE_NONE);
    return jsc_callback_call(context, jsc_callback, empty_value);
  }

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "theme_utils", "dirlist", arguments);
  if (reply == NULL) {
    return jsc_callback_call(context, jsc_callback, empty_value);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return jsc_callback_call(context, jsc_callback, value);
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
  g_ptr_array_free(locales, true);

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
  g_ptr_array_free(locales, true);

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
