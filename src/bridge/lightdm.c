#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <webkit2/webkit-web-extension.h>
#include <JavaScriptCore/JavaScript.h>
#include <lightdm-gobject-1/lightdm.h>

#include "lightdm/greeter.h"
#include "lightdm/power.h"
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
 * Starts the authentication procedure for a user
 * Provide a string to prompt for the password
 * Provide an empty string or NULL to prompt for the user
 */
static JSCValue *
LightDM_authenticate_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  gchar *user = NULL;
  if (arguments->len > 0) {
    JSCValue *v = arguments->pdata[0];
    user = js_value_to_string_or_null(v);
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
 * Starts the authentication procedure for the guest user
 */
static JSCValue *
LightDM_authenticate_as_guest_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  (void) arguments;
  JSCContext *context = instance->context;
  GError *err = NULL;
  if (!lightdm_greeter_authenticate_as_guest(Greeter, &err)) {
    logger_error(err->message);
    return jsc_value_new_boolean(context, false);
  }
  return jsc_value_new_boolean(context, true);
}
/**
 * Cancel user authentication that is currently in progress
 */
static JSCValue *
LightDM_cancel_authentication_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  (void) arguments;
  JSCContext *context = instance->context;
  GError *err = NULL;
  if (!lightdm_greeter_cancel_authentication(Greeter, &err)) {
    logger_error(err->message);
    return jsc_value_new_boolean(context, false);
  }
  return jsc_value_new_boolean(context, true);
}
/**
 * Cancel the automatic login
 */
static JSCValue *
LightDM_cancel_autologin_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  (void) arguments;
  JSCContext *context = instance->context;
  lightdm_greeter_cancel_autologin(Greeter);
  return jsc_value_new_boolean(context, true);
}
/**
 * Triggers the system to hibernate
 */
static JSCValue *
LightDM_hibernate_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  (void) arguments;
  JSCContext *context = instance->context;
  GError *err = NULL;
  if (!lightdm_hibernate(&err)) {
    logger_error(err->message);
    return jsc_value_new_boolean(context, false);
  }
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
  response = js_value_to_string_or_null(v);

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
 * Triggers the system to restart
 */
static JSCValue *
LightDM_restart_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  (void) arguments;
  JSCContext *context = instance->context;
  GError *err = NULL;
  if (!lightdm_restart(&err)) {
    logger_error(err->message);
    return jsc_value_new_boolean(context, false);
  }
  return jsc_value_new_boolean(context, true);
}
/**
 * Set the language for the currently authenticated user
 */
static JSCValue *
LightDM_set_language_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  gchar *language = NULL;
  if (arguments->len == 0) return jsc_value_new_boolean(context, false);
  JSCValue *v = arguments->pdata[0];
  language = js_value_to_string_or_null(v);

  GError *err = NULL;
  if (!lightdm_greeter_set_language(Greeter, language, &err)) {
    logger_error(err->message);
    g_free(language);
    return jsc_value_new_boolean(context, false);
  }
  g_free(language);
  return jsc_value_new_boolean(context, true);
}
/**
 * Triggers the system to shutdown
 */
static JSCValue *
LightDM_shutdown_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  (void) arguments;
  JSCContext *context = instance->context;
  GError *err = NULL;
  if (!lightdm_shutdown(&err)) {
    logger_error(err->message);
    return jsc_value_new_boolean(context, false);
  }
  return jsc_value_new_boolean(context, true);
}
/**
 * Start a session for the authenticated user
 */
static JSCValue *
LightDM_start_session_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  JSCContext *context = instance->context;

  gchar *session = NULL;
  if (arguments->len == 0) return jsc_value_new_boolean(context, false);
  JSCValue *v = arguments->pdata[0];
  session = js_value_to_string_or_null(v);

  GError *err = NULL;
  if (!lightdm_greeter_start_session_sync(Greeter, session, &err)) {
    logger_error(err->message);
    g_free(session);
    return jsc_value_new_boolean(context, false);
  }
  // reset_screensaver();
  g_free(session);
  return jsc_value_new_boolean(context, true);
}
/**
 * Triggers the system to suspend/sleep
 */
