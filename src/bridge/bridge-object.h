#ifndef BRIDGE_OBJECT_H
#define BRIDGE_OBJECT_H 1

#include <webkit2/webkit2.h>
#include <jsc/jsc.h>

#include "bridge/utils.h"

G_BEGIN_DECLS

#define BRIDGE_TYPE_OBJECT bridge_object_get_type()
G_DECLARE_FINAL_TYPE(BridgeObject, bridge_object, BRIDGE, OBJECT, GObject)

struct _BridgeObject {
    GObject parent_instance;

    gchar *name;
    GPtrArray *properties;
    GPtrArray *methods;
};

void
bridge_object_handle_accessor(BridgeObject *self, WebKitWebView *web_view, WebKitUserMessage *message);

BridgeObject *bridge_object_new(const gchar *name);

BridgeObject *
bridge_object_new_full(
    const gchar *name,
    const struct JSCClassProperty *properties,
    guint properties_length,
    const struct JSCClassMethod *methods,
    guint methods_length);

G_END_DECLS

#endif

