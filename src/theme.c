#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <webkit2/webkit2.h>
#include <yaml.h>

#include "logger.h"
#include "settings.h"

#include "browser.h"

extern GreeterConfig *greeter_config;

char *theme_dir = NULL;

#define WEB_GREETER_THEMES_PATH "/usr/share/web-greeter/themes/"

static gint
sort_ptr_array(gconstpointer a, gconstpointer b)
{
  const char *entry1 = *((char **) a);
  const char *entry2 = *((char **) b);

  char *entry1_f = g_utf8_casefold(entry1, strlen(entry1));
  char *entry2_f = g_utf8_casefold(entry2, strlen(entry2));

  const gboolean value = strcmp(entry1_f, entry2_f) > 0;

  g_free(entry1_f);
  g_free(entry2_f);

  return value;
}

GPtrArray *
list_themes()
{
  DIR *dir;
  struct dirent *ent;
  dir = opendir(WEB_GREETER_THEMES_PATH);
  if (dir == NULL) {
    printf("There are no themes located at %s\n", WEB_GREETER_THEMES_PATH);
    return NULL;
  }

  GPtrArray *themes = g_ptr_array_new();

  while ((ent = readdir(dir)) != NULL) {
    char *file_name = ent->d_name;
    if (g_strcmp0(file_name, ".") == 0 || g_strcmp0(file_name, "..") == 0) {
      continue;
    }
    char *file_path = g_build_path("/", WEB_GREETER_THEMES_PATH, file_name, NULL);
    struct stat file_stat;
    stat(file_path, &file_stat);
    if (S_ISDIR(file_stat.st_mode)) {
      g_ptr_array_add(themes, file_name);
    }
  }

  g_ptr_array_sort(themes, sort_ptr_array);

  return themes;
}

void
print_themes()
{
  GPtrArray *themes = list_themes();
  if (themes == NULL)
    return;

  printf("Themes are located in %s\n\n", WEB_GREETER_THEMES_PATH);

  for (uint i = 0; i < themes->len; i++) {
    char *theme = themes->pdata[i];
    printf("- %s\n", theme);
  }

  g_ptr_array_free(themes, true);
}

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
  char *theme_name = g_path_get_basename(theme);
  const char *dir = greeter_config->app->theme_dir;

  const char *def_theme = "gruvbox";

  if (g_str_has_suffix(theme_name, ".html")) {
    g_free(greeter_config->theme->primary_html);
    greeter_config->theme->primary_html = g_strdup(theme_name);
  }
  g_free(theme_name);

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

enum storage_flags { VAR, VAL, SEQ };

static void
process_layer(yaml_parser_t *parser, GNode *data)
{
  GNode *last_leaf = data;
  yaml_event_t event;
  int storage = VAR;

  while (1) {
    yaml_parser_parse(parser, &event);

    if (event.type == YAML_SCALAR_EVENT) {
      if (storage)
        g_node_append_data(last_leaf, g_strdup((gchar *) event.data.scalar.value));
      else
        last_leaf = g_node_append(data, g_node_new(g_strdup((gchar *) event.data.scalar.value)));
      storage ^= VAL;
    } else if (event.type == YAML_SEQUENCE_START_EVENT)
      storage = SEQ;
    else if (event.type == YAML_SEQUENCE_END_EVENT)
      storage = VAR;
    else if (event.type == YAML_MAPPING_START_EVENT) {
      process_layer(parser, last_leaf);
      storage ^= VAL;
    } else if (event.type == YAML_MAPPING_END_EVENT || event.type == YAML_STREAM_END_EVENT)
      break;

    yaml_event_delete(&event);
  }
}

static gboolean
node_free(GNode *node, gpointer data)
{
  (void) data;
  char *str = node->data;
  if (str != NULL) {
    g_free(str);
    node->data = NULL;
  }
  return false;
}

void
load_theme_config()
{
  if (!theme_dir)
    theme_dir = load_theme_dir();

  char *path_to_theme_config = g_build_path("/", theme_dir, "index.yml", NULL);
  FILE *file = fopen(path_to_theme_config, "rb");
  yaml_parser_t parser;

  if (!yaml_parser_initialize(&parser) || file == NULL) {
    logger_warn("Theme config was not loaded:\n\t%s", strerror(errno));
  }

  yaml_parser_set_input_file(&parser, file);

  GNode *cfg = g_node_new(g_strdup(path_to_theme_config));
  process_layer(&parser, cfg);

  yaml_parser_delete(&parser);
  fclose(file);

  GNode *node = cfg->children;

  do {
    /*printf("'%s'\n", (char *) node->data);*/
    if (g_strcmp0(node->data, "primary_html") == 0) {
      GNode *children = node->children;
      if (children != NULL) {
        char *value = children->data;
        greeter_config->theme->primary_html = g_strdup(value);
      }
    } else if (g_strcmp0(node->data, "secondary_html") == 0) {
      GNode *children = node->children;
      if (children != NULL) {
        char *value = children->data;
        greeter_config->theme->secondary_html = g_strdup(value);
      }
    }
    node = node->next;
  } while (node != NULL);

  g_node_traverse(cfg, G_PRE_ORDER, G_TRAVERSE_ALL, -1, node_free, NULL);
  g_node_destroy(cfg);

  g_free(path_to_theme_config);
}

void
load_theme(Browser *browser)
{
  WebKitWebView *web_view = WEBKIT_WEB_VIEW(browser->web_view);

  load_theme_dir();
  char *primary_html = load_primary_theme_path();
  char *secondary_html = load_secondary_theme_path();

  char *theme = NULL;

  if (browser->is_primary) {
    theme = g_strdup(primary_html);
  } else {
    theme = g_strdup(secondary_html);
  }

  g_free(primary_html);
  g_free(secondary_html);

  char *uri = g_strconcat("file://", theme, NULL);
  webkit_web_view_load_uri(web_view, uri);
  g_free(uri);
  g_free(theme);

  logger_debug("Theme loaded");
}
