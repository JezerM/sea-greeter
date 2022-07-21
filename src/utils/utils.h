#ifndef EXTENSION_UTILS_H
#define EXTENSION_UTILS_H

#include <glib.h>
#include <jsc/jsc.h>

const char *
g_variant_to_string(GVariant *variant);

GPtrArray *
jsc_array_to_g_ptr_array(JSCValue *jsc_array);

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
