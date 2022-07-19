#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <jsc/jsc.h>
#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit2.h>

#include "bridge/lightdm-objects.h"
#include "bridge/utils.h"
#include "browser.h"
#include "lightdm/layout.h"
#include "logger.h"
#include "settings.h"
#include "utils/utils.h"

static JSCVirtualMachine *VirtualMachine = NULL;
static JSCContext *Context = NULL;

extern GPtrArray *greeter_browsers;

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

static BridgeObject GreeterConfig_object;

static JSCValue *
GreeterConfig_branding_getter_cb()
{
  JSCContext *context = get_global_context();
  JSCValue *value = jsc_value_new_object(context, NULL, NULL);

  const gchar *background_images_dir = greeter_config->branding->background_images_dir->str;
  const gchar *logo_image = greeter_config->branding->logo_image->str;
  const gchar *user_image = greeter_config->branding->user_image->str;

  jsc_value_object_set_property(value, "background_images_dir", jsc_value_new_string(context, background_images_dir));
  jsc_value_object_set_property(value, "logo_image", jsc_value_new_string(context, logo_image));
  jsc_value_object_set_property(value, "user_image", jsc_value_new_string(context, user_image));
  return value;
}

static JSCValue *
GreeterConfig_greeter_getter_cb()
{
  JSCContext *context = get_global_context();
  JSCValue *value = jsc_value_new_object(context, NULL, NULL);

  const gboolean debug_mode = greeter_config->greeter->debug_mode;
  const gboolean detect_theme_errors = greeter_config->greeter->detect_theme_errors;
  const gint screensaver_timeout = greeter_config->greeter->screensaver_timeout;
  const gboolean secure_mode = greeter_config->greeter->secure_mode;
  const gchar *theme = greeter_config->greeter->theme->str;
  const gchar *icon_theme = greeter_config->greeter->icon_theme->str;
  const gchar *time_language = greeter_config->greeter->time_language->str;

  jsc_value_object_set_property(value, "debug_mode", jsc_value_new_boolean(context, debug_mode));
  jsc_value_object_set_property(value, "detect_theme_errors", jsc_value_new_boolean(context, detect_theme_errors));
  jsc_value_object_set_property(value, "screensaver_timeout", jsc_value_new_number(context, screensaver_timeout));
  jsc_value_object_set_property(value, "secure_mode", jsc_value_new_boolean(context, secure_mode));
  jsc_value_object_set_property(value, "theme", jsc_value_new_string(context, theme));
  jsc_value_object_set_property(value, "icon_theme", jsc_value_new_string(context, icon_theme));
  jsc_value_object_set_property(value, "time_language", jsc_value_new_string(context, time_language));
  return value;
}

static JSCValue *
GreeterConfig_features_getter_cb()
{
  JSCContext *context = get_global_context();
  JSCValue *value = jsc_value_new_object(context, NULL, NULL);

  const gboolean battery = greeter_config->features->battery;

  jsc_value_object_set_property(value, "battery", jsc_value_new_boolean(context, battery));

  JSCValue *backlight = jsc_value_new_object(context, NULL, NULL);

  const gboolean backlight_enabled = greeter_config->features->backlight->enabled;
  const gint backlight_value = greeter_config->features->backlight->value;
  const gint backlight_steps = greeter_config->features->backlight->steps;

  jsc_value_object_set_property(backlight, "enabled", jsc_value_new_boolean(context, backlight_enabled));
  jsc_value_object_set_property(backlight, "value", jsc_value_new_number(context, backlight_value));
  jsc_value_object_set_property(backlight, "steps", jsc_value_new_number(context, backlight_steps));

  jsc_value_object_set_property(value, "backlight", backlight);
  return value;
}

static JSCValue *
GreeterConfig_layouts_getter_cb()
{
  JSCContext *context = get_global_context();

  GList *layouts = lightdm_get_layouts();
  GList *config_layouts = greeter_config->layouts;
  GPtrArray *final = g_ptr_array_new();

  GList *ldm_layout = layouts;
  while (ldm_layout != NULL) {
    GList *conf_layout = config_layouts;
    while (conf_layout != NULL) {
      GString *str = g_string_new(conf_layout->data);
      g_string_replace(str, " ", "\t", 0);

      LightDMLayout *lay = ldm_layout->data;
      if (g_strcmp0(lightdm_layout_get_name(lay), str->str) == 0) {
        JSCValue *val = LightDMLayout_to_JSCValue(context, lay);
        if (val != NULL)
          g_ptr_array_add(final, val);
      }
      g_string_free(str, true);

      conf_layout = conf_layout->next;
    }

    ldm_layout = ldm_layout->next;
  }

  return jsc_value_new_array_from_garray(context, final);
}

