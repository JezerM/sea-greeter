#include <stdio.h>
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

static void
jsc_g_ptr_array_free(gpointer data)
{
  g_object_unref(data);
}

GPtrArray *
jsc_array_to_g_ptr_array(JSCValue *jsc_array)
{
  if (!jsc_value_is_array(jsc_array)) {
    return NULL;
  }
  GPtrArray *array = g_ptr_array_new_with_free_func(jsc_g_ptr_array_free);
  JSCValue *jsc_array_length = jsc_value_object_get_property(jsc_array, "length");

  int length = jsc_value_to_int32(jsc_array_length);
  g_object_unref(jsc_array_length);

  for (int i = 0; i < length; i++) {
    g_ptr_array_add(array, jsc_value_object_get_property_at_index(jsc_array, i));
  }

  return array;
}

/**
 * Convert JSCValue parameters to GVariant
 * @param context The JSCContext
 * @param name Custom string to send, useful to execute a "name" method with given parameters
 * @param parameters A GPtrArray of JSCValue parameters
 */
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
  g_object_unref(jsc_params);
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
