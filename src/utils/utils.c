#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include <jsc/jsc.h>

const char *
g_variant_to_string(GVariant *variant)
{
  if (!g_variant_is_of_type(variant, G_VARIANT_TYPE_STRING))
    return NULL;
  const gchar *value = g_variant_get_string(variant, NULL);
  return value;
}
GPtrArray *
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

GVariant *
jsc_parameters_to_g_variant_array(JSCContext *context, const gchar *name, GPtrArray *parameters)
{
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

  GVariant *result = g_variant_new_array(G_VARIANT_TYPE_STRING, param_arr, G_N_ELEMENTS(param_arr));

  g_free(json_params);
  return result;
}

JSCValue *
g_variant_reply_to_jsc_value(JSCContext *context, GVariant *reply)
{
  if (reply == NULL) {
    return NULL;
  }
  const gchar *json_value = g_variant_get_string(reply, NULL);
  JSCValue *value = jsc_value_new_from_json(context, json_value);
  if (jsc_value_is_null(value)) {
    return NULL;
  }
  return value;
}
