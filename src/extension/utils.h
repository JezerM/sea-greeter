#ifndef EXTENSION_UTILS_H
#define EXTENSION_UTILS_H

#include <glib.h>
#include <jsc/jsc.h>

GVariant *
jsc_parameters_to_g_variant_array(
    JSCContext *context,
    const gchar *name,
    GPtrArray *parameters
);
JSCValue *
g_variant_reply_to_jsc_value(
    JSCContext *context,
    GVariant *reply
);
#endif
