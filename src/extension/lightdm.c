#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <webkit2/webkit-web-extension.h>
#include <lightdm-gobject-1/lightdm.h>

#include "settings.h"
#include "logger.h"
#include "bridge/lightdm-objects.h"
#include "bridge/utils.h"
#include "bridge/lightdm-signal.h"
#include "utils/ipc-renderer.h"
#include "extension/utils.h"

/*extern WebKitWebExtension *WebExtension;*/
/*extern guint64 page_id;*/
static WebKitWebPage *WebPage;

static JSCClass *LightDM_class;
static ldm_object *LightDM_object;
static JSCValue *ready_event;

/*static GString *shared_data_directory;*/

/* LightDM Class definitions */

static JSCValue *
LightDM_authenticate_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "authenticate",
      arguments
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_authenticate_as_guest_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "authenticate_as_guest",
      arguments
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_cancel_authentication_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "cancel_authentication",
      arguments
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_cancel_autologin_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "cancel_autologin",
      arguments
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_hibernate_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "hibernate",
      arguments
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_respond_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "respond",
      arguments
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_restart_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "restart",
      arguments
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_set_language_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "set_language",
      arguments
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_shutdown_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "shutdown",
      arguments
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_start_session_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "start_session",
      arguments
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_suspend_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "suspend",
      arguments
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}

/* LightDM properties */

static JSCValue *
LightDM_authentication_user_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "authentication_user",
      NULL
      );
  if (reply == NULL) {
    return jsc_value_new_null(context);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_autologin_guest_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "autologin_guest",
      NULL
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_autologin_timeout_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "autologin_timeout",
      NULL
      );
  if (reply == NULL) {
    return jsc_value_new_number(context, 0);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_autologin_user_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "autologin_user",
      NULL
      );
  if (reply == NULL) {
    return jsc_value_new_null(context);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_can_hibernate_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "can_hibernate",
      NULL
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_can_restart_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "can_restart",
      NULL
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_can_shutdown_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "can_shutdown",
      NULL
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_can_suspend_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "can_suspend",
      NULL
      );
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}

