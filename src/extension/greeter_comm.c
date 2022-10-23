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
static JSCContext *global_context;
static JSCValue *Comm_value;
/*static ldm_object *GreeterComm_object;*/

typedef struct _Comm Comm;
struct _Comm {
  JSCValue *_window_metadata;
  JSCValue *_ready_promise;
  JSCValue *_ready;
};

static JSCValue *
GreeterComm_window_metadata_cb(Comm *instance)
{
  JSCContext *context = jsc_context_get_current();

  if (instance->_window_metadata != NULL && !jsc_value_is_undefined(instance->_window_metadata))
    return g_object_ref(instance->_window_metadata);

  jsc_context_throw(context, "window_metadata not available, did you wait for the GreeterReady event?");

  return jsc_value_new_null(context);
}

static void
GreeterComm_broadcast_cb(Comm *instance, GPtrArray *arguments)
{
  JSCContext *context = jsc_context_get_current();

  GVariant *parameters = jsc_parameters_to_g_variant_array(context, "broadcast", arguments);
  WebKitUserMessage *message = webkit_user_message_new("greeter_comm", parameters);

  webkit_web_page_send_message_to_view(WebPage, message, NULL, NULL, NULL);
}
static JSCValue *
GreeterComm_when_ready_cb(Comm *instance, GPtrArray *arguments)
{
  (void) arguments;
  return g_object_ref(instance->_ready_promise);
}

/**
 * Handle broadcast message
 */
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

  JSCContext *context = global_context;
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

/**
 * Callback to the window metadata request
 */
static void
request_window_metadata_cb(GObject *web_page, GAsyncResult *res, gpointer user_data)
{
  Comm *instance = user_data;

  WebKitUserMessage *response = webkit_web_page_send_message_to_view_finish(WEBKIT_WEB_PAGE(web_page), res, NULL);

  instance->_window_metadata = jsc_value_new_string(global_context, "UWU");

  /*printf("instance->_ready: %p - %d\n", instance->_ready, jsc_value_is_function(instance->_ready));*/

  if (instance->_ready != NULL && jsc_value_is_function(instance->_ready))
    (void) jsc_value_function_call(instance->_ready, G_TYPE_NONE, NULL);
}

/**
 * Internal method that requests the window metadata to the backend
 */
static void
GreeterComm_request_window_metadata(Comm *instance)
{
  ipc_renderer_send_message_with_arguments(
      WebPage,
      global_context,
      "greeter_comm",
      "window_metadata",
      NULL,
      request_window_metadata_cb,
      instance);
}

/**
 * Set _ready property as the resolve function
 */
static void
GreeterComm_promise_resolve(GPtrArray *arguments, Comm *instance)
{
  if (arguments->len == 0)
    return;
  JSCValue *resolve = arguments->pdata[0];

  instance->_ready = g_object_ref(resolve);
}
static Comm *
GreeterComm_constructor(JSCContext *context)
{
  Comm *instance = g_malloc(sizeof *instance);
  instance->_window_metadata = NULL;
  instance->_ready = NULL;

  // Construct a new Promise with GreeterComm_promise_resolve as the promise function
  JSCValue *global_object = jsc_context_get_global_object(global_context);
  JSCValue *promise_constructor = jsc_value_object_get_property(global_object, "Promise");

  JSCValue *promise_callback = jsc_value_new_function_variadic(
      context,
      "promise_resolve",
      G_CALLBACK(GreeterComm_promise_resolve),
      instance,
      NULL,
      G_TYPE_NONE);

  JSCValue *promise = jsc_value_constructor_call(promise_constructor, JSC_TYPE_VALUE, promise_callback, G_TYPE_NONE);

  instance->_ready_promise = promise;

  GreeterComm_request_window_metadata(instance);

  return instance;
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

  global_context = webkit_frame_get_js_context_for_script_world(web_frame, world);
  JSCValue *global_object = jsc_context_get_global_object(global_context);

  if (Comm_value != NULL) {
    jsc_value_object_set_property(global_object, "greeter_comm", Comm_value);
    return;
  }

  g_signal_connect(web_page, "user-message-received", G_CALLBACK(web_page_user_message_received), NULL);

  const struct JSCClassProperty Comm_properties[] = {
    { "window_metadata", G_CALLBACK(GreeterComm_window_metadata_cb), NULL, JSC_TYPE_VALUE },
    { NULL, NULL, NULL, 0 },
  };

  const struct JSCClassMethod Comm_methods[] = {
    { "broadcast", G_CALLBACK(GreeterComm_broadcast_cb), G_TYPE_NONE },
    { "whenReady", G_CALLBACK(GreeterComm_when_ready_cb), JSC_TYPE_VALUE },
    { NULL, NULL, 0 },
  };

  JSCClass *Comm_class = jsc_context_register_class(global_context, "__GreeterComm", NULL, NULL, NULL);
  JSCValue *gc_constructor = jsc_class_add_constructor(
      Comm_class,
      NULL,
      G_CALLBACK(GreeterComm_constructor),
      global_context,
      NULL,
      G_TYPE_POINTER,
      0,
      NULL);
  /*jsc_value_object_set_property(global_object, jsc_class_get_name(Comm_class), gc_constructor);*/

  initialize_class_properties(Comm_class, Comm_properties);
  initialize_class_methods(Comm_class, Comm_methods);

  Comm_value = jsc_value_constructor_callv(gc_constructor, 0, NULL);
  jsc_value_object_set_property(global_object, "greeter_comm", Comm_value);
}