static JSCValue *
LightDM_suspend_cb(
    ldm_object *instance,
    GPtrArray *arguments
) {
  (void) arguments;
  JSCContext *context = instance->context;
  GError *err = NULL;
  if (!lightdm_suspend(&err)) {
    logger_error(err->message);
    return jsc_value_new_boolean(context, false);
  }
  return jsc_value_new_boolean(context, true);
}

/* LightDM properties */

/**
 * Get the username of the user being authenticated
 * or NULL if no authentication is in progress
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_authentication_user_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  const gchar *user = lightdm_greeter_get_authentication_user(Greeter);
  if (user == NULL)
    return jsc_value_new_null(context);
  return jsc_value_new_string(context, user);
}
/**
 * Get whether or not the guest account should be automatically logged
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_autologin_guest_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean value = lightdm_greeter_get_autologin_guest_hint(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Get the number of seconds to wait before automatically loggin in
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_autologin_timeout_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gint value = lightdm_greeter_get_autologin_timeout_hint(Greeter);
  return jsc_value_new_number(context, value);
}
/**
 * Get the username with which to automatically log in when the timer expires
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_autologin_user_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  const gchar *value = lightdm_greeter_get_autologin_user_hint(Greeter);
  if (value == NULL)
    return jsc_value_new_null(context);
  return jsc_value_new_string(context, value);
}
/**
 * Get whether or not the greeter can make the system hibernate
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_can_hibernate_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean value = lightdm_get_can_hibernate();
  return jsc_value_new_boolean(context, value);
}
/**
 * Get whether or not the greeter can make the system restart
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_can_restart_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean value = lightdm_get_can_restart();
  return jsc_value_new_boolean(context, value);
}
/**
 * Get whether or not the greeter can make the system shutdown
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_can_shutdown_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean value = lightdm_get_can_shutdown();
  return jsc_value_new_boolean(context, value);
}
/**
 * Get whether or not the greeter can make the system suspend/sleep
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_can_suspend_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean value = lightdm_get_can_suspend();
  return jsc_value_new_boolean(context, value);
}
/**
 * Get the name of the default session
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_default_session_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  const gchar *session = lightdm_greeter_get_default_session_hint(Greeter);
  if (session == NULL)
    return jsc_value_new_null(context);
  return jsc_value_new_string(context, session);
}
/**
 * Get whether or not guest accounts are supported
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_has_guest_account_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean has_guest_account = lightdm_greeter_get_has_guest_account_hint(Greeter);
  return jsc_value_new_boolean(context, has_guest_account);
}
/**
 * Get whether or not user accounts should be hidden
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_hide_users_hint_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean hide_users = lightdm_greeter_get_hide_users_hint(Greeter);
  return jsc_value_new_boolean(context, hide_users);
}
/**
 * Get the system's hostname
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_hostname_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  const gchar *hostname = lightdm_get_hostname();
  if (hostname == NULL)
    return jsc_value_new_null(context);
  return jsc_value_new_string(context, hostname);
}
/**
 * Get whether or not the greeter is in the process of authenticating
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_in_authentication_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean value = lightdm_greeter_get_in_authentication(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Get whether or not the greeter has successfully authenticated
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
 * Get the current language or NULL if no language
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_language_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  LightDMLanguage *language = lightdm_get_language();

  JSCValue *object = LightDMLanguage_to_JSCValue(context, language);
  return object;
}
/**
 * Get a list of languages to present to the user
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_languages_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  GList *languages = lightdm_get_languages();
  GPtrArray *arr = g_ptr_array_new();
  GList *curr = languages;
  while (curr != NULL) {
    g_ptr_array_add(arr, LightDMLanguage_to_JSCValue(context, curr->data));
    curr = curr->next;
  }
  return jsc_value_new_array_from_garray(context, arr);
}
/**
 * Get the currently active layout for the selected user
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_layout_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  LightDMLayout *layout = lightdm_get_layout();

  JSCValue *object = LightDMLayout_to_JSCValue(context, layout);
  return object;
}
/**
 * Set the currently active layout for the selected user
 * @param instance The lightdm object instance
 */
