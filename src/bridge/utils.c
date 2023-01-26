#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bridge/utils.h"

static JSCContext *Context = NULL;

JSCContext *
get_global_context()
{
  if (Context == NULL)
    Context = jsc_context_new();
  return Context;
}

/**
 * Converts a JSCValue to a string
 */
gchar *
js_value_to_string_or_null(JSCValue *value)
{
  if (!jsc_value_is_string(value))
    return NULL;
  return jsc_value_to_string(value);
}

/**
 * Get index position of *find* inside *source*
 */
int
string_get_index_of(const char *source, const char *find)
{
  int ind = -1;
  char *found = strstr(source, find);
  if (found != NULL) {
    ind = found - source;
  }
  return ind;
}

/**
 * Get index position of last ocurrence of *find* inside *source*
 */
int
string_get_last_index_of(const char *source, const char *find)
{
  if (source == NULL || find == NULL)
    return -1;
  int index = -1;

  char *source_rev = g_utf8_strreverse(source, -1);
  char *find_rev = g_utf8_strreverse(find, -1);

  int index_rev = string_get_index_of(source_rev, find_rev);
  if (index_rev != -1) {
    index = strlen(source) - index_rev;
  }

  g_free(source_rev);
  g_free(find_rev);
  return index;
}

/**
 * Initialize the properties of a class
 */
void
initialize_class_properties(JSCClass *class, const struct JSCClassProperty properties[])
{
  int i = 0;
  struct JSCClassProperty current = properties[i];
  while (current.name != NULL) {
    switch (current.property_type) {
      case JSC_TYPE_VALUE_POST:
        current.property_type = JSC_TYPE_VALUE;
        break;
    }
    jsc_class_add_property(class, current.name, current.property_type, current.getter, current.setter, NULL, NULL);
    i++;
    current = properties[i];
  }
}

/**
 * Initialize the properties of a class
 */
void
initialize_class_methods(JSCClass *class, const struct JSCClassMethod methods[])
{
  int i = 0;
  struct JSCClassMethod current = methods[i];
  while (current.name != NULL) {
    switch (current.return_type) {
      case JSC_TYPE_VALUE_POST:
        current.return_type = JSC_TYPE_VALUE;
        break;
    }
    jsc_class_add_method_variadic(class, current.name, current.callback, NULL, NULL, current.return_type);
    i++;
    current = methods[i];
  }
}
