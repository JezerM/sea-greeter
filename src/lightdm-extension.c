#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <webkitdom/webkitdom.h>
#include <webkit2/webkit-web-extension.h>
#include <JavaScriptCore/JavaScript.h>
#include <lightdm-gobject-1/lightdm.h>

#include "lightdm/user.h"
#include "settings.h"
#include "logger.h"
#include "lightdm-extension.h"

static LightDMGreeter *Greeter;
static LightDMUserList *UserList;
static WebKitWebExtension *WebExtension;
extern guint64 page_id;

static JSCClass *LightDM_class;
static GString *shared_data_directory;

typedef struct _LDMObject {
  JSCContext *context;
  JSCValue *value;
} ldm_object;

struct _LightDMLayoutType {
  gchar *name;
};

struct JSCClassProperty {
  const gchar *name;
  GCallback getter;
  GCallback setter;
  GType property_type;
};
struct JSCClassMethod {
  const gchar *name;
  GCallback callback;
  GType return_type;
};

static gchar *
js_value_to_string(JSCValue *value) {
  if (!jsc_value_is_string(value)) return NULL;
  return jsc_value_to_string(value);
}

/* LightDM API definitions */

static JSCValue *
authenticate_cb(
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
static JSCValue *
respond_cb(
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
static JSCValue *
is_authenticated_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  gboolean value = lightdm_greeter_get_is_authenticated(Greeter);
  return jsc_value_new_boolean(context, value);
}

static GPtrArray *
g_list_to_gptrarray(GList *list) {
  GPtrArray *arr = g_ptr_array_new();

  GList *curr = list;
  while (curr != NULL) {
    printf("%p\n", curr->data);
    g_ptr_array_add(arr, curr->data);
    curr = curr->next;
  }

  LightDMUser *user = arr->pdata[0];
  printf("%s\n", lightdm_user_get_name(user));

  return arr;
}

static JSCValue *
LightDM_users_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  GList *users = lightdm_user_list_get_users(UserList);
  GPtrArray *arr = g_list_to_gptrarray(users);
  /*return jsc_value_new_boolean(context, true);*/
  return jsc_value_new_array_from_garray(context, arr);
}

static void
authentication_complete_cb(
    LightDMGreeter *greeter,
    WebKitWebExtension *extension)
{

}
static void autologin_timer_expired_cb(
    LightDMGreeter *greeter,
    WebKitWebExtension *extension)
{
}
static void show_prompt_cb(
    LightDMGreeter *greeter,
    const gchar *text,
    LightDMPromptType type,
    WebKitWebExtension *extension)
{

  WebKitWebPage *web_page = webkit_web_extension_get_page(extension, page_id);

  if (web_page) {
    WebKitFrame *web_frame = webkit_web_page_get_main_frame(web_page);
    JSCContext *js_context = webkit_frame_get_js_context(web_frame);
  }

  switch (type) {
    case LIGHTDM_PROMPT_TYPE_QUESTION:
      printf("USER:\n");
      break;
    case LIGHTDM_PROMPT_TYPE_SECRET:
      printf("PASSWORD:\n");
      break;
  }
}
static void show_message_cb(
    LightDMGreeter *greeter,
    const gchar *text,
    LightDMMessageType type,
    WebKitWebExtension *extension)
{
}

#define G_TYPE_ARRAY_POST -1

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
  /*{"is_authenticated", NULL, NULL, G_TYPE_BOOLEAN},*/

  /*{"language", NULL, NULL, LIGHTDM_INTERFACE_TYPE_LANGUAGE},*/
  /*{"languages", NULL, NULL, G_TYPE_VARIANT},*/
  /*{"layout", NULL, NULL, LIGHTDM_INTERFACE_TYPE_LAYOUT},*/
  /*{"layouts", NULL, NULL, G_TYPE_VARIANT},*/

  /*{"lock_hint", NULL, NULL, G_TYPE_BOOLEAN},*/
  /*{"sessions", NULL, NULL, G_TYPE_VARIANT},*/
  {"users", G_CALLBACK(LightDM_users_getter_cb), NULL, G_TYPE_ARRAY_POST},

  {NULL, NULL, NULL, 0},
};

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

int g_string_get_index_of(GString *source, GString *find) {
  gchar *found = strstr(source->str, find->str);
  if (found != NULL) return found - source->str;
  return -1;
}

int g_string_get_last_index_of(GString *source, GString *find) {
  int index = -1, tmp;
  GString *str = g_string_new(source->str);
  while ((tmp = g_string_get_index_of(str, find)) != -1) {
    str = g_string_erase(str, 0, tmp + find->len);
    index += tmp + find->len;
  }
  return index;
}

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

static void initialize_properties(
    JSCClass *class,
    struct JSCClassProperty properties[])
{
  int i = 0;
  struct JSCClassProperty current = properties[i];
  while (current.name != NULL) {
    switch (current.property_type) {
      case G_TYPE_ARRAY_POST:
        current.property_type = G_TYPE_ARRAY;
        break;
    }
    jsc_class_add_property(
        class,
        current.name,
        current.property_type,
        current.getter,
        current.setter,
        NULL,
        NULL
        );
    i++;
    current = properties[i];
  }
}

static void
LightDM_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension) {
  (void) web_page;
  (void) extension;

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
  jsc_class_add_method_variadic(
      LightDM_class,
      "authenticate",
      G_CALLBACK(authenticate_cb),
      NULL, NULL,
      G_TYPE_BOOLEAN);
  jsc_class_add_method_variadic(
      LightDM_class,
      "respond",
      G_CALLBACK(respond_cb),
      NULL, NULL,
      G_TYPE_BOOLEAN);
  /*jsc_class_add_property(*/
      /*LightDM_class,*/
      /*"is_authenticated",*/
      /*G_TYPE_BOOLEAN,*/
      /*NULL,*/
      /*NULL, NULL, NULL);*/
  initialize_properties(LightDM_class, LightDM_properties);

  JSCValue *value = jsc_value_constructor_callv(ldm_constructor, 0, NULL);
  ldm_object *instance = malloc(sizeof *instance);
  instance->value = value;
  instance->context = js_context;
  JSCValue *ldm_object = jsc_value_new_object(js_context, instance, LightDM_class);

  jsc_value_object_set_property(
      global_object,
      "lightdmg",
      ldm_object
      );
}

void web_page_initialize(WebKitWebExtension *extension) {
  Greeter = lightdm_greeter_new();
  UserList = lightdm_user_list_get_instance();

  WebExtension = extension;

  g_signal_connect(
    webkit_script_world_get_default(),
    "window-object-cleared",
    G_CALLBACK(LightDM_initialize),
    extension
  );

  /*GError *err = NULL;*/
  /*if (!lightdm_greeter_authenticate(Greeter, NULL, &err)) {*/
    /*printf("ERROR: %s\n", err->message);*/
  /*}*/
}