static void *
LightDM_layout_setter_cb(ldm_object *instance, JSCValue *object)
{
  lightdm_get_layout();
  JSCContext *context = instance->context;
  LightDMLayout *layout = JSCValue_to_LightDMLayout(context, object);
  lightdm_set_layout(layout);
  return NULL;
}
/**
 * Get a list of keyboard layouts to present to the user
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_layouts_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  GList *layouts = lightdm_get_layouts();
  GPtrArray *arr = g_ptr_array_new();
  GList *curr = layouts;
  while (curr != NULL) {
    g_ptr_array_add(arr, LightDMLayout_to_JSCValue(context, curr->data));
    curr = curr->next;
  }
  return jsc_value_new_array_from_garray(context, arr);
}
/**
 * Get whether or not the greeter was started as a lock screen
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_lock_hint_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean value = lightdm_greeter_get_lock_hint(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Get a list of remote sessions
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_remote_sessions_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  GList *sessions = lightdm_get_remote_sessions();
  GPtrArray *arr = g_ptr_array_new();
  GList *curr = sessions;
  while (curr != NULL) {
    g_ptr_array_add(arr, LightDMSession_to_JSCValue(context, curr->data));
    curr = curr->next;
  }
  return jsc_value_new_array_from_garray(context, arr);
}
/**
 * Get whether or not the guest account should be selected by default
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_select_guest_hint_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean value = lightdm_greeter_get_select_guest_hint(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Get the username to select by default
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_select_user_hint_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  const gchar *value = lightdm_greeter_get_select_user_hint(Greeter);
  if (value == NULL)
    return jsc_value_new_null(context);
  return jsc_value_new_string(context, value);
}
/**
 * Get a list of available sessions
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
 * Get the LightDM shared data directory
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_shared_data_directory_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  if (shared_data_directory->len == 0 || shared_data_directory->str == NULL)
    return jsc_value_new_null(context);
  return jsc_value_new_string(context, shared_data_directory->str);
}
/**
 * Check if a manual login option should be shown
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_show_manual_login_hint_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean value = lightdm_greeter_get_show_manual_login_hint(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Check if a remote login option should be shown
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_show_remote_login_hint_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean value = lightdm_greeter_get_show_remote_login_hint(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Get a list of available users
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

/* LightDM callbacks */

