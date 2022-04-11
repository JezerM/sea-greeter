#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <webkit2/webkit-web-extension.h>
#include <JavaScriptCore/JavaScript.h>
#include <lightdm-gobject-1/lightdm.h>

#include "settings.h"
#include "logger.h"
#include "lightdm-extension.h"
#include "bridge/lightdm-objects.h"
#include "bridge/utils.h"
#include "bridge/lightdm-signal.h"

LightDMGreeter *Greeter;
LightDMUserList *UserList;
extern WebKitWebExtension *WebExtension;
extern guint64 page_id;

JSCClass *LightDM_class;
ldm_object *LightDM_object;
static GString *shared_data_directory;

/* LightDM Class definitions */

/**
 * Starts a LightDM authentication process
 * Provide a string to prompt for the password
 * Provide an empty string or NULL to prompt for the user
 */
static JSCValue *
LightDM_authenticate_cb(
    ldm_object *instance,
    GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  gchar *user = NULL;
  if (arguments->len > 0) {
    JSCValue *v = arguments->pdata[0];
    user = js_value_to_string(v);
  }
  if (user && strcmp(user, "") == 0) user = NULL;

  GError *err = NULL;
  if (!lightdm_greeter_authenticate(Greeter, user, &err)) {
    logger_error(err->message);
    g_free(user);
    return jsc_value_new_boolean(context, false);
  }
  g_free(user);
  return jsc_value_new_boolean(context, true);
}
/**
 * Provides a response to a LightDM prompt
 * This could be either the user or the password
 * @param instance The lightdm object instance
 * @param arguments A pointer array to all JSCValue arguments
 */
static JSCValue *
LightDM_respond_cb(
    ldm_object *instance,
    GPtrArray *arguments)
{
  JSCContext *context = instance->context;

  gchar *response = NULL;
  if (arguments->len == 0) return jsc_value_new_boolean(context, false);
  JSCValue *v = arguments->pdata[0];
  response = js_value_to_string(v);

  GError *err = NULL;
  if (!lightdm_greeter_respond(Greeter, response, &err)) {
    logger_error(err->message);
    g_free(response);
    return jsc_value_new_boolean(context, false);
  }
  g_free(response);
  return jsc_value_new_boolean(context, true);
}
/**
 * Get whether the user is authenticated
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_is_authenticated_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean value = lightdm_greeter_get_is_authenticated(Greeter);
  return jsc_value_new_boolean(context, value);
}

/**
 * Get the available sessions in an array
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_sessions_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  GList *sessions = lightdm_get_sessions();
  GPtrArray *arr = g_ptr_array_new();
  GList *curr = sessions;
  while (curr != NULL) {
    g_ptr_array_add(arr, LightDMSession_to_JSCValue(context, curr->data));
    curr = curr->next;
  }
  return jsc_value_new_array_from_garray(context, arr);
}
/**
 * Get the available users in an array
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_users_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  GList *users = lightdm_user_list_get_users(UserList);
  GPtrArray *arr = g_ptr_array_new();
  GList *curr = users;
  while (curr != NULL) {
    g_ptr_array_add(arr, LightDMUser_to_JSCValue(context, curr->data));
    curr = curr->next;
  }
  return jsc_value_new_array_from_garray(context, arr);
}

struct JSCClassProperty LightDM_properties[] = {
  /*{"authentication_user", NULL, NULL, G_TYPE_STRING},*/
  /*{"autologin_guest", NULL, NULL, G_TYPE_BOOLEAN},*/
  /*{"autologin_timeout", NULL, NULL, G_TYPE_INT},*/
  /*{"autologin_user", NULL, NULL, G_TYPE_STRING},*/

  /*{"can_hibernate", NULL, NULL, G_TYPE_BOOLEAN},*/
  /*{"can_restart", NULL, NULL, G_TYPE_BOOLEAN},*/
  /*{"can_shutdown", NULL, NULL, G_TYPE_BOOLEAN},*/
  /*{"can_suspend", NULL, NULL, G_TYPE_BOOLEAN},*/

  /*{"default_session", NULL, NULL, G_TYPE_STRING},*/
  /*{"has_guest_account", NULL, NULL, G_TYPE_BOOLEAN},*/
  /*{"hide_users", NULL, NULL, G_TYPE_BOOLEAN},*/
  /*{"hostname", NULL, NULL, G_TYPE_STRING},*/

  /*{"in_authentication", NULL, NULL, G_TYPE_BOOLEAN},*/
  {"is_authenticated", G_CALLBACK(LightDM_is_authenticated_getter_cb), NULL, G_TYPE_BOOLEAN},

  /*{"language", NULL, NULL, LIGHTDM_INTERFACE_TYPE_LANGUAGE},*/
  /*{"languages", NULL, NULL, G_TYPE_VARIANT},*/
  /*{"layout", NULL, NULL, LIGHTDM_INTERFACE_TYPE_LAYOUT},*/
  /*{"layouts", NULL, NULL, G_TYPE_VARIANT},*/

  /*{"lock_hint", NULL, NULL, G_TYPE_BOOLEAN},*/
  {"sessions", G_CALLBACK(LightDM_sessions_getter_cb), NULL, G_TYPE_ARRAY_POST},
  {"users", G_CALLBACK(LightDM_users_getter_cb), NULL, G_TYPE_ARRAY_POST},

  {NULL, NULL, NULL, 0},
};
struct JSCClassMethod LightDM_methods[] = {
  {"authenticate", G_CALLBACK(LightDM_authenticate_cb), G_TYPE_BOOLEAN},
  {"respond", G_CALLBACK(LightDM_respond_cb), G_TYPE_BOOLEAN},

