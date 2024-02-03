#ifndef BRIDGE_UTILS_H
#define BRIDGE_UTILS_H 1

#include <glib-object.h>
#include <jsc/jsc.h>

#define JSC_TYPE_VALUE_POST -1101

struct JSCClassProperty {
  const gchar *name;
  GCallback getter;
  GCallback setter;
  GType property_type;
};
struct JSCClassMethod {
  const gchar *name;
  GCallback callback;
  GType return_type;
};
struct JSCClassSignal {
  const gchar *name;
};

JSCContext *get_global_context();

gchar *js_value_to_string_or_null(JSCValue *value);

int string_get_index_of(const char *source, const char *find);

int string_get_last_index_of(const char *source, const char *find);

void initialize_class_properties(JSCClass *class, const struct JSCClassProperty properties[]);

void initialize_class_methods(JSCClass *class, const struct JSCClassMethod methods[]);

void initialize_object_signals(JSCContext *js_context, JSCValue *object, const struct JSCClassSignal signals[]);
#endif
