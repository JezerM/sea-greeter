#ifndef BRIDGE_LIGHTDM_OBJECTS
#define BRIDGE_LIGHTDM_OBJECTS 1

#include <glib-object.h>
#include <webkit2/webkit-web-extension.h>
#include <lightdm-gobject-1/lightdm.h>

#include "lightdm-extension.h"

JSCValue * LightDMSession_to_JSCValue(JSCContext *context, LightDMSession *session);
JSCValue * LightDMUser_to_JSCValue(JSCContext *context, LightDMUser *user);
JSCValue * LightDMLanguage_to_JSCValue(JSCContext *context, LightDMLanguage *language);
JSCValue * LightDMLayout_to_JSCValue(JSCContext *context, LightDMLayout *layout);
LightDMLayoutType * LightDMLayout_to_type(LightDMLayout *layout);
LightDMLayout * JSCValue_to_LightDMLayout(JSCContext *context, JSCValue *object);

#endif
