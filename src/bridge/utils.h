#ifndef BRIDGE_UTILS_H
#define BRIDGE_UTILS_H 1

#include <glib-object.h>
#include <webkit2/webkit-web-extension.h>
#include <JavaScriptCore/JavaScript.h>

#define G_TYPE_ARRAY_POST -1101

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

gchar *
js_value_to_string(JSCValue *value);

int
g_string_get_index_of(GString *source, GString *find);

int
g_string_get_last_index_of(GString *source, GString *find);

void
initialize_class_properties(JSCClass *class, struct JSCClassProperty properties[]);

void
initialize_class_methods(JSCClass *class, struct JSCClassMethod methods[]);

void
initialize_object_signals(
    JSCContext *js_context,
    JSCValue *object,
    struct JSCClassSignal signals[]
);
#endif
