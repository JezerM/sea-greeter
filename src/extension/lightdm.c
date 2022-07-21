#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit-web-extension.h>

#include "logger.h"
#include "settings.h"

#include "extension/lightdm-signal.h"
#include "utils/ipc-renderer.h"
#include "utils/utils.h"

/*extern WebKitWebExtension *WebExtension;*/
/*extern guint64 page_id;*/
static WebKitWebPage *WebPage;

static JSCClass *LightDM_class;
static ldm_object *LightDM_object;
static JSCValue *ready_event;

/*static GString *shared_data_directory;*/

/* LightDM Class definitions */

static JSCValue *
LightDM_authenticate_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "authenticate", arguments);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_authenticate_as_guest_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "authenticate_as_guest", arguments);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_cancel_authentication_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "cancel_authentication", arguments);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_cancel_autologin_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "cancel_autologin", arguments);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_hibernate_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "hibernate", arguments);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_respond_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "respond", arguments);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_restart_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "restart", arguments);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_set_language_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "set_language", arguments);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_shutdown_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "shutdown", arguments);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_start_session_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "start_session", arguments);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_suspend_cb(ldm_object *instance, GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "suspend", arguments);
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
  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "authentication_user", NULL);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_autologin_guest_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "autologin_guest", NULL);
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
  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "autologin_timeout", NULL);
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
  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "autologin_user", NULL);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_can_hibernate_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "can_hibernate", NULL);
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
  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "can_restart", NULL);
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
  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "can_shutdown", NULL);
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
  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "can_suspend", NULL);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}

static JSCValue *
LightDM_brightness_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "brightness", NULL);
  if (reply == NULL) {
    return jsc_value_new_number(context, -1);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_brightness_setter_cb(ldm_object *instance, JSCValue *object)
{
  JSCContext *context = instance->context;
  GPtrArray *arguments = g_ptr_array_new();
  g_ptr_array_add(arguments, object);

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "brightness", arguments);
  if (reply == NULL) {
    return jsc_value_new_number(context, -1);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  g_ptr_array_free(arguments, true);
  return value;
}
static JSCValue *
LightDM_default_session_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "default_session", NULL);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_has_guest_account_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "has_guest_account", NULL);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_hide_users_hint_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "hide_users_hint", NULL);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_hostname_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "hostname", NULL);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_in_authentication_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "in_authentication", NULL);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_is_authenticated_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "is_authenticated", NULL);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_language_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "language", NULL);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_languages_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "languages", NULL);
  if (reply == NULL) {
    return jsc_value_new_array(context, G_TYPE_NONE);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_layout_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "layout", NULL);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_layout_setter_cb(ldm_object *instance, JSCValue *object)
{
  JSCContext *context = instance->context;
  GPtrArray *arguments = g_ptr_array_new();
  g_ptr_array_add(arguments, object);

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "layout", arguments);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  g_ptr_array_free(arguments, true);
  return value;
}
static JSCValue *
LightDM_layouts_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "layouts", NULL);
  if (reply == NULL) {
    return jsc_value_new_array(context, G_TYPE_NONE);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_lock_hint_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "lock_hint", NULL);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_remote_sessions_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "remote_sessions", NULL);
  if (reply == NULL) {
    return jsc_value_new_array(context, G_TYPE_NONE);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_select_guest_hint_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "select_guest_hint", NULL);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_select_user_hint_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "select_user_hint", NULL);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_sessions_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "sessions", NULL);
  if (reply == NULL) {
    return jsc_value_new_array(context, G_TYPE_NONE);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_shared_data_directory_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "shared_data_directory", NULL);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_show_manual_login_hint_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "show_manual_login_hint", NULL);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_show_remote_login_hint_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "show_remote_login_hint", NULL);
  if (reply == NULL) {
    return jsc_value_new_boolean(context, false);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
LightDM_users_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;

  WebKitUserMessage *reply = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "lightdm", "users", NULL);
  if (reply == NULL) {
    return jsc_value_new_array(context, G_TYPE_NONE);
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}

