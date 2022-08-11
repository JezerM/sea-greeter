#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit-web-extension.h>

#include "extension/lightdm-signal.h"
#include "utils/ipc-renderer.h"
#include "utils/utils.h"

static WebKitWebPage *WebPage;
static ldm_object *GreeterComm_object;

static void *
GreeterComm_broadcast_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  GVariant *parameters = jsc_parameters_to_g_variant_array(context, "broadcast", arguments);
  WebKitUserMessage *message = webkit_user_message_new("greeter_comm", parameters);

  webkit_web_page_send_message_to_view(WebPage, message, NULL, NULL, NULL);

  return NULL;
}

static gboolean
handle_comm_broadcast(WebKitWebPage *web_page, WebKitUserMessage *message)
{
  (void) web_page;

  const char *name = webkit_user_message_get_name(message);
  if (g_strcmp0(name, "greeter_comm") != 0)
    return false;

  GVariant *msg_param = webkit_user_message_get_parameters(message);
  if (!g_variant_is_of_type(msg_param, G_VARIANT_TYPE_ARRAY)) {
    return false;
  }
  int parameters_length = g_variant_n_children(msg_param);
  if (parameters_length == 0 || parameters_length > 2) {
    return false;
  }

  JSCContext *context = GreeterComm_object->context;
  JSCValue *parameters = NULL;

  GVariant *method_var = g_variant_get_child_value(msg_param, 0);
  GVariant *params_var = g_variant_get_child_value(msg_param, 1);

  const gchar *method = g_variant_to_string(method_var);
  const gchar *json_params = g_variant_to_string(params_var);

  if (method == NULL) {
    return false;
  }
  if (g_strcmp0(method, "_emit") != 0) {
    return false;
  }
  parameters = jsc_value_new_from_json(context, json_params);
  JSCValue *data = jsc_value_object_get_property_at_index(parameters, 0);

  JSCValue *global_object = jsc_context_get_global_object(context);
  JSCValue *dispatch_event = jsc_value_object_get_property(global_object, "dispatchEvent");

  JSCValue *event_class = jsc_value_object_get_property(global_object, "Event");
  JSCValue *broadcast_event
      = jsc_value_constructor_call(event_class, G_TYPE_STRING, "GreeterBroadcastEvent", G_TYPE_NONE);

  jsc_value_object_set_property(broadcast_event, "window", jsc_value_new_null(context));
  jsc_value_object_set_property(broadcast_event, "data", data);

  (void) jsc_value_function_call(dispatch_event, JSC_TYPE_VALUE, broadcast_event, G_TYPE_NONE);

  return true;
}

static gboolean
web_page_user_message_received(WebKitWebPage *web_page, WebKitUserMessage *message, gpointer user_data)
{
  (void) user_data;
  /*const char *name = webkit_user_message_get_name(message);*/
  /*printf("Got web_view message: '%s'\n", name);*/

  return handle_comm_broadcast(web_page, message);
}

static JSCValue *
GreeterComm_constructor(JSCContext *context)
{
  return jsc_value_new_null(context);
}

/**
 * Initialize GreeterComm
 */
void
GreeterComm_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension)
{
  (void) extension;

  WebPage = web_page;

  JSCContext *js_context = webkit_frame_get_js_context_for_script_world(web_frame, world);
  JSCValue *global_object = jsc_context_get_global_object(js_context);

  if (GreeterComm_object != NULL) {
    jsc_value_object_set_property(global_object, "greeter_comm", GreeterComm_object->value);
    return;
  }

  g_signal_connect(web_page, "user-message-received", G_CALLBACK(web_page_user_message_received), NULL);

  const struct JSCClassMethod Comm_methods[] = {
    { "broadcast", G_CALLBACK(GreeterComm_broadcast_cb), G_TYPE_NONE },
    { NULL, NULL, 0 },
  };

  JSCClass *Comm_class = jsc_context_register_class(js_context, "__GreeterComm", NULL, NULL, NULL);
  JSCValue *gc_constructor = jsc_class_add_constructor(
      Comm_class,
      NULL,
      G_CALLBACK(GreeterComm_constructor),
      js_context,
      NULL,
      JSC_TYPE_VALUE,
      0,
      NULL);

  /*initialize_class_properties(Comm_class, LightDM_properties);*/
  initialize_class_methods(Comm_class, Comm_methods);

  JSCValue *value = jsc_value_constructor_callv(gc_constructor, 0, NULL);
  GreeterComm_object = malloc(sizeof *GreeterComm_object);
  GreeterComm_object->value = value;
  GreeterComm_object->context = js_context;

  JSCValue *comm_object = jsc_value_new_object(js_context, GreeterComm_object, Comm_class);

  GreeterComm_object->value = comm_object;

  jsc_value_object_set_property(global_object, "greeter_comm", comm_object);

  /*g_signal_connect(web_page, "document-loaded", G_CALLBACK(ready_event_loaded), js_context);*/
}
