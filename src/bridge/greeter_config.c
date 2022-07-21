#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <jsc/jsc.h>
#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit2.h>

#include "bridge/bridge-object.h"
#include "bridge/lightdm-objects.h"
#include "bridge/utils.h"

#include "logger.h"
#include "settings.h"
#include "utils/utils.h"

static BridgeObject *GreeterConfig_object = NULL;

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
  JSCValue *value = jsc_value_new_array_from_garray(context, final);
  g_ptr_array_free(final, true);
  return value;
}

void
handle_greeter_config_accessor(WebKitWebView *web_view, WebKitUserMessage *message)
{
  bridge_object_handle_accessor(GreeterConfig_object, web_view, message);
}

void
GreeterConfig_destroy()
{
  g_object_unref(GreeterConfig_object);
}

void
GreeterConfig_initialize()
{
  const struct JSCClassProperty GreeterConfig_properties[] = {
    { "branding", G_CALLBACK(GreeterConfig_branding_getter_cb), NULL, JSC_TYPE_VALUE },
    { "greeter", G_CALLBACK(GreeterConfig_greeter_getter_cb), NULL, JSC_TYPE_VALUE },
    { "features", G_CALLBACK(GreeterConfig_features_getter_cb), NULL, JSC_TYPE_VALUE },
    { "layouts", G_CALLBACK(GreeterConfig_layouts_getter_cb), NULL, JSC_TYPE_VALUE },
  };

  GreeterConfig_object = bridge_object_new_full(
      "greeter_config",
      GreeterConfig_properties,
      G_N_ELEMENTS(GreeterConfig_properties),
      NULL,
      0);
}
