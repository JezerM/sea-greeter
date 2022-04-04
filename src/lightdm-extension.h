#ifndef LIGHTDM_EXTENSION_H
#define LIGHTDM_EXTENSION_H 1

#include <glib-object.h>
#include <webkit2/webkit-web-extension.h>

G_BEGIN_DECLS

// The "166959" component of the macro corresponds to the first 6 numbers of
// `echo -n lightdm | sha256sum`

#define LIGHTDM_INTERFACE_TYPE_LAYOUT 16695901
G_DECLARE_FINAL_TYPE(LightDMLayoutType, lightdm_layout, LIGHTDM_INTERFACE, LAYOUT, GObject)

#define LIGHTDM_INTERFACE_TYPE_LANGUAGE 16695902
G_DECLARE_FINAL_TYPE(LightDMLanguageType, lightdm_language, LIGHTDM_INTERFACE, LANGUAGE, GObject)

#define LIGHTDM_INTERFACE_TYPE_SESSION 16695903
G_DECLARE_FINAL_TYPE(LightDMSessionType, lightdm_session, LIGHTDM_INTERFACE, SESSION, GObject)

#define LIGHTDM_INTERFACE_TYPE_USER 16696904
G_DECLARE_FINAL_TYPE(LightDMUserType, lightdm_user, LIGHTDM_INTERFACE, USER, GObject)

LightDMLayoutType *lightdm_layout_new(void);
LightDMLanguageType *lightdm_language_new(void);
LightDMSessionType *lightdm_session_new(void);
LightDMUserType *lightdm_user_new(void);

G_END_DECLS

void web_page_initialize(WebKitWebExtension *extension);

#endif