static void
handle_lightdm_signal(WebKitWebPage *web_page, WebKitUserMessage *message)
{
  (void) web_page;
  const char *name = webkit_user_message_get_name(message);
  if (g_strcmp0(name, "lightdm") != 0)
    return;

  GVariant *msg_param = webkit_user_message_get_parameters(message);
  if (!g_variant_is_of_type(msg_param, G_VARIANT_TYPE_ARRAY)) {
    return;
  }
  int parameters_length = g_variant_n_children(msg_param);
  if (parameters_length == 0 || parameters_length > 2) {
    return;
  }

  JSCContext *context = LightDM_object->context;
  JSCValue *parameters = NULL;

  GVariant *method_var = g_variant_get_child_value(msg_param, 0);
  GVariant *params_var = g_variant_get_child_value(msg_param, 1);

  const gchar *signal = g_variant_to_string(method_var);
  const gchar *json_params = g_variant_to_string(params_var);
  parameters = jsc_value_new_from_json(context, json_params);

  if (signal == NULL) {
    return;
  }

  JSCValue *jsc_signal = jsc_value_object_get_property(LightDM_object->value, signal);
  if (jsc_signal == NULL) {
    return;
  }
  GPtrArray *g_array = jsc_array_to_g_ptr_array(parameters);
  (void) jsc_value_object_invoke_methodv(jsc_signal, "emit", g_array->len, (JSCValue **) g_array->pdata);

  g_ptr_array_free(g_array, true);
}

static void
web_page_user_message_received(WebKitWebPage *web_page, WebKitUserMessage *message, gpointer user_data)
{
  (void) user_data;
  /*const char *name = webkit_user_message_get_name(message);*/
  /*printf("Got web_view message: '%s'\n", name);*/

  handle_lightdm_signal(web_page, message);
}

static JSCValue *
LightDM_constructor(JSCContext *context)
{
  return jsc_value_new_null(context);
}

static void
ready_event_loaded(WebKitWebPage *web_page, JSCContext *context)
{
  (void) web_page;
  JSCValue *global_object = jsc_context_get_global_object(context);

  JSCValue *dispatch_event = jsc_value_object_get_property(global_object, "dispatchEvent");
  JSCValue *parameters[] = { ready_event, NULL };
  (void) jsc_value_function_callv(dispatch_event, 1, parameters);
}

/**
 * Initialize the LightDM environment
 */
