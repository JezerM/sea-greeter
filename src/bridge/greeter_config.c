#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <jsc/jsc.h>
#include <webkit2/webkit-web-extension.h>

#include "bridge/lightdm-objects.h"
#include "bridge/utils.h"
#include "settings.h"

ldm_object *GreeterConfig_object = NULL;

static JSCValue *
GreeterConfig_branding_getter_cb(ldm_object *instance, JSCValue *object)
{
  (void) object;
  JSCContext *context = instance->context;
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
GreeterConfig_greeter_getter_cb(ldm_object *instance, JSCValue *object)
{
  (void) object;
  JSCContext *context = instance->context;
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
GreeterConfig_features_getter_cb(ldm_object *instance, JSCValue *object)
{
  (void) object;
  JSCContext *context = instance->context;
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

/**
 * GreeterConfig Class constructor, should be called only once in sea-greeter's life
 */
static JSCValue *
GreeterConfig_constructor(JSCContext *context)
{
  return jsc_value_new_null(context);
}

void
GreeterConfig_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension)
{
  (void) web_page;
  (void) extension;

  load_configuration();

  JSCContext *js_context = webkit_frame_get_js_context_for_script_world(web_frame, world);
  JSCValue *global_object = jsc_context_get_global_object(js_context);

  if (GreeterConfig_object != NULL) {
    jsc_value_object_set_property(global_object, "greeter_config", GreeterConfig_object->value);
    return;
  }

  JSCClass *GreeterConfig_class = jsc_context_register_class(js_context, "__GreeterConfig", NULL, NULL, NULL);

  JSCValue *gc_constructor = jsc_class_add_constructor(
      GreeterConfig_class,
      NULL,
      G_CALLBACK(GreeterConfig_constructor),
      js_context,
      NULL,
      JSC_TYPE_VALUE,
      0,
      NULL);

  const struct JSCClassProperty GreeterConfig_properties[] = {
    { "branding", G_CALLBACK(GreeterConfig_branding_getter_cb), NULL, JSC_TYPE_VALUE },
    { "greeter", G_CALLBACK(GreeterConfig_greeter_getter_cb), NULL, JSC_TYPE_VALUE },
    { "features", G_CALLBACK(GreeterConfig_features_getter_cb), NULL, JSC_TYPE_VALUE },
    { NULL, NULL, NULL, 0 },
  };

  initialize_class_properties(GreeterConfig_class, GreeterConfig_properties);

  JSCValue *value = jsc_value_constructor_callv(gc_constructor, 0, NULL);
  GreeterConfig_object = malloc(sizeof *GreeterConfig_object);
  GreeterConfig_object->value = value;
  GreeterConfig_object->context = js_context;

  JSCValue *greeter_config_object = jsc_value_new_object(js_context, GreeterConfig_object, GreeterConfig_class);
  GreeterConfig_object->value = greeter_config_object;

  jsc_value_object_set_property(global_object, "greeter_config", greeter_config_object);
}
