#include <webkit2/webkit-web-extension.h>

#include "bridge/greeter_config.h"
#include "bridge/theme_utils.h"
#include "bridge/lightdm-signal.h"

#include "extension/lightdm.h"

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
  /*LightDM_signal_initialize(world, web_page, web_frame, extension);*/
  /*LightDM_initialize(world, web_page, web_frame, extension);*/
  /*GreeterConfig_initialize(world, web_page, web_frame, extension);*/
  /*ThemeUtils_initialize(world, web_page, web_frame, extension);*/

  LightDM_Test_initialize(world, web_page, web_frame, extension);
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