static void
authentication_complete_cb(
    LightDMGreeter *greeter,
    WebKitWebExtension *extension
) {
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
static void
autologin_timer_expired_cb(
    LightDMGreeter *greeter,
    WebKitWebExtension *extension
) {
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
static JSCValue*
LightDM_constructor(JSCContext *context) {
  GError *err = NULL;

  LightDM_connect_signals();

  gboolean connected = lightdm_greeter_connect_to_daemon_sync(Greeter, &err);
  if (!connected && err) {
    logger_error(err->message);
    g_error_free(err);
  }

  LightDMUser *user = lightdm_user_list_get_users(UserList)->data;
  if (user != NULL) {
    gchar *user_data_dir = lightdm_greeter_ensure_shared_data_dir_sync(Greeter, lightdm_user_get_name(user), &err);

    int ind = g_string_get_last_index_of(
        g_string_new(user_data_dir),
        g_string_new("/")
        );

    shared_data_directory = g_string_new(
        g_utf8_substring(user_data_dir, 0, ind)
        );
  } else {
    shared_data_directory = g_string_new("");
  }

  logger_debug("LightDM API connected");

  return jsc_value_new_null(context);
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

    {"default_session", G_CALLBACK(LightDM_default_session_getter_cb), NULL, JSC_TYPE_VALUE},
    {"has_guest_account", G_CALLBACK(LightDM_has_guest_account_getter_cb), NULL, G_TYPE_BOOLEAN},
    {"hide_users_hint", G_CALLBACK(LightDM_hide_users_hint_getter_cb), NULL, G_TYPE_BOOLEAN},
    {"hostname", G_CALLBACK(LightDM_hostname_getter_cb), NULL, JSC_TYPE_VALUE},

    {"in_authentication", G_CALLBACK(LightDM_in_authentication_getter_cb), NULL, G_TYPE_BOOLEAN},
    {"is_authenticated", G_CALLBACK(LightDM_is_authenticated_getter_cb), NULL, G_TYPE_BOOLEAN},

    {"language", G_CALLBACK(LightDM_language_getter_cb), NULL, JSC_TYPE_VALUE},
    {"languages", G_CALLBACK(LightDM_languages_getter_cb), NULL, JSC_TYPE_VALUE},
    {"layout", G_CALLBACK(LightDM_layout_getter_cb), G_CALLBACK(LightDM_layout_setter_cb), JSC_TYPE_VALUE},
    {"layouts", G_CALLBACK(LightDM_layouts_getter_cb), NULL, JSC_TYPE_VALUE},

    {"lock_hint", G_CALLBACK(LightDM_lock_hint_getter_cb), NULL, G_TYPE_BOOLEAN},
    {"remote_sessions", G_CALLBACK(LightDM_remote_sessions_getter_cb), NULL, JSC_TYPE_VALUE},
    {"select_guest_hint", G_CALLBACK(LightDM_select_guest_hint_getter_cb), NULL, G_TYPE_BOOLEAN},
    {"select_user_hint", G_CALLBACK(LightDM_select_user_hint_getter_cb), NULL, JSC_TYPE_VALUE},
    {"sessions", G_CALLBACK(LightDM_sessions_getter_cb), NULL, JSC_TYPE_VALUE},
    {"shared_data_directory", G_CALLBACK(LightDM_shared_data_directory_getter_cb), NULL, JSC_TYPE_VALUE},
    {"show_manual_login_hint", G_CALLBACK(LightDM_show_manual_login_hint_getter_cb), NULL, G_TYPE_BOOLEAN},
    {"show_remote_login_hint", G_CALLBACK(LightDM_show_remote_login_hint_getter_cb), NULL, G_TYPE_BOOLEAN},
    {"users", G_CALLBACK(LightDM_users_getter_cb), NULL, JSC_TYPE_VALUE},

    {NULL, NULL, NULL, 0},
  };
  const struct JSCClassMethod LightDM_methods[] = {
    {"authenticate", G_CALLBACK(LightDM_authenticate_cb), G_TYPE_BOOLEAN},
    {"authenticate_as_guest", G_CALLBACK(LightDM_authenticate_as_guest_cb), G_TYPE_BOOLEAN},
    {"cancel_authentication", G_CALLBACK(LightDM_cancel_authentication_cb), G_TYPE_BOOLEAN},
    {"cancel_autologin", G_CALLBACK(LightDM_cancel_autologin_cb), G_TYPE_BOOLEAN},
    {"hibernate", G_CALLBACK(LightDM_hibernate_cb), G_TYPE_BOOLEAN},
    {"respond", G_CALLBACK(LightDM_respond_cb), G_TYPE_BOOLEAN},
    {"restart", G_CALLBACK(LightDM_restart_cb), G_TYPE_BOOLEAN},
    {"set_language", G_CALLBACK(LightDM_set_language_cb), G_TYPE_BOOLEAN},
    {"shutdown", G_CALLBACK(LightDM_shutdown_cb), G_TYPE_BOOLEAN},
    {"start_session", G_CALLBACK(LightDM_start_session_cb), G_TYPE_BOOLEAN},
    {"suspend", G_CALLBACK(LightDM_suspend_cb), G_TYPE_BOOLEAN},

    {NULL, NULL, 0},
  };
  const struct JSCClassSignal LightDM_signals[] = {
    {"authentication_complete"},
    {"autologin_timer_expired"},
    {"show_prompt"},
    {"show_message"},
    {NULL},
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

  jsc_value_object_set_property(
      global_object,
      "lightdm",
      ldm_greeter_object
      );

  JSCValue *event_class = jsc_value_object_get_property(global_object, "Event");
  JSCValue *event_parameters[] = {
    jsc_value_new_string(js_context, "GreeterReady"),
    NULL
  };
  JSCValue *event_obj = jsc_value_constructor_callv(event_class, 1, event_parameters);

  jsc_value_object_set_property(
      global_object,
      "_ready_event",
      event_obj
      );

}
