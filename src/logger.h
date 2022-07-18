#ifndef LOGGER_H
#define LOGGER_H 1
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <glib.h>
#include <locale.h>

#define logger_raw(type, message, ...) {\
  GDateTime *now = g_date_time_new_now_local();\
  char *timestamp = g_date_time_format(now, "%Y-%m-%d %H:%M:%S");\
  char *str = g_strdup_printf(message, ##__VA_ARGS__);\
  fprintf(stderr, "%s [ %s ] %s %d: %s\n",\
      timestamp, type, g_path_get_basename(__FILE__), __LINE__, str);\
  g_free(str);\
}

#define logger_debug(message, ...) { logger_raw("DEBUG", message, ##__VA_ARGS__) }
#define logger_error(message, ...) { logger_raw("ERROR", message, ##__VA_ARGS__) }
#define logger_warn(message, ...) { logger_raw("WARN", message, ##__VA_ARGS__) }

#endif