static JSCValue *
LightDM_brightness_getter_cb(ldm_object *instance) {
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "brightness",
      NULL
      );
  if (reply == NULL) {
    return jsc_value_new_number(context, -1);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_brightness_setter_cb(
    ldm_object *instance,
    JSCValue *object
) {
  JSCContext *context = instance->context;
  GPtrArray *arguments = g_ptr_array_new();
  g_ptr_array_add(arguments, object);

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(
      WebPage, context,
      "lightdm", "brightness",
      arguments
      );
  if (reply == NULL) {
    return jsc_value_new_number(context, -1);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}


static JSCValue *
LightDM_constructor(JSCContext *context) {
  return jsc_value_new_null(context);
}

static void
ready_event_loaded(WebKitWebPage *web_page, JSCContext *context) {
  (void) web_page;
  JSCValue *global_object = jsc_context_get_global_object(context);

  JSCValue *dispatch_event = jsc_value_object_get_property(global_object, "dispatchEvent");
  JSCValue *parameters[] = {
    ready_event,
    NULL
  };
  (void) jsc_value_function_callv(dispatch_event, 1, parameters);
}

/**
 * Initialize the LightDM environment
 */
void
LightDM_Test_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension
) {
  (void) extension;

  WebPage = web_page;

  JSCContext *js_context = webkit_frame_get_js_context_for_script_world(web_frame, world);
  JSCValue *global_object = jsc_context_get_global_object(js_context);

  if (LightDM_object != NULL) {
    jsc_value_object_set_property(
        global_object,
        "lightdm_test",
        LightDM_object->value
        );

    /*jsc_value_object_set_property(*/
        /*global_object,*/
        /*"_ready_event",*/
        /*ready_event*/
        /*);*/
    return;
  }

  LightDM_class = jsc_context_register_class(
      js_context,
      "__LightDMGreeterTest",
      NULL,
      NULL,
      NULL
      );
  JSCValue *ldm_constructor = jsc_class_add_constructor(
      LightDM_class, NULL,
      G_CALLBACK(LightDM_constructor),
      js_context, NULL,
      JSC_TYPE_VALUE, 0, NULL);

  const struct JSCClassProperty LightDM_properties[] = {
    {"authentication_user", G_CALLBACK(LightDM_authentication_user_getter_cb), NULL, JSC_TYPE_VALUE},
    {"autologin_guest", G_CALLBACK(LightDM_autologin_guest_getter_cb), NULL, G_TYPE_BOOLEAN},
    {"autologin_timeout", G_CALLBACK(LightDM_autologin_timeout_getter_cb), NULL, G_TYPE_INT},
    {"autologin_user", G_CALLBACK(LightDM_autologin_user_getter_cb), NULL, JSC_TYPE_VALUE},

    {"can_hibernate", G_CALLBACK(LightDM_can_hibernate_getter_cb), NULL, G_TYPE_BOOLEAN},
    {"can_restart", G_CALLBACK(LightDM_can_restart_getter_cb), NULL, G_TYPE_BOOLEAN},
    {"can_shutdown", G_CALLBACK(LightDM_can_shutdown_getter_cb), NULL, G_TYPE_BOOLEAN},
    {"can_suspend", G_CALLBACK(LightDM_can_suspend_getter_cb), NULL, G_TYPE_BOOLEAN},

    {"brightness", G_CALLBACK(LightDM_brightness_getter_cb), G_CALLBACK(LightDM_brightness_setter_cb), JSC_TYPE_VALUE},

    {NULL, NULL, NULL, 0},
  };
  const struct JSCClassMethod LightDM_methods[] = {
    {"authenticate", G_CALLBACK(LightDM_authenticate_cb), JSC_TYPE_VALUE},
    {"authenticate_as_guest", G_CALLBACK(LightDM_authenticate_as_guest_cb), JSC_TYPE_VALUE},
    {"cancel_authentication", G_CALLBACK(LightDM_cancel_authentication_cb), JSC_TYPE_VALUE},
    {"cancel_autologin", G_CALLBACK(LightDM_cancel_autologin_cb), JSC_TYPE_VALUE},
    {"hibernate", G_CALLBACK(LightDM_hibernate_cb), JSC_TYPE_VALUE},
    {"respond", G_CALLBACK(LightDM_respond_cb), JSC_TYPE_VALUE},
    {"restart", G_CALLBACK(LightDM_restart_cb), JSC_TYPE_VALUE},
    {"set_language", G_CALLBACK(LightDM_set_language_cb), JSC_TYPE_VALUE},
    {"shutdown", G_CALLBACK(LightDM_shutdown_cb), JSC_TYPE_VALUE},
    {"start_session", G_CALLBACK(LightDM_start_session_cb), JSC_TYPE_VALUE},
    {"suspend", G_CALLBACK(LightDM_suspend_cb), JSC_TYPE_VALUE},

    {NULL, NULL, 0},
  };
  /*const struct JSCClassSignal LightDM_signals[] = {*/
    /*{"authentication_complete"},*/
    /*{"autologin_timer_expired"},*/
    /*{"show_prompt"},*/
    /*{"show_message"},*/
    /*{NULL},*/
  /*};*/

  initialize_class_properties(LightDM_class, LightDM_properties);
  initialize_class_methods(LightDM_class, LightDM_methods);

  JSCValue *value = jsc_value_constructor_callv(ldm_constructor, 0, NULL);
  LightDM_object = malloc(sizeof *LightDM_object);
  LightDM_object->value = value;
  LightDM_object->context = js_context;

  JSCValue *ldm_greeter_object = jsc_value_new_object(js_context, LightDM_object, LightDM_class);

  /*initialize_object_signals(js_context, ldm_greeter_object, LightDM_signals);*/

  LightDM_object->value = ldm_greeter_object;

  jsc_value_object_set_property(
      global_object,
      "lightdm_test",
      ldm_greeter_object
      );

  JSCValue *event_class = jsc_value_object_get_property(global_object, "Event");
  JSCValue *event_parameters[] = {
    jsc_value_new_string(js_context, "GreeterReady"),
    NULL
  };
  ready_event = jsc_value_constructor_callv(event_class, 1, event_parameters);

  /*jsc_value_object_set_property(*/
      /*global_object,*/
      /*"_ready_event",*/
      /*ready_event*/
      /*);*/

  /*g_signal_connect(web_page, "document-loaded",*/
      /*G_CALLBACK(ready_event_loaded),*/
      /*js_context);*/
}
