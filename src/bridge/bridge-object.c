#include <jsc/jsc.h>
#include <webkit2/webkit2.h>

#include "bridge/bridge-object.h"
#include "bridge/utils.h"
#include "browser-web-view.h"
#include "utils/utils.h"

G_DEFINE_TYPE(BridgeObject, bridge_object, G_TYPE_OBJECT)

typedef enum {
  PROP_PROPERTIES = 1,
  PROP_NAME,
  PROP_METHODS,
  N_PROPERTIES,
} BridgeObjectProperty;

static GParamSpec *bridge_object_properties[N_PROPERTIES] = { NULL };

static void
bridge_object_free_property(gpointer data)
{
  struct JSCClassProperty *prop = data;
  g_free(prop);
}
static void
bridge_object_free_method(gpointer data)
{
  struct JSCClassMethod *method = data;
  g_free(method);
}

static void
bridge_object_dispose(GObject *object)
{
  G_OBJECT_CLASS(bridge_object_parent_class)->dispose(object);
  BridgeObject *self = BRIDGE_OBJECT(object);

  g_free(self->name);
  g_ptr_array_free(self->properties, true);
  g_ptr_array_free(self->methods, true);
}

static void
sea_bridge_constructed(GObject *object)
{
  G_OBJECT_CLASS(bridge_object_parent_class)->constructed(object);
}

static void
bridge_object_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  (void) pspec;
  BridgeObject *self = BRIDGE_OBJECT(object);
  switch ((BridgeObjectProperty) property_id) {
    case PROP_NAME:
      if (self->name != NULL)
        g_free(self->name);
      self->name = g_value_dup_string(value);
      break;
    case PROP_PROPERTIES:
      self->properties = g_value_get_pointer(value);
      break;
    case PROP_METHODS:
      self->methods = g_value_get_pointer(value);
    default:
      break;
  }
}
static void
bridge_object_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  (void) pspec;
  BridgeObject *self = BRIDGE_OBJECT(object);
  switch ((BridgeObjectProperty) property_id) {
    case PROP_NAME:
      g_value_set_string(value, self->name);
      break;
    case PROP_PROPERTIES:
      g_value_set_pointer(value, self->properties);
      break;
    case PROP_METHODS:
      g_value_set_pointer(value, self->methods);
    default:
      break;
  }
}

