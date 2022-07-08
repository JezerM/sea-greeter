#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "bridge/utils.h"
#include "bridge/lightdm-signal.h"

/**
 * Converts a JSCValue to a string
 */
gchar *
js_value_to_string_or_null(JSCValue *value) {
  if (!jsc_value_is_string(value)) return NULL;
  return jsc_value_to_string(value);
}

/**
 * Get index position of *find* inside *source*
 */
int
g_string_get_index_of(GString *source, GString *find) {
  gchar *found = strstr(source->str, find->str);
  if (found != NULL) return found - source->str;
  return -1;
}

/**
 * Get index position of last ocurrence of *find* inside *source*
 */
int
g_string_get_last_index_of(GString *source, GString *find) {
  int index = -1, tmp;
  GString *str = g_string_new(source->str);
  while ((tmp = g_string_get_index_of(str, find)) != -1) {
    str = g_string_erase(str, 0, tmp + find->len);
    index += tmp + find->len;
  }
  return index;
}

/**
 * Initialize the properties of a class
 */
void initialize_class_properties(
    JSCClass *class,
    const struct JSCClassProperty properties[])
{
  int i = 0;
  struct JSCClassProperty current = properties[i];
  while (current.name != NULL) {
    switch (current.property_type) {
      case JSC_TYPE_VALUE_POST:
        current.property_type = JSC_TYPE_VALUE;
        break;
    }
    jsc_class_add_property(
        class,
        current.name,
        current.property_type,
        current.getter,
        current.setter,
        NULL,
        NULL
        );
    i++;
    current = properties[i];
  }
}

/**
 * Initialize the properties of a class
 */
void initialize_class_methods(
    JSCClass *class,
    const struct JSCClassMethod methods[])
{
  int i = 0;
  struct JSCClassMethod current = methods[i];
  while (current.name != NULL) {
    switch (current.return_type) {
      case JSC_TYPE_VALUE_POST:
        current.return_type = JSC_TYPE_VALUE;
        break;
    }
    jsc_class_add_method_variadic(
        class,
        current.name,
        current.callback,
        NULL,
        NULL,
        current.return_type
        );
    i++;
    current = methods[i];
  }
}

/**
 * Initialize class signals
 */
void initialize_object_signals(
    JSCContext *js_context,
    JSCValue *object,
    const struct JSCClassSignal signals[])
{
  int i = 0;
  struct JSCClassSignal current = signals[i];
  while (current.name != NULL) {
    JSCValue *signal = LightDM_signal_new(js_context, current.name);
    jsc_value_object_set_property(
        object,
        current.name,
        signal
        );
    i++;
    current = signals[i];
  }
}
