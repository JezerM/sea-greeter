#include "settings.h"
#include "logger.h"
#include <glib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>

GreeterConfig *greeter_config;

static void
init_greeter_config_branding()
{
  GreeterConfigBranding *branding = greeter_config->branding;
  branding = malloc(sizeof *branding);
  branding->background_images_dir = g_string_new(NULL);
  branding->logo_image = g_string_new(NULL);
  branding->user_image = g_string_new(NULL);
  greeter_config->branding = branding;
}
static void
init_greeter_config_greeter()
{
  GreeterConfigGreeter *greeter = greeter_config->greeter;
  greeter = malloc(sizeof *greeter);
  greeter->debug_mode = false;
  greeter->detect_theme_errors = true;
  greeter->screensaver_timeout = 300;
  greeter->secure_mode = true;
  greeter->theme = g_string_new("gruvbox");
  greeter->icon_theme = g_string_new(NULL);
  greeter->time_language = g_string_new(NULL);
  greeter_config->greeter = greeter;
}
static void
init_greeter_config_features()
{
  GreeterConfigFeatures *features = greeter_config->features;
  features = malloc(sizeof *features);
  features->battery = false;
  features->backlight = malloc(sizeof *(features->backlight));
  features->backlight->enabled = false;
  features->backlight->steps = 0;
  features->backlight->value = 10;
  greeter_config->features = features;
}
static void
init_greeter_config_app()
{
  GreeterConfigApp *app = greeter_config->app;
  app = malloc(sizeof *app);
  app->debug_mode = false;
  app->fullscreen = true;
  app->theme_dir = g_string_new("/usr/share/web-greeter/themes/");
  greeter_config->app = app;
}

void
print_greeter_config()
{
  GString *layouts = g_string_new("");
  GList *actual = greeter_config->layouts;
  if (actual == NULL)
    g_string_append(layouts, "\n");
  else {
    while (actual != NULL) {
      if (actual->data != NULL)
        g_string_append_printf(layouts, "  \"%s\"\n", (char *) actual->data);
      actual = actual->next;
    }
  }
  printf(
      "branding:\n"
      "  background_images_dir: \"%s\"\n"
      "  logo_image: \"%s\"\n"
      "  user_image: \"%s\"\n"
      "greeter:\n"
      "  debug_mode: %d\n"
      "  detect_theme_errors: %d\n"
      "  screensaver_timeout: %d\n"
      "  theme: \"%s\"\n"
      "  icon_theme: \"%s\"\n"
      "  time_language: \"%s\"\n"
      "layouts:\n"
      "%s"
      "features:\n"
      "  battery: %d\n"
      "  backlight:\n"
      "    enabled: %d\n"
      "    value: %d\n"
      "    steps: %d\n",
      greeter_config->branding->background_images_dir->str,
      greeter_config->branding->logo_image->str,
      greeter_config->branding->user_image->str,
      greeter_config->greeter->debug_mode,
      greeter_config->greeter->detect_theme_errors,
      greeter_config->greeter->screensaver_timeout,
      greeter_config->greeter->theme->str,
      greeter_config->greeter->icon_theme->str,
      greeter_config->greeter->time_language->str,
      layouts->str,
      greeter_config->features->battery,
      greeter_config->features->backlight->enabled,
      greeter_config->features->backlight->value,
      greeter_config->features->backlight->steps);
  g_string_free(layouts, true);
}

static void
init_greeter_config()
{
  greeter_config = g_malloc(sizeof *greeter_config);
  init_greeter_config_branding();
  init_greeter_config_greeter();
  init_greeter_config_features();
  init_greeter_config_app();
  greeter_config->layouts = g_list_alloc();
}

void
free_greeter_config()
{
  /*free_greeter_config_branding();*/
  /*free_greeter_config_greeter();*/
  /*free_greeter_config_features();*/
  /*free_greeter_config_app();*/
  g_list_free(greeter_config->layouts);
  g_free(greeter_config);
}

static bool
yaml_get_bool(char *str)
{
  if (strcmp(str, "True") == 0)
    return true;
  else
    return false;
}
static int
yaml_get_int(char *str)
{
  return strtol(str, NULL, 10);
}

