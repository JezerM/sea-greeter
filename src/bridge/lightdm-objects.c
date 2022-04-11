#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <webkit2/webkit-web-extension.h>
#include <JavaScriptCore/JavaScript.h>
#include <lightdm-gobject-1/lightdm.h>

#include "settings.h"
#include "logger.h"
#include "lightdm-extension.h"
#include "lightdm-objects.h"

JSCValue *
LightDMUser_to_JSCValue(JSCContext *context, LightDMUser *user) {
  JSCValue *value = jsc_value_new_object(context, NULL, NULL);

  const gchar *background = lightdm_user_get_background(user);
  const gchar *display_name = lightdm_user_get_display_name(user);
  const gchar *home_directory = lightdm_user_get_home_directory(user);
  const gchar *image = lightdm_user_get_image(user);
  const gchar *language = lightdm_user_get_language(user);
  const gchar *layout = lightdm_user_get_layout(user);
  const gchar *const *layouts = lightdm_user_get_layouts(user);
  const gboolean logged_in = lightdm_user_get_logged_in(user);
  const gchar *session = lightdm_user_get_session(user);
  const gchar *username = lightdm_user_get_name(user);

  GPtrArray *layouts_array = g_ptr_array_new();
  int i = 0;
  const gchar *curr;
  while ((curr = layouts[i]) != NULL) {
    g_ptr_array_add(layouts_array,
        jsc_value_new_string(context, curr));
    i++;
  }

  jsc_value_object_set_property(value, "background",
      jsc_value_new_string(context, background));
  jsc_value_object_set_property(value, "display_name",
      jsc_value_new_string(context, display_name));
  jsc_value_object_set_property(value, "home_directory",
      jsc_value_new_string(context, home_directory));
  jsc_value_object_set_property(value, "image",
      jsc_value_new_string(context, image));
  jsc_value_object_set_property(value, "language",
      jsc_value_new_string(context, language));
  jsc_value_object_set_property(value, "layout",
      jsc_value_new_string(context, layout));
  jsc_value_object_set_property(value, "layouts",
      jsc_value_new_array_from_garray(context, layouts_array));
  jsc_value_object_set_property(value, "logged_in",
      jsc_value_new_boolean(context, logged_in));
  jsc_value_object_set_property(value, "session",
      jsc_value_new_string(context, session));
  jsc_value_object_set_property(value, "username",
      jsc_value_new_string(context, username));
  return value;
}


// Esto es un proyecto aún no publicado
// hecho en C que utiliza WebKitGtk y LightDM para
// iniciar sesión en Linux :D

JSCValue *
LightDMSession_to_JSCValue(JSCContext *context, LightDMSession *session) {
  JSCValue *value = jsc_value_new_object(context, NULL, NULL);

  const gchar *comment = lightdm_session_get_comment(session);
  const gchar *key = lightdm_session_get_key(session);
  const gchar *name = lightdm_session_get_name(session);
  const gchar *type = lightdm_session_get_session_type(session);

  jsc_value_object_set_property(value, "comment",
      jsc_value_new_string(context, comment));
  jsc_value_object_set_property(value, "key",
      jsc_value_new_string(context, key));
  jsc_value_object_set_property(value, "name",
      jsc_value_new_string(context, name));
  jsc_value_object_set_property(value, "type",
      jsc_value_new_string(context, type));
  return value;
}
JSCValue *
LightDMLanguage_to_JSCValue(JSCContext *context, LightDMLanguage *language) {
  JSCValue *value = jsc_value_new_object(context, NULL, NULL);

  const gchar *code = lightdm_language_get_code(language);
  const gchar *name = lightdm_language_get_name(language);
  const gchar *territory = lightdm_language_get_territory(language);

  jsc_value_object_set_property(value, "code",
      jsc_value_new_string(context, code));
  jsc_value_object_set_property(value, "name",
      jsc_value_new_string(context, name));
  jsc_value_object_set_property(value, "territory",
      jsc_value_new_string(context, territory));
  return value;
}
