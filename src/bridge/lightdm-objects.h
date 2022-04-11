#ifndef BRIDGE_LIGHTDM_OBJECTS
#define BRIDGE_LIGHTDM_OBJECTS 1

#include <glib-object.h>
#include <webkit2/webkit-web-extension.h>
#include <lightdm-gobject-1/lightdm.h>

JSCValue * LightDMSession_to_JSCValue(JSCContext *context, LightDMSession *session);
JSCValue * LightDMUser_to_JSCValue(JSCContext *context, LightDMUser *user);

#endif
