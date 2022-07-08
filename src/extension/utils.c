#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <glib.h>
#include <jsc/jsc.h>

GVariant *
jsc_parameters_to_g_variant_array(
    JSCContext *context,
    const gchar *name,
    GPtrArray *parameters
) {
  JSCValue *jsc_params;
  if (parameters == NULL) {
    jsc_params = jsc_value_new_array(context, G_TYPE_NONE);
  } else {
    jsc_params = jsc_value_new_array_from_garray(context, parameters);
  }
  char *json_params = jsc_value_to_json(jsc_params, 0);
  GVariant *name_p = g_variant_new_string(name);
  GVariant *params = g_variant_new_string(json_params);

  GVariant *param_arr[] = { name_p, params };

  GVariant *result = g_variant_new_array(
      G_VARIANT_TYPE_STRING,
      param_arr,
      G_N_ELEMENTS(param_arr)
      );

  return result;
}

JSCValue *
g_variant_reply_to_jsc_value(
    JSCContext *context,
    GVariant *reply
) {
  if (reply == NULL) {
    return NULL;
  }
  const gchar *json_value = g_variant_get_string(reply, NULL);
  JSCValue *value = jsc_value_new_from_json(context, json_value);
  return value;
}
