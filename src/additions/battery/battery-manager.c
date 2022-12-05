#include <glib.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "additions/battery/battery-manager.h"
#include "additions/battery/battery.h"

#define PS_PATH "/sys/class/power_supply/"

G_DEFINE_TYPE(BatteryManager, battery_manager, G_TYPE_OBJECT)

static void
battery_manager_update_batteries(BatteryManager *self)
{
  GPtrArray *batteries = self->batteries;

  int sum_rate_current = 0;
  int sum_rate_power = 0;
  int sum_rate_energy = 0;
  int sum_energy_now = 0;
  int sum_energy_full = 0;
  int sum_charge_full = 0;
  int sum_charge_design = 0;

  for (guint i = 0; i < batteries->len; i++) {
    Battery *battery = batteries->pdata[i];
    battery_update(battery);

    BatteryStatistics statistics = battery_get_statistics(battery);
    sum_rate_current += statistics.current_now;
    sum_rate_power += statistics.power_now;

    if (statistics.power_now != -INT_MAX) {
      sum_rate_energy += statistics.power_now;
    } else {
      sum_rate_energy += (statistics.voltage_now * statistics.current_now) / 1e6;
    }

    sum_energy_now += statistics.energy_now;
    sum_energy_full += statistics.energy_full;
    sum_charge_full += statistics.charge_full;
    sum_charge_design += statistics.charge_full_design;
  }

  self->capacity = floor((double) sum_charge_full / sum_charge_design * 100);

  Battery *first_battery = self->batteries->pdata[0];
  g_free(self->status);
  self->status = first_battery != NULL ? g_strdup(battery_get_status(first_battery)) : g_strdup("N/A");

  for (guint i = 0; i < self->batteries->len; i++) {
    Battery *battery = self->batteries->pdata[i];
    const char *battery_status = battery_get_status(battery);
    if (g_strcmp0(battery_status, "Discharging") == 0 || g_strcmp0(battery_status, "Charging") == 0) {
      g_free(self->status);
      self->status = g_strdup(battery_status);
    }
  }

  self->ac_status = false;
}

static void
battery_manager_populate_batteries(BatteryManager *self)
{
  DIR *dir;
  struct dirent *ent;

  dir = opendir(PS_PATH);

  while ((ent = readdir(dir)) != NULL) {
    char *file_name = ent->d_name;
    if (g_strcmp0(file_name, ".") == 0 || g_strcmp0(file_name, "..") == 0)
      continue;

    g_autofree char *file_path = g_build_path(PS_PATH, file_name, NULL);

    if (strncmp("BAT", file_name, 3)) {
      Battery *battery = battery_new(file_path);
      g_ptr_array_add(self->batteries, battery);
    }
  }

  closedir(dir);

  return;
}

static void
battery_manager_class_init(BatteryManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
}

static void
battery_manager_init(BatteryManager *self)
{
  self->batteries = g_ptr_array_new();

  battery_manager_populate_batteries(self);
  battery_manager_update_batteries(self);
}

BatteryManager *
battery_manager_new()
{
  return g_object_new(ADDITION_TYPE_BATTERY_MANAGER, NULL);
}