static char *
g_variant_to_string(GVariant *variant)
{
  if (!g_variant_is_of_type(variant, G_VARIANT_TYPE_STRING))
    return NULL;
  const gchar *value = g_variant_get_string(variant, NULL);
  return g_strdup(value);
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
handle_greeter_config_property(WebKitUserMessage *message, const gchar *method, GPtrArray *parameters)
{
  (void) parameters;

  int i = 0;
  struct JSCClassProperty *current = GreeterConfig_object.properties->pdata[i];
  while (current->name != NULL) {
    /*printf("Current: %d - %s\n", i, current->name);*/
    if (g_strcmp0(current->name, method) == 0) {

      if (parameters->len > 0) {
        JSCValue *param = parameters->pdata[0];
        ((void (*)(JSCValue *)) current->setter)(param);
        WebKitUserMessage *empty_msg = webkit_user_message_new("", NULL);
        webkit_user_message_send_reply(message, empty_msg);
        break;
      }

      JSCValue *jsc_value = ((JSCValue * (*) (void) ) current->getter)();
      const gchar *json_value = jsc_value_to_json(jsc_value, 0);
      /*printf("JSON value: '%s'\n", json_value);*/

      GVariant *value = g_variant_new_string(json_value);
      WebKitUserMessage *reply = webkit_user_message_new("reply", value);

      webkit_user_message_send_reply(message, reply);
      break;
    }
    i++;
    current = GreeterConfig_object.properties->pdata[i];
  }
}

void
handle_greeter_config_accessor(WebKitWebView *web_view, WebKitUserMessage *message)
{
  (void) web_view;
  const char *name = webkit_user_message_get_name(message);
  if (g_strcmp0(name, "greeter_config") != 0)
    return;

  WebKitUserMessage *empty_msg = webkit_user_message_new("", NULL);
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
  char *method = NULL;
  JSCValue *parameters = NULL;

  GVariant *method_var = g_variant_get_child_value(msg_param, 0);
  GVariant *params_var = g_variant_get_child_value(msg_param, 1);

  method = g_variant_to_string(method_var);
  const gchar *json_params = g_variant_to_string(params_var);
  parameters = jsc_value_new_from_json(context, json_params);
  /*printf("Handling: '%s'\n", method);*/
  /*printf("JSON params: '%s'\n", json_params);*/

  if (method == NULL) {
    webkit_user_message_send_reply(message, empty_msg);
    return;
  }

  GPtrArray *g_array = jsc_array_to_g_ptr_array(parameters);

  handle_greeter_config_property(message, method, g_array);

  g_free(method);
  g_ptr_array_free(g_array, true);
}

void
GreeterConfig_initialize()
{
  const struct JSCClassProperty GreeterConfig_properties[] = {
    { "branding", G_CALLBACK(GreeterConfig_branding_getter_cb), NULL, JSC_TYPE_VALUE },
    { "greeter", G_CALLBACK(GreeterConfig_greeter_getter_cb), NULL, JSC_TYPE_VALUE },
    { "features", G_CALLBACK(GreeterConfig_features_getter_cb), NULL, JSC_TYPE_VALUE },
    { "layouts", G_CALLBACK(GreeterConfig_layouts_getter_cb), NULL, JSC_TYPE_VALUE },
    { NULL, NULL, NULL, 0 },
  };

  GPtrArray *gc_properties = g_ptr_array_new_full(G_N_ELEMENTS(GreeterConfig_properties), NULL);
  for (gsize i = 0; i < G_N_ELEMENTS(GreeterConfig_properties); i++) {
    struct JSCClassProperty *prop = malloc(sizeof *prop);
    prop->name = GreeterConfig_properties[i].name;
    prop->property_type = GreeterConfig_properties[i].property_type;
    prop->getter = GreeterConfig_properties[i].getter;
    prop->setter = GreeterConfig_properties[i].setter;
    g_ptr_array_add(gc_properties, prop);
  }

  VirtualMachine = jsc_virtual_machine_new();

  GreeterConfig_object.properties = gc_properties;
  GreeterConfig_object.methods = g_ptr_array_new();
}