static void
load_branding(GNode *node)
{
  if (node == NULL)
    return;
  char *key = node->data;
  char *value = node->children->data;
  if (strcmp(key, "background_images_dir") == 0) {
    g_string_free(greeter_config->branding->background_images_dir, true);
    greeter_config->branding->background_images_dir = g_string_new(value);
  } else if (strcmp(key, "logo_image") == 0) {
    g_string_free(greeter_config->branding->logo_image, true);
    greeter_config->branding->logo_image = g_string_new(value);
  } else if (strcmp(key, "user_image") == 0) {
    g_string_free(greeter_config->branding->user_image, true);
    greeter_config->branding->user_image = g_string_new(value);
  }
  /*printf("  %s: %s\n", key, (char*) value);*/
  load_branding(node->next);
}
static void
load_greeter(GNode *node)
{
  if (node == NULL)
    return;
  char *key = node->data;
  char *value = node->children->data;
  if (strcmp(key, "debug_mode") == 0) {
    greeter_config->greeter->debug_mode = yaml_get_bool(value);
  } else if (strcmp(key, "detect_theme_errors") == 0) {
    greeter_config->greeter->detect_theme_errors = yaml_get_bool(value);
  } else if (strcmp(key, "screensaver_timeout") == 0) {
    greeter_config->greeter->screensaver_timeout = yaml_get_int(value);
  } else if (strcmp(key, "secure_mode") == 0) {
    greeter_config->greeter->secure_mode = yaml_get_bool(value);
  } else if (strcmp(key, "theme") == 0) {
    g_string_free(greeter_config->greeter->theme, true);
    greeter_config->greeter->theme = g_string_new(value);
  } else if (strcmp(key, "icon_theme") == 0) {
    g_string_free(greeter_config->greeter->icon_theme, true);
    greeter_config->greeter->icon_theme = g_string_new(value);
  } else if (strcmp(key, "time_language") == 0) {
    g_string_free(greeter_config->greeter->time_language, true);
    greeter_config->greeter->time_language = g_string_new(value);
  }
  /*printf("  %s: %s\n", key, (char*) value);*/
  load_greeter(node->next);
}
static void
load_layouts(GNode *node)
{
  if (node == NULL)
    return;
  char *value = node->data;
  greeter_config->layouts = g_list_append(greeter_config->layouts, value);
  /*printf("  %s\n", (char*) value);*/
  load_layouts(node->next);
}
static void
load_backlight(GNode *node)
{
  if (node == NULL)
    return;
  char *key = node->data;
  char *value = node->children->data;
  if (strcmp(key, "enabled") == 0) {
    greeter_config->features->backlight->enabled = yaml_get_bool(value);
  } else if (strcmp(key, "value") == 0) {
    greeter_config->features->backlight->value = yaml_get_int(value);
  } else if (strcmp(key, "steps") == 0) {
    greeter_config->features->backlight->steps = yaml_get_int(value);
  }
  /*printf("    %s: %s\n", key, (char*) value);*/
  load_backlight(node->next);
}
static void
load_features(GNode *node)
{
  if (node == NULL)
    return;
  char *key = node->data;
  char *value = node->children->data;
  if (strcmp(key, "battery") == 0) {
    greeter_config->features->battery = yaml_get_bool(value);
  } else if (strcmp(key, "backlight") == 0) {
    load_backlight(node->children);
  }
  /*printf("  %s: %s\n", key, (char*) value);*/
  load_features(node->next);
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

void
load_configuration()
{
  init_greeter_config();

  char *path_to_config = "/etc/lightdm/web-greeter.yml";
  FILE *fh = fopen(path_to_config, "rb");
  yaml_parser_t parser;

  if (!yaml_parser_initialize(&parser) || fh == NULL) {
    logger_error("Config was not loaded");
    return;
  }

  yaml_parser_set_input_file(&parser, fh);

  GNode *cfg = g_node_new(path_to_config);
  process_layer(&parser, cfg);

  yaml_parser_delete(&parser);
  fclose(fh);

  GNode *node = cfg->children;
  do {
    /*printf("'%s'\n", (char*) node->data);*/
    if (strcmp(node->data, "branding") == 0) {
      load_branding(node->children);
    } else if (strcmp(node->data, "greeter") == 0) {
      load_greeter(node->children);
    } else if (strcmp(node->data, "layouts") == 0) {
      load_layouts(node->children);
    } else if (strcmp(node->data, "features") == 0) {
      load_features(node->children);
    } else
      break;
    node = node->next;
  } while (node != NULL);
  g_node_destroy(cfg);

  /*logger_debug("Configuration loaded");*/
}
