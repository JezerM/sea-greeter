#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <jsc/jsc.h>
#include <lightdm-gobject-1/lightdm.h>
#include <webkit2/webkit2.h>

#include "bridge/bridge-object.h"
#include "bridge/lightdm-objects.h"
#include "bridge/utils.h"

#include "browser.h"
#include "lightdm/language.h"
#include "logger.h"
#include "settings.h"
#include "utils/utils.h"

static LightDMGreeter *Greeter;
static LightDMUserList *UserList;
extern guint64 page_id;

GString *shared_data_directory;

extern GPtrArray *greeter_browsers;

static BridgeObject *LightDM_object = NULL;

/* LightDM Class definitions */

/**
 * Starts the authentication procedure for a user
 * Provide a string to prompt for the password
 * Provide an empty string or NULL to prompt for the user
 */
static JSCValue *
LightDM_authenticate_cb(GPtrArray *arguments)
{
  JSCContext *context = get_global_context();

  gchar *user = NULL;
  if (arguments->len > 0) {
    JSCValue *v = arguments->pdata[0];
    user = js_value_to_string_or_null(v);
  }
  if (user && strcmp(user, "") == 0)
    user = NULL;

  GError *err = NULL;
  if (!lightdm_greeter_authenticate(Greeter, user, &err)) {
    logger_error("%s", err != NULL ? err->message : "Could not authenticate");
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
LightDM_authenticate_as_guest_cb(GPtrArray *arguments)
{
  (void) arguments;
  JSCContext *context = get_global_context();
  GError *err = NULL;
  if (!lightdm_greeter_authenticate_as_guest(Greeter, &err)) {
    logger_error("%s", err != NULL ? err->message : "Could not authenticate as guest");
    return jsc_value_new_boolean(context, false);
  }
  return jsc_value_new_boolean(context, true);
}
/**
 * Cancel user authentication that is currently in progress
 */
static JSCValue *
LightDM_cancel_authentication_cb(GPtrArray *arguments)
{
  (void) arguments;
  JSCContext *context = get_global_context();
  GError *err = NULL;
  if (!lightdm_greeter_cancel_authentication(Greeter, &err)) {
    logger_error("%s", err != NULL ? err->message : "Could not cancel authentication");
    return jsc_value_new_boolean(context, false);
  }
  return jsc_value_new_boolean(context, true);
}
/**
 * Cancel the automatic login
 */
static JSCValue *
LightDM_cancel_autologin_cb(GPtrArray *arguments)
{
  (void) arguments;
  JSCContext *context = get_global_context();
  lightdm_greeter_cancel_autologin(Greeter);
  return jsc_value_new_boolean(context, true);
}
/**
 * Triggers the system to hibernate
 */
static JSCValue *
LightDM_hibernate_cb(GPtrArray *arguments)
{
  (void) arguments;
  JSCContext *context = get_global_context();
  GError *err = NULL;
  if (!lightdm_hibernate(&err)) {
    logger_error("%s", err != NULL ? err->message : "Could not hibernate");
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
LightDM_respond_cb(GPtrArray *arguments)
{
  JSCContext *context = get_global_context();

  gchar *response = NULL;
  if (arguments->len == 0)
    return jsc_value_new_boolean(context, false);
  JSCValue *v = arguments->pdata[0];
  response = js_value_to_string_or_null(v);

  GError *err = NULL;
  if (!lightdm_greeter_respond(Greeter, response, &err)) {
    logger_error("%s", err != NULL ? err->message : "Could not provide a response");
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
LightDM_restart_cb(GPtrArray *arguments)
{
  (void) arguments;
  JSCContext *context = get_global_context();
  GError *err = NULL;
  if (!lightdm_restart(&err)) {
    logger_error("%s", err != NULL ? err->message : "Could not restart");
    return jsc_value_new_boolean(context, false);
  }
  return jsc_value_new_boolean(context, true);
}
/**
 * Set the language for the currently authenticated user
 */
static JSCValue *
LightDM_set_language_cb(GPtrArray *arguments)
{
  JSCContext *context = get_global_context();

  gchar *language = NULL;
  if (arguments->len == 0)
    return jsc_value_new_boolean(context, false);
  JSCValue *v = arguments->pdata[0];
  language = js_value_to_string_or_null(v);

  GError *err = NULL;
  if (!lightdm_greeter_set_language(Greeter, language, &err)) {
    logger_error("%s", err != NULL ? err->message : "Could not set language");
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
LightDM_shutdown_cb(GPtrArray *arguments)
{
  (void) arguments;
  JSCContext *context = get_global_context();
  GError *err = NULL;
  if (!lightdm_shutdown(&err)) {
    logger_error("%s", err != NULL ? err->message : "Could not shutdown");
    return jsc_value_new_boolean(context, false);
  }
  return jsc_value_new_boolean(context, true);
}
/**
 * Start a session for the authenticated user
 */
static JSCValue *
LightDM_start_session_cb(GPtrArray *arguments)
{
  JSCContext *context = get_global_context();

  gchar *session = NULL;
  if (arguments->len == 0)
    return jsc_value_new_boolean(context, false);
  JSCValue *v = arguments->pdata[0];
  session = js_value_to_string_or_null(v);

  GError *err = NULL;
  if (!lightdm_greeter_start_session_sync(Greeter, session, &err)) {
    logger_error("%s", err != NULL ? err->message : "Could not start session");
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
LightDM_suspend_cb(GPtrArray *arguments)
{
  (void) arguments;
  JSCContext *context = get_global_context();
  GError *err = NULL;
  if (!lightdm_suspend(&err)) {
    logger_error("%s", err != NULL ? err->message : "Could not suspend");
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
LightDM_authentication_user_getter_cb(void)
{
  JSCContext *context = get_global_context();
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
LightDM_autologin_guest_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean value = lightdm_greeter_get_autologin_guest_hint(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Get the number of seconds to wait before automatically loggin in
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_autologin_timeout_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gint value = lightdm_greeter_get_autologin_timeout_hint(Greeter);
  return jsc_value_new_number(context, value);
}
/**
 * Get the username with which to automatically log in when the timer expires
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_autologin_user_getter_cb(void)
{
  JSCContext *context = get_global_context();
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
LightDM_can_hibernate_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean value = lightdm_get_can_hibernate();
  return jsc_value_new_boolean(context, value);
}
/**
 * Get whether or not the greeter can make the system restart
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_can_restart_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean value = lightdm_get_can_restart();
  return jsc_value_new_boolean(context, value);
}
/**
 * Get whether or not the greeter can make the system shutdown
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_can_shutdown_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean value = lightdm_get_can_shutdown();
  return jsc_value_new_boolean(context, value);
}
/**
 * Get whether or not the greeter can make the system suspend/sleep
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_can_suspend_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean value = lightdm_get_can_suspend();
  return jsc_value_new_boolean(context, value);
}
static int brightness = 85;
static JSCValue *
LightDM_brightness_getter_cb(void)
{
  JSCContext *context = get_global_context();
  return jsc_value_new_number(context, brightness);
}
static void *
LightDM_brightness_setter_cb(JSCValue *object)
{
  brightness = jsc_value_to_int32(object);
  return NULL;
}
/**
 * Get the name of the default session
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_default_session_getter_cb(void)
{
  JSCContext *context = get_global_context();
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
LightDM_has_guest_account_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean has_guest_account = lightdm_greeter_get_has_guest_account_hint(Greeter);
  return jsc_value_new_boolean(context, has_guest_account);
}
/**
 * Get whether or not user accounts should be hidden
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_hide_users_hint_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean hide_users = lightdm_greeter_get_hide_users_hint(Greeter);
  return jsc_value_new_boolean(context, hide_users);
}
/**
 * Get the system's hostname
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_hostname_getter_cb(void)
{
  JSCContext *context = get_global_context();
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
LightDM_in_authentication_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean value = lightdm_greeter_get_in_authentication(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Get whether or not the greeter has successfully authenticated
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_is_authenticated_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean value = lightdm_greeter_get_is_authenticated(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Get the current language or NULL if no language
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_language_getter_cb(void)
{
  JSCContext *context = get_global_context();
  LightDMLanguage *language = lightdm_get_language();

  if (language == NULL) {
    GList *languages = lightdm_get_languages();
    language = languages->data;
  }
  if (language == NULL) {
    return jsc_value_new_string(context, "undefined");
  }

  JSCValue *object = LightDMLanguage_to_JSCValue(context, language);
  return object;
}
/**
 * Get a list of languages to present to the user
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_languages_getter_cb(void)
{
  JSCContext *context = get_global_context();
  GList *languages = lightdm_get_languages();
  GPtrArray *arr = g_ptr_array_new();
  GList *curr = languages;
  while (curr != NULL) {
    JSCValue *language = LightDMLanguage_to_JSCValue(context, curr->data);
    if (language != NULL)
      g_ptr_array_add(arr, language);
    curr = curr->next;
  }
  JSCValue *value = jsc_value_new_array_from_garray(context, arr);
  g_ptr_array_free(arr, true);
  return value;
}
/**
 * Get the currently active layout for the selected user
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_layout_getter_cb(void)
{
  JSCContext *context = get_global_context();
  LightDMLayout *layout = lightdm_get_layout();

  JSCValue *object = LightDMLayout_to_JSCValue(context, layout);
  return object;
}
/**
 * Set the currently active layout for the selected user
 * @param instance The lightdm object instance
 */
static void *
LightDM_layout_setter_cb(JSCValue *object)
{
  lightdm_get_layout();
  JSCContext *context = get_global_context();
  LightDMLayout *layout = JSCValue_to_LightDMLayout(context, object);
  lightdm_set_layout(layout);
  return NULL;
}
/**
 * Get a list of keyboard layouts to present to the user
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_layouts_getter_cb(void)
{
  JSCContext *context = get_global_context();
  GList *layouts = lightdm_get_layouts();
  GPtrArray *arr = g_ptr_array_new();
  GList *curr = layouts;
  while (curr != NULL) {
    JSCValue *layout = LightDMLayout_to_JSCValue(context, curr->data);
    if (layout != NULL)
      g_ptr_array_add(arr, layout);
    curr = curr->next;
  }
  JSCValue *value = jsc_value_new_array_from_garray(context, arr);
  g_ptr_array_free(arr, true);
  return value;
}
/**
 * Get whether or not the greeter was started as a lock screen
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_lock_hint_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean value = lightdm_greeter_get_lock_hint(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Get a list of remote sessions
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_remote_sessions_getter_cb(void)
{
  JSCContext *context = get_global_context();
  GList *sessions = lightdm_get_remote_sessions();
  GPtrArray *arr = g_ptr_array_new();
  GList *curr = sessions;
  while (curr != NULL) {
    JSCValue *session = LightDMSession_to_JSCValue(context, curr->data);
    if (session != NULL)
      g_ptr_array_add(arr, session);
    curr = curr->next;
  }
  JSCValue *value = jsc_value_new_array_from_garray(context, arr);
  g_ptr_array_free(arr, true);
  return value;
}
/**
 * Get whether or not the guest account should be selected by default
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_select_guest_hint_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean value = lightdm_greeter_get_select_guest_hint(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Get the username to select by default
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_select_user_hint_getter_cb(void)
{
  JSCContext *context = get_global_context();
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
LightDM_sessions_getter_cb(void)
{
  JSCContext *context = get_global_context();
  GList *sessions = lightdm_get_sessions();
  GPtrArray *arr = g_ptr_array_new();
  GList *curr = sessions;
  while (curr != NULL) {
    JSCValue *session = LightDMSession_to_JSCValue(context, curr->data);
    if (session != NULL)
      g_ptr_array_add(arr, session);
    curr = curr->next;
  }
  JSCValue *value = jsc_value_new_array_from_garray(context, arr);
  g_ptr_array_free(arr, true);
  return value;
}
/**
 * Get the LightDM shared data directory
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_shared_data_directory_getter_cb(void)
{
  JSCContext *context = get_global_context();
  if (shared_data_directory->len == 0 || shared_data_directory->str == NULL)
    return jsc_value_new_null(context);
  return jsc_value_new_string(context, shared_data_directory->str);
}
/**
 * Check if a manual login option should be shown
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_show_manual_login_hint_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean value = lightdm_greeter_get_show_manual_login_hint(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Check if a remote login option should be shown
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_show_remote_login_hint_getter_cb(void)
{
  JSCContext *context = get_global_context();
  gboolean value = lightdm_greeter_get_show_remote_login_hint(Greeter);
  return jsc_value_new_boolean(context, value);
}
/**
 * Get a list of available users
 * @param instance The lightdm object instance
 */
static JSCValue *
LightDM_users_getter_cb(void)
{
  JSCContext *context = get_global_context();
  GList *users = lightdm_user_list_get_users(UserList);
  GPtrArray *arr = g_ptr_array_new();
  GList *curr = users;
  while (curr != NULL) {
    JSCValue *user = LightDMUser_to_JSCValue(context, curr->data);
    if (user != NULL)
      g_ptr_array_add(arr, user);
    curr = curr->next;
  }
  JSCValue *value = jsc_value_new_array_from_garray(context, arr);
  g_ptr_array_free(arr, true);
  return value;
}

/* LightDM callbacks */

static void
authentication_complete_cb(LightDMGreeter *greeter)
{
  (void) greeter;
  JSCContext *context = get_global_context();

  for (guint i = 0; i < greeter_browsers->len; i++) {
    GVariant *parameters = jsc_parameters_to_g_variant_array(context, "authentication_complete", NULL);
    WebKitUserMessage *message = webkit_user_message_new("lightdm", parameters);

    Browser *browser = greeter_browsers->pdata[i];
    webkit_web_view_send_message_to_page(WEBKIT_WEB_VIEW(browser->web_view), message, NULL, NULL, NULL);
  }
}
static void
autologin_timer_expired_cb(LightDMGreeter *greeter)
{
  (void) greeter;
  JSCContext *context = get_global_context();

  for (guint i = 0; i < greeter_browsers->len; i++) {
    GVariant *parameters = jsc_parameters_to_g_variant_array(context, "autologin_timer_expired", NULL);
    WebKitUserMessage *message = webkit_user_message_new("lightdm", parameters);

    Browser *browser = greeter_browsers->pdata[i];
    webkit_web_view_send_message_to_page(WEBKIT_WEB_VIEW(browser->web_view), message, NULL, NULL, NULL);
  }
}
static void
show_prompt_cb(LightDMGreeter *greeter, const gchar *text, LightDMPromptType type)
{
  (void) greeter;
  JSCContext *context = get_global_context();

  GPtrArray *arr = g_ptr_array_new();
  g_ptr_array_add(arr, jsc_value_new_string(context, text));
  g_ptr_array_add(arr, jsc_value_new_number(context, type));

  for (guint i = 0; i < greeter_browsers->len; i++) {
    GVariant *parameters = jsc_parameters_to_g_variant_array(context, "show_prompt", arr);
    WebKitUserMessage *message = webkit_user_message_new("lightdm", parameters);

    Browser *browser = greeter_browsers->pdata[i];
    webkit_web_view_send_message_to_page(WEBKIT_WEB_VIEW(browser->web_view), message, NULL, NULL, NULL);
  }
  g_ptr_array_free(arr, true);
}
static void
show_message_cb(LightDMGreeter *greeter, const gchar *text, LightDMMessageType type)
{
  (void) greeter;
  JSCContext *context = get_global_context();

  GPtrArray *arr = g_ptr_array_new();
  g_ptr_array_add(arr, jsc_value_new_string(context, text));
  g_ptr_array_add(arr, jsc_value_new_number(context, type));

  for (guint i = 0; i < greeter_browsers->len; i++) {
    GVariant *parameters = jsc_parameters_to_g_variant_array(context, "show_message", arr);
    WebKitUserMessage *message = webkit_user_message_new("lightdm", parameters);

    Browser *browser = greeter_browsers->pdata[i];
    webkit_web_view_send_message_to_page(WEBKIT_WEB_VIEW(browser->web_view), message, NULL, NULL, NULL);
  }
  g_ptr_array_free(arr, true);
}

/**
 * Connect LightDM signals to their callbacks
 */
void
LightDM_connect_signals(void)
{
  g_signal_connect(Greeter, "authentication-complete", G_CALLBACK(authentication_complete_cb), NULL);
  g_signal_connect(Greeter, "autologin-timer-expired", G_CALLBACK(autologin_timer_expired_cb), NULL);
  g_signal_connect(Greeter, "show-prompt", G_CALLBACK(show_prompt_cb), NULL);
  g_signal_connect(Greeter, "show-message", G_CALLBACK(show_message_cb), NULL);
}

/**
 * LightDM Class constructor, should be called only once in sea-greeter's life
 */
static void
LightDM_constructor(void)
{
  GError *err = NULL;

  LightDM_connect_signals();

  gboolean connected = lightdm_greeter_connect_to_daemon_sync(Greeter, &err);
  if (!connected && err) {
    logger_error("%s", err->message);
    g_error_free(err);
  }

  LightDMUser *user = lightdm_user_list_get_users(UserList)->data;
  if (user != NULL) {
    g_autofree gchar *user_data_dir
        = lightdm_greeter_ensure_shared_data_dir_sync(Greeter, lightdm_user_get_name(user), &err);

    if (user_data_dir != NULL) {
      int ind = string_get_last_index_of(user_data_dir, "/");
      g_autofree gchar *substr = g_utf8_substring(user_data_dir, 0, ind);

      shared_data_directory = g_string_new(substr);
    } else {
      shared_data_directory = g_string_new("");
    }
  } else {
    shared_data_directory = g_string_new("");
  }

  logger_debug("LightDM API connected");
}

void
handle_lightdm_accessor(BrowserWebView *web_view, WebKitUserMessage *message)
{
  bridge_object_handle_accessor(LightDM_object, web_view, message);
}

void
LightDM_destroy(void)
{
  g_object_unref(Greeter);
  g_object_unref(LightDM_object);
  g_string_free(shared_data_directory, true);
}

/**
 * Initialize the LightDM environment
 */
void
LightDM_initialize(void)
{
  UserList = lightdm_user_list_get_instance();
  Greeter = lightdm_greeter_new();

  LightDM_constructor();

  /**
   * The property type value is not being used in the main process.
   * It just serves as a help.
   */
  struct JSCClassProperty LightDM_properties[] = {
    { "authentication_user", G_CALLBACK(LightDM_authentication_user_getter_cb), NULL, G_TYPE_BOOLEAN },
    { "autologin_guest", G_CALLBACK(LightDM_autologin_guest_getter_cb), NULL, G_TYPE_BOOLEAN },
    { "autologin_timeout", G_CALLBACK(LightDM_autologin_timeout_getter_cb), NULL, G_TYPE_INT },
    { "autologin_user", G_CALLBACK(LightDM_autologin_user_getter_cb), NULL, G_TYPE_STRING },

    { "can_hibernate", G_CALLBACK(LightDM_can_hibernate_getter_cb), NULL, G_TYPE_BOOLEAN },
    { "can_restart", G_CALLBACK(LightDM_can_restart_getter_cb), NULL, G_TYPE_BOOLEAN },
    { "can_shutdown", G_CALLBACK(LightDM_can_shutdown_getter_cb), NULL, G_TYPE_BOOLEAN },
    { "can_suspend", G_CALLBACK(LightDM_can_suspend_getter_cb), NULL, G_TYPE_BOOLEAN },

    { "brightness", G_CALLBACK(LightDM_brightness_getter_cb), G_CALLBACK(LightDM_brightness_setter_cb), G_TYPE_INT },

    { "default_session", G_CALLBACK(LightDM_default_session_getter_cb), NULL, G_TYPE_STRING },
    { "has_guest_account", G_CALLBACK(LightDM_has_guest_account_getter_cb), NULL, G_TYPE_BOOLEAN },
    { "hide_users_hint", G_CALLBACK(LightDM_hide_users_hint_getter_cb), NULL, G_TYPE_BOOLEAN },
    { "hostname", G_CALLBACK(LightDM_hostname_getter_cb), NULL, G_TYPE_STRING },

    { "in_authentication", G_CALLBACK(LightDM_in_authentication_getter_cb), NULL, G_TYPE_BOOLEAN },
    { "is_authenticated", G_CALLBACK(LightDM_is_authenticated_getter_cb), NULL, G_TYPE_BOOLEAN },

    { "language", G_CALLBACK(LightDM_language_getter_cb), NULL, JSC_TYPE_VALUE },
    { "languages", G_CALLBACK(LightDM_languages_getter_cb), NULL, JSC_TYPE_VALUE },
    { "layout", G_CALLBACK(LightDM_layout_getter_cb), G_CALLBACK(LightDM_layout_setter_cb), JSC_TYPE_VALUE },
    { "layouts", G_CALLBACK(LightDM_layouts_getter_cb), NULL, JSC_TYPE_VALUE },

    { "lock_hint", G_CALLBACK(LightDM_lock_hint_getter_cb), NULL, G_TYPE_BOOLEAN },
    { "remote_sessions", G_CALLBACK(LightDM_remote_sessions_getter_cb), NULL, JSC_TYPE_VALUE },
    { "select_guest_hint", G_CALLBACK(LightDM_select_guest_hint_getter_cb), NULL, G_TYPE_BOOLEAN },
    { "select_user_hint", G_CALLBACK(LightDM_select_user_hint_getter_cb), NULL, G_TYPE_STRING },
    { "sessions", G_CALLBACK(LightDM_sessions_getter_cb), NULL, JSC_TYPE_VALUE },
    { "shared_data_directory", G_CALLBACK(LightDM_shared_data_directory_getter_cb), NULL, G_TYPE_STRING },
    { "show_manual_login_hint", G_CALLBACK(LightDM_show_manual_login_hint_getter_cb), NULL, G_TYPE_BOOLEAN },
    { "show_remote_login_hint", G_CALLBACK(LightDM_show_remote_login_hint_getter_cb), NULL, G_TYPE_BOOLEAN },
    { "users", G_CALLBACK(LightDM_users_getter_cb), NULL, JSC_TYPE_VALUE },
  };
  struct JSCClassMethod LightDM_methods[] = {
    { "authenticate", G_CALLBACK(LightDM_authenticate_cb), G_TYPE_BOOLEAN },
    { "authenticate_as_guest", G_CALLBACK(LightDM_authenticate_as_guest_cb), G_TYPE_BOOLEAN },
    { "cancel_authentication", G_CALLBACK(LightDM_cancel_authentication_cb), G_TYPE_BOOLEAN },
    { "cancel_autologin", G_CALLBACK(LightDM_cancel_autologin_cb), G_TYPE_BOOLEAN },
    { "hibernate", G_CALLBACK(LightDM_hibernate_cb), G_TYPE_BOOLEAN },
    { "respond", G_CALLBACK(LightDM_respond_cb), G_TYPE_BOOLEAN },
    { "restart", G_CALLBACK(LightDM_restart_cb), G_TYPE_BOOLEAN },
    { "set_language", G_CALLBACK(LightDM_set_language_cb), G_TYPE_BOOLEAN },
    { "shutdown", G_CALLBACK(LightDM_shutdown_cb), G_TYPE_BOOLEAN },
    { "start_session", G_CALLBACK(LightDM_start_session_cb), G_TYPE_BOOLEAN },
    { "suspend", G_CALLBACK(LightDM_suspend_cb), G_TYPE_BOOLEAN },
  };

  LightDM_object = bridge_object_new_full(
      "lightdm",
      LightDM_properties,
      G_N_ELEMENTS(LightDM_properties),
      LightDM_methods,
      G_N_ELEMENTS(LightDM_methods));
}
