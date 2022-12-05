#ifndef ADDITION_BATTERY_MANAGER_H
#define ADDITION_BATTERY_MANAGER_H 1

#include <glib-object.h>

G_BEGIN_DECLS

#define ADDITION_TYPE_BATTERY_MANAGER battery_manager_get_type()

G_DECLARE_FINAL_TYPE(BatteryManager, battery_manager, ADDITION, BATTERY_MANAGER, GObject)

struct _BatteryManager {
  GObject parent_instance;

  int capacity;
  char *status;
  gboolean ac_status;

  GPtrArray *batteries;
};

G_END_DECLS

#endif
