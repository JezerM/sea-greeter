#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <webkit2/webkit-web-extension.h>
#include <jsc/jsc.h>

#include "bridge/utils.h"
#include "bridge/lightdm-objects.h"

static JSCClass *LightDM_signal_class;
static JSCValue *ldm_signal_constructor;

static void
LightDM_signal_constructor() {}

void
LightDM_signal_connect(
        ldm_object *instance,
        GPtrArray *arguments
) {
    if (arguments->len <= 0) return;

    JSCValue *function = arguments->pdata[0];
    if (!jsc_value_is_function(function)) return;

    JSCValue *value = instance->value;
    JSCValue *array = jsc_value_object_get_property(
            value,
            "_callbacks"
            );
    gchar **properties = jsc_value_object_enumerate_properties(array);

    int i = 0;
    if (properties != NULL) {
        gchar *curr = properties[i];
        while (curr != NULL) {
            i++;
            curr = properties[i];
        }
    }

    jsc_value_object_set_property_at_index(
            array,
            i,
            function
            );

    g_strfreev(properties);
}
void
LightDM_signal_disconnect(
        ldm_object *instance,
        GPtrArray *arguments
) {
    if (arguments->len <= 0) return;

    JSCValue *function = arguments->pdata[0];
    if (!jsc_value_is_function(function)) return;

    JSCContext *js_context = instance->context;
    JSCValue *value = instance->value;
    JSCValue *array = jsc_value_object_get_property(
            value,
            "_callbacks"
            );
    gchar **properties = jsc_value_object_enumerate_properties(array);

    if (properties == NULL) return;

    JSCValue *new_array = jsc_value_new_array(js_context, G_TYPE_NONE);

    int i = 0;
    int j = 0;
    while (properties[i] != NULL) {
        JSCValue *obtained = jsc_value_object_get_property_at_index(array, i);
        if (obtained != function) {
            jsc_value_object_set_property_at_index(
                    new_array,
                    j,
                    obtained
                    );
            j++;
        }
        i++;
    }

    jsc_value_object_set_property(
            value,
            "_callbacks",
            new_array
            );

    g_strfreev(properties);
}
void
LightDM_signal_emit(
        ldm_object *instance,
        GPtrArray *arguments
) {
    JSCValue *value = instance->value;
    JSCValue *array = jsc_value_object_get_property(
            value,
            "_callbacks"
            );
    gchar **properties = jsc_value_object_enumerate_properties(array);

    if (properties == NULL) return;

    int i = 0;
    while (properties[i] != NULL) {
        JSCValue *obtained = jsc_value_object_get_property_at_index(array, i);
        int length = arguments->len;
        JSCValue **parameters = (JSCValue **) arguments->pdata;
        (void) jsc_value_function_callv(
                obtained,
                length,
                parameters
                );
        i++;
    }
}

JSCValue *
LightDM_signal_new(JSCContext *js_context, const gchar *name) {
    JSCValue *value = jsc_value_constructor_callv(ldm_signal_constructor, 0, NULL);
    ldm_object *instance = malloc(sizeof *instance);
    instance->value = value;
    instance->context = js_context;

    JSCValue *ldm_signal_object = jsc_value_new_object(
            js_context,
            instance,
            LightDM_signal_class);

    instance->value = ldm_signal_object;

    JSCValue *signal_name = jsc_value_new_string(js_context, name);
    JSCValue *signal_callbacks = jsc_value_new_array(
            js_context,
            G_TYPE_NONE
            );

    jsc_value_object_set_property(ldm_signal_object,
            "_name",
            signal_name
            );
    jsc_value_object_set_property(ldm_signal_object,
            "_callbacks",
            signal_callbacks
            );

    gchar **properties = jsc_value_object_enumerate_properties(ldm_signal_object);

    int i = 0;
    if (properties != NULL) {
        gchar *curr = properties[i];
        while (curr != NULL) {
            /*printf("%d: %s\n", i, curr);*/
            i++;
            curr = properties[i];
        }
    }
    /*printf("PROPERTIES: %d\n", i);*/

    return ldm_signal_object;
}

void
LightDM_signal_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension
) {
  (void) web_page;
  (void) extension;

  JSCContext *js_context = webkit_frame_get_js_context_for_script_world(web_frame, world);
  LightDM_signal_class = jsc_context_register_class(
      js_context,
      "__LightDMSignal",
      NULL,
      NULL,
      NULL
      );
  ldm_signal_constructor = jsc_class_add_constructor(
          LightDM_signal_class, NULL,
          G_CALLBACK(LightDM_signal_constructor),
          NULL, NULL,
          JSC_TYPE_VALUE, 0);
  jsc_class_add_method_variadic(
          LightDM_signal_class,
          "connect",
          G_CALLBACK(LightDM_signal_connect),
          NULL,
          NULL,
          G_TYPE_NONE
          );
  jsc_class_add_method_variadic(
          LightDM_signal_class,
          "disconnect",
          G_CALLBACK(LightDM_signal_disconnect),
          NULL,
          NULL,
          G_TYPE_NONE
          );
  jsc_class_add_method_variadic(
          LightDM_signal_class,
          "emit",
          G_CALLBACK(LightDM_signal_emit),
          NULL,
          NULL,
          G_TYPE_NONE
          );
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
