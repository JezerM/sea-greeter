#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <webkit2/webkit-web-extension.h>
#include <JavaScriptCore/JavaScript.h>
#include <lightdm-gobject-1/lightdm.h>

#include "settings.h"
#include "logger.h"
#include "lightdm-extension.h"
#include "bridge/lightdm-objects.h"
#include "bridge/lightdm.h"
#include "bridge/greeter_config.h"
#include "bridge/theme_utils.h"
#include "bridge/lightdm-signal.h"

WebKitWebExtension *WebExtension;
extern guint64 page_id;

static void
extension_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebExtension *extension) {
  (void) web_page;
  (void) extension;

  LightDM_signal_initialize(world, web_page, web_frame, extension);
  LightDM_initialize(world, web_page, web_frame, extension);
  GreeterConfig_initialize(world, web_page, web_frame, extension);
  ThemeUtils_initialize(world, web_page, web_frame, extension);
}

void web_page_initialize(WebKitWebExtension *extension) {
  WebExtension = extension;
  g_signal_connect(
    webkit_script_world_get_default(),
    "window-object-cleared",
    G_CALLBACK(extension_initialize),
    extension
  );
}