  {NULL, NULL, 0},
};
struct JSCClassSignal LightDM_signals[] = {
  {"authentication_complete"},
  {"autologin_timer_expired"},
  {"show_prompt"},
  {"show_message"},
  {NULL},
};

/* LightDM callbacks */

static void
authentication_complete_cb(
    LightDMGreeter *greeter,
    WebKitWebExtension *extension)
{
  (void) greeter;
  (void) extension;
  JSCValue *value = LightDM_object->value;

  JSCValue *signal = jsc_value_object_get_property(value, "authentication_complete");
  (void) jsc_value_object_invoke_method(
      signal,
      "emit",
      G_TYPE_NONE
      );
}
static void autologin_timer_expired_cb(
    LightDMGreeter *greeter,
    WebKitWebExtension *extension)
{
  (void) greeter;
  (void) extension;
  JSCValue *value = LightDM_object->value;

  JSCValue *signal = jsc_value_object_get_property(value, "autologin_timer_expired");
  (void) jsc_value_object_invoke_method(
      signal,
      "emit",
      G_TYPE_NONE
      );
}
static void show_prompt_cb(
    LightDMGreeter *greeter,
    const gchar *text,
    LightDMPromptType type,
    WebKitWebExtension *extension)
{
  (void) greeter;
  (void) extension;
  JSCValue *value = LightDM_object->value;

  JSCValue *signal = jsc_value_object_get_property(value, "show_prompt");
  (void) jsc_value_object_invoke_method(
      signal,
      "emit",
      G_TYPE_STRING,
      text,
      G_TYPE_INT,
      type,
      G_TYPE_NONE
      );
}
static void show_message_cb(
    LightDMGreeter *greeter,
    const gchar *text,
    LightDMMessageType type,
    WebKitWebExtension *extension)
{
  (void) greeter;
  (void) extension;
  JSCValue *value = LightDM_object->value;

  JSCValue *signal = jsc_value_object_get_property(value, "show_message");
  (void) jsc_value_object_invoke_method(
      signal,
      "emit",
      G_TYPE_STRING,
      text,
      G_TYPE_INT,
      type,
      G_TYPE_NONE
      );
}

/**
 * Connect LightDM signals to their callbacks
 */
void LightDM_connect_signals() {
  g_signal_connect(
    Greeter,
    "authentication-complete",
    G_CALLBACK(authentication_complete_cb),
    WebExtension
  );
  g_signal_connect(
    Greeter,
    "autologin-timer-expired",
    G_CALLBACK(autologin_timer_expired_cb),
    WebExtension
  );
  g_signal_connect(
    Greeter,
    "show-prompt",
    G_CALLBACK(show_prompt_cb),
    WebExtension
  );
  g_signal_connect(
    Greeter,
    "show-message",
    G_CALLBACK(show_message_cb),
    WebExtension
  );
}

/**
 * LightDM Class constructor, should be called only once in sea-greeter's life
 */
static void
LightDM_constructor() {
  GError *err = NULL;

  LightDM_connect_signals();

  gboolean connected = lightdm_greeter_connect_to_daemon_sync(Greeter, &err);
  if (!connected && err) {
    logger_error(err->message);
    g_error_free(err);
  }

  LightDMUser *user = lightdm_user_list_get_users(UserList)->data;
  gchar *user_data_dir = lightdm_greeter_ensure_shared_data_dir_sync(Greeter, lightdm_user_get_name(user), &err);

  int ind = g_string_get_last_index_of(
      g_string_new(user_data_dir),
      g_string_new("/")
      );

  shared_data_directory = g_string_new(
      g_utf8_substring(user_data_dir, 0, ind)
      );

  logger_debug("LightDM API connected");
}

/**
 * Initialize the LightDM environment
 */
void
LightDM_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension
) {
  (void) web_page;
  (void) extension;

  Greeter = lightdm_greeter_new();
  UserList = lightdm_user_list_get_instance();

  JSCContext *js_context = webkit_frame_get_js_context_for_script_world(web_frame, world);
  JSCValue *global_object = jsc_context_get_global_object(js_context);

  LightDM_class = jsc_context_register_class(
      js_context,
      "__LightDMGreeter",
      NULL,
      NULL,
      NULL
      );
  JSCValue *ldm_constructor = jsc_class_add_constructor(
      LightDM_class, NULL,
      G_CALLBACK(LightDM_constructor),
      NULL, NULL,
      JSC_TYPE_VALUE, 0);

  initialize_class_properties(LightDM_class, LightDM_properties);
  initialize_class_methods(LightDM_class, LightDM_methods);

  /*JSCValue *authentication_complete_signal = LightDM_signal_new(*/
      /*js_context,*/
      /*"authentication_complete"*/
      /*);*/
  /*JSCValue *show_prompt_signal = LightDM_signal_new(*/
      /*js_context,*/
      /*"show_prompt"*/
      /*);*/

  JSCValue *value = jsc_value_constructor_callv(ldm_constructor, 0, NULL);
  LightDM_object = malloc(sizeof *LightDM_object);
  LightDM_object->value = value;
  LightDM_object->context = js_context;
  JSCValue *ldm_greeter_object = jsc_value_new_object(js_context, LightDM_object, LightDM_class);
  initialize_object_signals(js_context, ldm_greeter_object, LightDM_signals);

  LightDM_object->value = ldm_greeter_object;

  /*jsc_value_object_set_property(*/
      /*ldm_greeter_object,*/
      /*"authentication_complete",*/
      /*authentication_complete_signal*/
      /*);*/
  /*jsc_value_object_set_property(*/
      /*ldm_greeter_object,*/
      /*"show_prompt",*/
      /*show_prompt_signal*/
      /*);*/

  jsc_value_object_set_property(
      global_object,
      "lightdmg",
      ldm_greeter_object
      );
}
