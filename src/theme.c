#include "logger.h"
#include "settings.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <webkit2/webkit2.h>

extern GreeterConfig *greeter_config;

void
load_theme(WebKitWebView *web_view)
{
  char *theme = greeter_config->greeter->theme;
  char *dir = greeter_config->app->theme_dir;
  char *path_to_theme = g_strconcat(dir, theme, "/index.html", NULL);
  const char *def_theme = "gruvbox";

  if (theme[0] == '/') {
    g_free(path_to_theme);
    path_to_theme = g_strdup(theme);
  } else if (strstr(theme, ".") || strstr(theme, "/")) {
    g_free(path_to_theme);
    path_to_theme = g_strconcat(getcwd(NULL, 0), "/", theme, NULL);
  }

  if (!g_str_has_suffix(path_to_theme, ".html")) {
    path_to_theme = g_strconcat(path_to_theme, "/index.html", NULL);
  }

  if (access(path_to_theme, F_OK) != 0) {
    logger_warn("%s", g_strdup_printf("\"%s\" theme does not exists. Using \"%s\" theme", theme, def_theme));
    g_free(path_to_theme);
    path_to_theme = g_strconcat(dir, def_theme, "/index.html", NULL);
  }

  g_free(greeter_config->greeter->theme);
  greeter_config->greeter->theme = g_strdup(path_to_theme);

  g_free(path_to_theme);

  FILE *file = fopen(greeter_config->greeter->theme, "r");
  if (!file) {
    logger_error("Theme couldn't be loaded");
    webkit_web_view_load_plain_text(web_view, "Theme couldn't be loaded.");
    return;
  }
  GString *html = g_string_new(NULL);
  int c;
  while ((c = fgetc(file)) != EOF) {
    g_string_append_c(html, c);
  }
  fclose(file);

  char *uri = g_strconcat("file://", greeter_config->greeter->theme, NULL);
  webkit_web_view_load_html(web_view, html->str, uri);
  g_free(uri);
  g_string_free(html, true);

  logger_debug("Theme loaded");
}