static void
bridge_object_class_init(BridgeObjectClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = bridge_object_set_property;
  object_class->get_property = bridge_object_get_property;

  object_class->dispose = bridge_object_dispose;
  object_class->constructed = sea_bridge_constructed;

  bridge_object_properties[PROP_NAME]
      = g_param_spec_string("name", "Name", "Bridge name", NULL, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  bridge_object_properties[PROP_PROPERTIES]
      = g_param_spec_pointer("properties", "Properties", "Bridge properties", G_PARAM_READWRITE);

  bridge_object_properties[PROP_METHODS]
      = g_param_spec_pointer("methods", "Methods", "Bridge methods", G_PARAM_READWRITE);

  g_object_class_install_properties(object_class, N_PROPERTIES, bridge_object_properties);
}

static void
bridge_object_init(BridgeObject *self)
{
  self->properties = g_ptr_array_new_with_free_func(bridge_object_free_property);
  self->methods = g_ptr_array_new_with_free_func(bridge_object_free_method);
}

static void
bridge_object_handle_property(
    BridgeObject *self,
    WebKitUserMessage *message,
    const gchar *method,
    GPtrArray *parameters,
    BrowserWebView *web_view)
{
  if (self->properties->len == 0)
    return;

  for (guint i = 0; i < self->properties->len; i++) {
    struct JSCClassProperty *current = self->properties->pdata[i];
    /*printf("Current: %d - %s\n", i, current->name);*/

    if (g_strcmp0(current->name, method) == 0) {

      if (parameters->len > 0) {
        JSCValue *param = parameters->pdata[0];
        ((void (*)(JSCValue *, BrowserWebView *)) current->setter)(param, web_view);
        WebKitUserMessage *empty_msg = webkit_user_message_new("", NULL);
        webkit_user_message_send_reply(message, empty_msg);
        break;
      }

      g_autoptr(JSCValue) jsc_value = ((JSCValue * (*) (BrowserWebView * web_view)) current->getter)(web_view);
      g_autofree gchar *json_value = g_strdup("undefined");
      if (JSC_IS_VALUE(jsc_value)) {
        json_value = jsc_value_to_json(jsc_value, 0);
      }
      /*printf("JSON value: '%s'\n", json_value);*/

      GVariant *value = g_variant_new_string(json_value);
      WebKitUserMessage *reply = webkit_user_message_new("reply", value);

      webkit_user_message_send_reply(message, reply);
      break;
    }
  }
}
static void
bridge_object_handle_method(
    BridgeObject *self,
    WebKitUserMessage *message,
    const gchar *method,
    GPtrArray *parameters,
    BrowserWebView *web_view)
{
  if (self->methods->len == 0)
    return;

  for (guint i = 0; i < self->methods->len; i++) {
    struct JSCClassMethod *current = self->methods->pdata[i];
    /*printf("Current: %d - %s\n", i, current->name);*/

    if (g_strcmp0(current->name, method) == 0) {
      g_autoptr(JSCValue) jsc_value
          = ((JSCValue * (*) (GPtrArray *, BrowserWebView *) ) current->callback)(parameters, web_view);
      g_autofree gchar *json_value = g_strdup("undefined");
      if (JSC_IS_VALUE(jsc_value)) {
        json_value = jsc_value_to_json(jsc_value, 0);
      }
      /*printf("JSON value: '%s'\n", json_value);*/

      GVariant *value = g_variant_new_string(json_value);
      WebKitUserMessage *reply = webkit_user_message_new("reply", value);

      webkit_user_message_send_reply(message, reply);
      break;
    }
  }
}

void
bridge_object_handle_accessor(BridgeObject *self, BrowserWebView *web_view, WebKitUserMessage *message)
{

  const char *name = webkit_user_message_get_name(message);
  if (g_strcmp0(name, self->name) != 0)
    return;

  g_autoptr(WebKitUserMessage) empty_msg = webkit_user_message_new("", NULL);
  GVariant *msg_param = webkit_user_message_get_parameters(message);

  if (!g_variant_is_of_type(msg_param, G_VARIANT_TYPE_ARRAY)) {
    webkit_user_message_send_reply(message, empty_msg);
    return;
  }
  int parameters_length = g_variant_n_children(msg_param);
  if (parameters_length == 0 || parameters_length > 2) {
    webkit_user_message_send_reply(message, empty_msg);
    return;
  }

  JSCContext *context = get_global_context();

  GVariant *method_var = g_variant_get_child_value(msg_param, 0);
  GVariant *params_var = g_variant_get_child_value(msg_param, 1);

  const gchar *method = g_variant_to_string(method_var);
  const gchar *json_params = g_variant_to_string(params_var);
  /*printf("Handling: '%s.%s'\n", name, method);*/
  /*printf("JSON params: '%s'\n", json_params);*/

  g_variant_unref(method_var);
  g_variant_unref(params_var);
  if (method == NULL) {
    webkit_user_message_send_reply(message, empty_msg);
    return;
  }
  g_autoptr(JSCValue) parameters = jsc_value_new_from_json(context, json_params);

  g_autoptr(GPtrArray) g_array = jsc_array_to_g_ptr_array(parameters);

  bridge_object_handle_property(self, message, method, g_array, web_view);
  bridge_object_handle_method(self, message, method, g_array, web_view);
}

BridgeObject *
bridge_object_new(const gchar *name)
{
  return g_object_new(BRIDGE_TYPE_OBJECT, "name", name, NULL);
}
BridgeObject *
bridge_object_new_full(
    const gchar *name,
    const struct JSCClassProperty *properties,
    guint properties_length,
    const struct JSCClassMethod *methods,
    guint methods_length)
{
  g_autoptr(GPtrArray) props = g_ptr_array_new_full(properties_length, bridge_object_free_property);

  for (guint i = 0; i < properties_length; i++) {
    struct JSCClassProperty *prop = malloc(sizeof *prop);
    prop->name = properties[i].name;
    prop->property_type = properties[i].property_type;
    prop->getter = properties[i].getter;
    prop->setter = properties[i].setter;
    g_ptr_array_add(props, prop);
  }

  g_autoptr(GPtrArray) mets = g_ptr_array_new_full(methods_length, bridge_object_free_method);

  for (guint i = 0; i < methods_length; i++) {
    struct JSCClassMethod *method = malloc(sizeof *method);
    method->name = methods[i].name;
    method->return_type = methods[i].return_type;
    method->callback = methods[i].callback;
    g_ptr_array_add(mets, method);
  }

  return g_object_new(
      BRIDGE_TYPE_OBJECT,
      "name",
      name,
      "properties",
      g_steal_pointer(&props),
      "methods",
      g_steal_pointer(&mets),
      NULL);
}