void
LightDM_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension)
{
  (void) extension;

  WebPage = web_page;

  JSCContext *js_context = webkit_frame_get_js_context_for_script_world(web_frame, world);
  JSCValue *global_object = jsc_context_get_global_object(js_context);

  if (LightDM_object != NULL) {
    jsc_value_object_set_property(global_object, "lightdm", LightDM_object->value);

    jsc_value_object_set_property(global_object, "_ready_event", ready_event);
    return;
  }

  g_signal_connect(web_page, "user-message-received", G_CALLBACK(web_page_user_message_received), NULL);

  LightDM_class = jsc_context_register_class(js_context, "__LightDMGreeter", NULL, NULL, NULL);
  JSCValue *ldm_constructor = jsc_class_add_constructor(
      LightDM_class,
      NULL,
      G_CALLBACK(LightDM_constructor),
      js_context,
      NULL,
      JSC_TYPE_VALUE,
      0,
      NULL);

  const struct JSCClassProperty LightDM_properties[] = {
    { "authentication_user", G_CALLBACK(LightDM_authentication_user_getter_cb), NULL, JSC_TYPE_VALUE },
    { "autologin_guest", G_CALLBACK(LightDM_autologin_guest_getter_cb), NULL, JSC_TYPE_VALUE },
    { "autologin_timeout", G_CALLBACK(LightDM_autologin_timeout_getter_cb), NULL, JSC_TYPE_VALUE },
    { "autologin_user", G_CALLBACK(LightDM_autologin_user_getter_cb), NULL, JSC_TYPE_VALUE },

    { "can_hibernate", G_CALLBACK(LightDM_can_hibernate_getter_cb), NULL, JSC_TYPE_VALUE },
    { "can_restart", G_CALLBACK(LightDM_can_restart_getter_cb), NULL, JSC_TYPE_VALUE },
    { "can_shutdown", G_CALLBACK(LightDM_can_shutdown_getter_cb), NULL, JSC_TYPE_VALUE },
    { "can_suspend", G_CALLBACK(LightDM_can_suspend_getter_cb), NULL, JSC_TYPE_VALUE },

    { "brightness",
      G_CALLBACK(LightDM_brightness_getter_cb),
      G_CALLBACK(LightDM_brightness_setter_cb),
      JSC_TYPE_VALUE },

    { "default_session", G_CALLBACK(LightDM_default_session_getter_cb), NULL, JSC_TYPE_VALUE },
    { "has_guest_account", G_CALLBACK(LightDM_has_guest_account_getter_cb), NULL, JSC_TYPE_VALUE },
    { "hide_users_hint", G_CALLBACK(LightDM_hide_users_hint_getter_cb), NULL, JSC_TYPE_VALUE },
    { "hostname", G_CALLBACK(LightDM_hostname_getter_cb), NULL, JSC_TYPE_VALUE },

    { "in_authentication", G_CALLBACK(LightDM_in_authentication_getter_cb), NULL, JSC_TYPE_VALUE },
    { "is_authenticated", G_CALLBACK(LightDM_is_authenticated_getter_cb), NULL, JSC_TYPE_VALUE },

    { "language", G_CALLBACK(LightDM_language_getter_cb), NULL, JSC_TYPE_VALUE },
    { "languages", G_CALLBACK(LightDM_languages_getter_cb), NULL, JSC_TYPE_VALUE },
    { "layout", G_CALLBACK(LightDM_layout_getter_cb), G_CALLBACK(LightDM_layout_setter_cb), JSC_TYPE_VALUE },
    { "layouts", G_CALLBACK(LightDM_layouts_getter_cb), NULL, JSC_TYPE_VALUE },

    { "lock_hint", G_CALLBACK(LightDM_lock_hint_getter_cb), NULL, JSC_TYPE_VALUE },
    { "remote_sessions", G_CALLBACK(LightDM_remote_sessions_getter_cb), NULL, JSC_TYPE_VALUE },
    { "select_guest_hint", G_CALLBACK(LightDM_select_guest_hint_getter_cb), NULL, JSC_TYPE_VALUE },
    { "select_user_hint", G_CALLBACK(LightDM_select_user_hint_getter_cb), NULL, JSC_TYPE_VALUE },
    { "sessions", G_CALLBACK(LightDM_sessions_getter_cb), NULL, JSC_TYPE_VALUE },
    { "shared_data_directory", G_CALLBACK(LightDM_shared_data_directory_getter_cb), NULL, JSC_TYPE_VALUE },
    { "show_manual_login_hint", G_CALLBACK(LightDM_show_manual_login_hint_getter_cb), NULL, JSC_TYPE_VALUE },
    { "show_remote_login_hint", G_CALLBACK(LightDM_show_remote_login_hint_getter_cb), NULL, JSC_TYPE_VALUE },
    { "users", G_CALLBACK(LightDM_users_getter_cb), NULL, JSC_TYPE_VALUE },

    { NULL, NULL, NULL, 0 },
  };
  const struct JSCClassMethod LightDM_methods[] = {
    { "authenticate", G_CALLBACK(LightDM_authenticate_cb), JSC_TYPE_VALUE },
    { "authenticate_as_guest", G_CALLBACK(LightDM_authenticate_as_guest_cb), JSC_TYPE_VALUE },
    { "cancel_authentication", G_CALLBACK(LightDM_cancel_authentication_cb), JSC_TYPE_VALUE },
    { "cancel_autologin", G_CALLBACK(LightDM_cancel_autologin_cb), JSC_TYPE_VALUE },
    { "hibernate", G_CALLBACK(LightDM_hibernate_cb), JSC_TYPE_VALUE },
    { "respond", G_CALLBACK(LightDM_respond_cb), JSC_TYPE_VALUE },
    { "restart", G_CALLBACK(LightDM_restart_cb), JSC_TYPE_VALUE },
    { "set_language", G_CALLBACK(LightDM_set_language_cb), JSC_TYPE_VALUE },
    { "shutdown", G_CALLBACK(LightDM_shutdown_cb), JSC_TYPE_VALUE },
    { "start_session", G_CALLBACK(LightDM_start_session_cb), JSC_TYPE_VALUE },
    { "suspend", G_CALLBACK(LightDM_suspend_cb), JSC_TYPE_VALUE },

    { NULL, NULL, 0 },
  };
  const struct JSCClassSignal LightDM_signals[] = {
    { "authentication_complete" }, { "autologin_timer_expired" }, { "show_prompt" }, { "show_message" }, { NULL },
  };

  initialize_class_properties(LightDM_class, LightDM_properties);
  initialize_class_methods(LightDM_class, LightDM_methods);

  JSCValue *value = jsc_value_constructor_callv(ldm_constructor, 0, NULL);
  LightDM_object = malloc(sizeof *LightDM_object);
  LightDM_object->value = value;
  LightDM_object->context = js_context;

  JSCValue *ldm_greeter_object = jsc_value_new_object(js_context, LightDM_object, LightDM_class);

  initialize_object_signals(js_context, ldm_greeter_object, LightDM_signals);

  LightDM_object->value = ldm_greeter_object;

  jsc_value_object_set_property(global_object, "lightdm", ldm_greeter_object);

  JSCValue *event_class = jsc_value_object_get_property(global_object, "Event");
  JSCValue *event_parameters[] = { jsc_value_new_string(js_context, "GreeterReady"), NULL };
  ready_event = jsc_value_constructor_callv(event_class, 1, event_parameters);

  jsc_value_object_set_property(global_object, "_ready_event", ready_event);

  g_signal_connect(web_page, "document-loaded", G_CALLBACK(ready_event_loaded), js_context);
}
