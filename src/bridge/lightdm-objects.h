#ifndef BRIDGE_LIGHTDM_OBJECTS
#define BRIDGE_LIGHTDM_OBJECTS 1

#include <glib-object.h>
#include <jsc/jsc.h>
#include <lightdm-gobject-1/lightdm.h>

JSCValue * LightDMSession_to_JSCValue(JSCContext *context, LightDMSession *session);
JSCValue * LightDMUser_to_JSCValue(JSCContext *context, LightDMUser *user);
JSCValue * LightDMLanguage_to_JSCValue(JSCContext *context, LightDMLanguage *language);
JSCValue * LightDMLayout_to_JSCValue(JSCContext *context, LightDMLayout *layout);
LightDMLayout * JSCValue_to_LightDMLayout(JSCContext *context, JSCValue *object);

typedef struct _LDMObject {
  JSCContext *context;
  JSCValue *value;
} ldm_object;

#endif
