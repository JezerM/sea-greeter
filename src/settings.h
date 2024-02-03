#ifndef SETTINGS_H
#define SETTINGS_H 1

#include <glib.h>
#include <stdbool.h>

typedef struct greeter_config_branding_st {
  /**
   * Path to directory that contains background images for use by themes
   */
  char *background_images_dir;
  /**
   * Path to logo image for use by greeter themes
   */
  char *logo_image;
  /**
   * Default user image/avatar. This is used by themes when user has no .face image
   */
  char *user_image;
} GreeterConfigBranding;

typedef struct greeter_config_greeter_st {
  /**
   * Enable debug mode for the greeter as well as greeter themes
   */
  bool debug_mode;
  /**
   * Provide an option to load a fallback theme when theme errors are detected
   */
  bool detect_theme_errors;
  /**
   * Blank the screen after this many seconds of inactivity
   */
  int screensaver_timeout;
  /**
   * Don't allow themes to make remote http requests
   */
  bool secure_mode;
  /**
   * Greeter theme to use
   */
  char *theme;
  /**
   * Icon/cursor theme to use, located in /usr/share/icons/
   */
  char *icon_theme;
  /**
   * Language to use when displaying the date or time
   */
  char *time_language;
} GreeterConfigGreeter;

typedef struct greeter_config_features_backlight_st {
  /**
   * Enable greeter and themes to control display backlight
   */
  bool enabled;
  /**
   * The amount to icrease/decrease brightness by greeter
   */
  int steps;
  /**
   * How many steps are needed to do the change
   */
  int value;
} GreeterConfigFeaturesBacklight;

typedef struct greeter_config_features_st {
  /**
   * Enable greeter and themes to get battery status
   */
  bool battery;
  /**
   * Backlight options
   */
  GreeterConfigFeaturesBacklight *backlight;
} GreeterConfigFeatures;

typedef struct greeter_config_app_st {
  bool fullscreen;
  bool frame;
  bool debug_mode;
  char *theme_dir;
} GreeterConfigApp;

typedef struct greeter_config_theme_st {
  char *primary_html;
  char *secondary_html;
} GreeterConfigTheme;

/**
 * Greeter configuration
 */
typedef struct greeter_config_st {
  GreeterConfigBranding *branding;
  GreeterConfigGreeter *greeter;
  GPtrArray *layouts;
  GreeterConfigFeatures *features;
  GreeterConfigApp *app;
  GreeterConfigTheme *theme;
} GreeterConfig;

extern GreeterConfig *greeter_config;

void print_greeter_config(void);
void free_greeter_config(void);
void load_configuration(void);
#endif
