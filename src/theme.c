#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <webkit2/webkit2.h>

#include "logger.h"
#include "settings.h"

#include "browser.h"

extern GreeterConfig *greeter_config;

char *theme_dir = NULL;

/**
 * Loads the theme directory
 */
char *
load_theme_dir()
{
  const char *theme = greeter_config->greeter->theme;
  const char *dir = greeter_config->app->theme_dir;
  const char *def_theme = "gruvbox";

  char *final_dir = NULL;

  if (g_str_has_prefix(theme, "/")) {
    g_free(final_dir);
    final_dir = g_strdup(theme);
  } else if (strstr(theme, ".") || strstr(theme, "/")) {
    g_free(final_dir);
    final_dir = g_build_path("/", getcwd(NULL, 0), theme, NULL);
  } else {
    final_dir = g_build_path("/", dir, theme, NULL);
  }

  if (g_str_has_suffix(final_dir, ".html")) {
    char *dirname = g_path_get_dirname(final_dir);
    g_free(final_dir);
    final_dir = dirname;
  }

  if (access(final_dir, F_OK) != 0) {
    logger_warn("\"%s\" theme does not exists. Using \"%s\" theme", theme, def_theme);
    g_free(final_dir);
    final_dir = g_build_path("/", dir, def_theme, NULL);
  }

  g_free(theme_dir);
  theme_dir = final_dir;
  return theme_dir;
}

/**
 * Loads the primary theme path
 * The provided theme with `--theme` flag is preferred over index.yml
 */
char *
load_primary_theme_path()
{
  if (!theme_dir)
    load_theme_dir();

  const char *theme = greeter_config->greeter->theme;
  const char *theme_name = g_path_get_basename(theme);
  const char *dir = greeter_config->app->theme_dir;

  const char *def_theme = "gruvbox";

  if (g_str_has_suffix(theme_name, ".html")) {
    g_free(greeter_config->theme->primary_html);
    greeter_config->theme->primary_html = g_strdup(theme_name);
  }

  const char *primary = greeter_config->theme->primary_html;
  char *path_to_theme = g_build_path("/", theme_dir, primary, NULL);

  if (!g_str_has_suffix(path_to_theme, ".html")) {
    char *to_index = g_build_path("/", path_to_theme, "index.html", NULL);
    g_free(path_to_theme);
    path_to_theme = to_index;
  }

  if (access(path_to_theme, F_OK) != 0) {
    logger_warn("\"%s\" theme does not exists. Using \"%s\" theme", path_to_theme, def_theme);
    g_free(path_to_theme);
    path_to_theme = g_build_path("/", dir, def_theme, "index.html", NULL);
  }

  g_free(greeter_config->greeter->theme);
  greeter_config->greeter->theme = g_strdup(path_to_theme);
  return path_to_theme;
}

/**
 * Loads the secondary theme path
 * This can only be set with index.yml, either it defaults to primary html
 */
char *
load_secondary_theme_path()
{
  if (!theme_dir)
    load_theme_dir();

  const char *primary = greeter_config->theme->primary_html;
  const char *secondary = greeter_config->theme->secondary_html;
  char *path_to_theme = g_build_path("/", theme_dir, secondary != NULL ? secondary : primary, NULL);

  if (!g_str_has_suffix(path_to_theme, ".html")) {
    char *to_index = g_build_path("/", path_to_theme, "index.html", NULL);
    g_free(path_to_theme);
    path_to_theme = to_index;
  }

  if (access(path_to_theme, F_OK) != 0) {
    logger_warn("\"%s\" does not exists. Using \"%s\" for secondary monitors", path_to_theme, primary);
    g_free(path_to_theme);
    path_to_theme = load_primary_theme_path();
  }

  return path_to_theme;
}

void
load_theme(Browser *browser)
{
  WebKitWebView *web_view = WEBKIT_WEB_VIEW(browser->web_view);

  load_theme_dir();
  char *primary_html = load_primary_theme_path();
  char *secondary_html = load_secondary_theme_path();

  char *theme = NULL;

  if (gdk_monitor_is_primary(browser->monitor)) {
    theme = g_strdup(primary_html);
  } else {
    theme = g_strdup(secondary_html);
  }

  g_free(primary_html);
  g_free(secondary_html);

  FILE *file = fopen(theme, "r");
  if (!file) {
    logger_error("Theme couldn't be loaded");
    webkit_web_view_load_plain_text(web_view, "Theme couldn't be loaded.");
    g_free(theme);
    return;
  }
  GString *html = g_string_new(NULL);
  int c;
  while ((c = fgetc(file)) != EOF) {
    g_string_append_c(html, c);
  }
  fclose(file);

  char *uri = g_strconcat("file://", theme, NULL);
  webkit_web_view_load_html(web_view, html->str, uri);
  g_free(uri);
  g_string_free(html, true);
  g_free(theme);

  logger_debug("Theme loaded");
}
