#ifndef EXTENSION_LIGHTDM_SIGNAL
#define EXTENSION_LIGHTDM_SIGNAL 1

#include <glib-object.h>
#include <webkit2/webkit-web-extension.h>

#include "bridge/lightdm-objects.h"
#include "bridge/utils.h"

void LightDM_signal_connect(ldm_object *instance, GPtrArray *arguments);

JSCValue *LightDM_signal_new(JSCContext *js_context, const gchar *name);

void LightDM_signal_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension);
void initialize_object_signals(JSCContext *js_context, JSCValue *object, const struct JSCClassSignal signals[]);

#endif
