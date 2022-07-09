#ifndef SEA_GREETER_H
#define SEA_GREETER_H

#include <webkit2/webkit2.h>

typedef struct _GtkBrowser {
  WebKitWebView *web_view;
  GtkApplicationWindow *window;
} GtkBrowser;

#endif
