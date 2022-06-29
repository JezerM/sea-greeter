#ifndef LIGHTDM_EXTENSION_H
#define LIGHTDM_EXTENSION_H 1

#include <glib-object.h>
#include <webkit2/webkit-web-extension.h>

G_BEGIN_DECLS

// The "166959" component of the macro corresponds to the first 6 numbers of
// `echo -n lightdm | sha256sum`

#define LIGHTDM_INTERFACE_TYPE_LAYOUT lightdm_layout_type_get_type()
G_DECLARE_FINAL_TYPE(LightDMLayoutType, lightdm_layout_type, LIGHTDM_INTERFACE, LAYOUT, GObject)

#define LIGHTDM_INTERFACE_TYPE_LANGUAGE 16695902
G_DECLARE_FINAL_TYPE(LightDMLanguageType, lightdm_language_type, LIGHTDM_INTERFACE, LANGUAGE, GObject)

#define LIGHTDM_INTERFACE_TYPE_SESSION 16695903
G_DECLARE_FINAL_TYPE(LightDMSessionType, lightdm_session_type, LIGHTDM_INTERFACE, SESSION, GObject)

#define LIGHTDM_INTERFACE_TYPE_USER 16696904
G_DECLARE_FINAL_TYPE(LightDMUserType, lightdm_user_type, LIGHTDM_INTERFACE, USER, GObject)


struct _LightDMLayoutType {
  GObject parent_instance;
  GString *name;
  GString *description;
  GString *short_description;
};

LightDMLayoutType *lightdm_layout_type_new(const gchar *name, const gchar *description, const gchar *short_description);
LightDMLanguageType *lightdm_language_type_new(void);
LightDMSessionType *lightdm_session_type_new(void);
LightDMUserType *lightdm_user_type_new(void);

G_END_DECLS

void web_page_initialize(WebKitWebExtension *extension);

typedef struct _LDMObject {
  JSCContext *context;
  JSCValue *value;
} ldm_object;

#endif
