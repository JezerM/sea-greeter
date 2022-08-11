#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <jsc/jsc.h>
#include <webkit2/webkit2.h>

#include "bridge/bridge-object.h"
#include "bridge/lightdm-objects.h"
#include "bridge/utils.h"

#include "browser.h"
#include "logger.h"
#include "utils/utils.h"

extern GPtrArray *greeter_browsers;

static BridgeObject *GreeterComm_object = NULL;

static void *
GreeterComm_broadcast_cb(GPtrArray *arguments)
{
  JSCContext *context = get_global_context();

  for (guint i = 0; i < greeter_browsers->len; i++) {
    GVariant *parameters = jsc_parameters_to_g_variant_array(context, "_emit", arguments);
    WebKitUserMessage *message = webkit_user_message_new("greeter_comm", parameters);

    Browser *browser = greeter_browsers->pdata[i];
    webkit_web_view_send_message_to_page(WEBKIT_WEB_VIEW(browser->web_view), message, NULL, NULL, NULL);
  }

  return NULL;
}
void
handle_greeter_comm_accessor(WebKitWebView *web_view, WebKitUserMessage *message)
{
  bridge_object_handle_accessor(GreeterComm_object, web_view, message);
}

void
GreeterComm_destroy()
{
  g_object_unref(GreeterComm_object);
}

/**
 * Initialize GreeterComm
 */
void
GreeterComm_initialize()
{
  struct JSCClassMethod Comm_methods[] = {
    { "broadcast", G_CALLBACK(GreeterComm_broadcast_cb), G_TYPE_NONE },
  };

  GreeterComm_object = bridge_object_new_full("greeter_comm", NULL, 0, Comm_methods, G_N_ELEMENTS(Comm_methods));
}
