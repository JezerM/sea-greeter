#ifndef ADDITION_BATTERY_H
#define ADDITION_BATTERY_H 1

#include <glib-object.h>

G_BEGIN_DECLS

#define ADDITION_TYPE_BATTERY battery_get_type()

G_DECLARE_FINAL_TYPE(Battery, battery, ADDITION, BATTERY, GObject)

struct _Battery {
  GObject parent_instance;
};

typedef struct {
  int current_now;
  int voltage_now;
  int power_now;
  int charge_full;
  int charge_full_design;
  int energy_now;
  int energy_full;
  int energy_percentage;
} BatteryStatistics;

G_END_DECLS

const char *
battery_get_name(Battery *battery);
const char *
battery_get_status(Battery *battery);
gboolean
battery_get_ac_status(Battery *battery);
int
battery_get_level(Battery *battery);
int
battery_get_capacity(Battery *battery);
const char *
battery_get_time(Battery *battery);
int
battery_get_watt(Battery *battery);
BatteryStatistics
battery_get_statistics(Battery *battery);
void
battery_update(Battery *self);

Battery *
battery_new(const char *path);

#endif
