#include <glib-object.h>
#include <math.h>
#include <stdio.h>

#include "additions/battery/battery.h"

typedef struct {
  char *name;
  char *location;

  gboolean ac_status;
  char *status;
  int level;
  int capacity;

  char *time;
  int watt;

  BatteryStatistics statistics;
} BatteryPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(Battery, battery, G_TYPE_OBJECT)

typedef enum {
  PROP_NAME = 1,
  PROP_STATUS,
  PROP_LEVEL,
  PROP_CAPACITY,
  PROP_LOCATION,
  N_PROPERTIES,
} BatteryProperty;

static GParamSpec *props[N_PROPERTIES];

const char *
battery_get_name(Battery *battery)
{
  BatteryPrivate *priv = battery_get_instance_private(battery);
  return priv->name;
}
const char *
battery_get_status(Battery *battery)
{
  BatteryPrivate *priv = battery_get_instance_private(battery);
  return priv->status;
}
gboolean
battery_get_ac_status(Battery *battery)
{
  BatteryPrivate *priv = battery_get_instance_private(battery);
  return priv->ac_status;
}
int
battery_get_level(Battery *battery)
{
  BatteryPrivate *priv = battery_get_instance_private(battery);
  return priv->level;
}
int
battery_get_capacity(Battery *battery)
{
  BatteryPrivate *priv = battery_get_instance_private(battery);
  return priv->capacity;
}
const char *
battery_get_time(Battery *battery)
{
  BatteryPrivate *priv = battery_get_instance_private(battery);
  return priv->time;
}
int
battery_get_watt(Battery *battery)
{
  BatteryPrivate *priv = battery_get_instance_private(battery);
  return priv->watt;
}
BatteryStatistics
battery_get_statistics(Battery *battery)
{
  BatteryPrivate *priv = battery_get_instance_private(battery);
  return priv->statistics;
}

static char *
read_first_line(const char *path)
{
  return NULL;
}
static gint64
str_to_int(char *str)
{
  char *endptr = NULL;
  gint64 value = g_ascii_strtoll(str, &endptr, 10);

  if (g_strcmp0(str, endptr) == 0) {
    return -INT_MAX;
  }

  g_free(str);
  return value;
}

void
battery_update(Battery *self)
{
  BatteryPrivate *priv = battery_get_instance_private(self);

  g_autofree char *current_now_path = g_build_path("/", priv->location, "current_now", NULL);
  g_autofree char *voltage_now_path = g_build_path("/", priv->location, "voltage_now", NULL);
  g_autofree char *power_now_path = g_build_path("/", priv->location, "power_now", NULL);
  g_autofree char *charge_now_path = g_build_path("/", priv->location, "charge_now", NULL);
  g_autofree char *charge_full_path = g_build_path("/", priv->location, "charge_full", NULL);
  g_autofree char *charge_full_design_path = g_build_path("/", priv->location, "charge_full_design", NULL);
  g_autofree char *energy_now_path = g_build_path("/", priv->location, "energy_now", NULL);
  g_autofree char *energy_full_path = g_build_path("/", priv->location, "energy_full", NULL);
  g_autofree char *capacity_path = g_build_path("/", priv->location, "capacity", NULL);
  g_autofree char *status_path = g_build_path("/", priv->location, "status", NULL);

  int current_now = str_to_int(read_first_line(current_now_path));
  int voltage_now = str_to_int(read_first_line(voltage_now_path));
  int power_now = str_to_int(read_first_line(power_now_path));
  int charge_full = str_to_int(read_first_line(charge_full_path));
  int charge_full_design = str_to_int(read_first_line(charge_full_design_path));

  int energy_now = str_to_int(read_first_line(energy_now_path));
  if (energy_now == -INT_MAX) {
    energy_now = str_to_int(read_first_line(charge_now_path));
  }

  int energy_full = str_to_int(read_first_line(energy_full_path));
  if (energy_now == -INT_MAX) {
    energy_full = charge_full;
  }

  int energy_percentage = str_to_int(read_first_line(capacity_path));
  if (energy_percentage == -INT_MAX && charge_full_design != 0) {
    energy_percentage = (charge_full / charge_full_design) * 100;
  }

  char *status = read_first_line(status_path);

  priv->status = status;

  if (charge_full_design == 0) {
    priv->capacity = 0;
  } else {
    priv->capacity = floor((float) charge_full / (float) charge_full_design * 100);
  }

  priv->level = energy_percentage != -INT_MAX ? energy_percentage : priv->level;

  priv->statistics.current_now = current_now;
  priv->statistics.voltage_now = voltage_now;
  priv->statistics.power_now = power_now;
  priv->statistics.charge_full = charge_full;
  priv->statistics.charge_full_design = charge_full_design;
  priv->statistics.energy_now = energy_now;
  priv->statistics.energy_full = energy_full;
  priv->statistics.energy_percentage = energy_percentage;

  printf("UPDATE\n");
}

static void
battery_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  (void) pspec;
  Battery *self = ADDITION_BATTERY(object);
  BatteryPrivate *priv = battery_get_instance_private(self);

  switch ((BatteryProperty) prop_id) {
    case PROP_NAME:
      g_value_set_string(value, priv->name);
      break;
    case PROP_STATUS:
      g_value_set_string(value, priv->status);
      break;
    case PROP_LEVEL:
      g_value_set_int(value, priv->level);
      break;
    case PROP_CAPACITY:
      g_value_set_int(value, priv->capacity);
      break;
    case PROP_LOCATION:
      g_value_set_string(value, priv->location);
      break;
    default:
      break;
  }
}
static void
battery_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  (void) pspec;
  Battery *self = ADDITION_BATTERY(object);
  BatteryPrivate *priv = battery_get_instance_private(self);

  switch ((BatteryProperty) prop_id) {
    case PROP_NAME:
      priv->name = g_value_dup_string(value);
      break;
    case PROP_STATUS:
      priv->status = g_value_dup_string(value);
      break;
    case PROP_LEVEL:
      priv->level = g_value_get_int(value);
      break;
    case PROP_CAPACITY:
      priv->capacity = g_value_get_int(value);
      break;
    case PROP_LOCATION:
      priv->location = g_value_dup_string(value);
      break;
    default:
      break;
  }
}

static void
battery_constructed(GObject *object)
{
  G_OBJECT_CLASS(battery_parent_class)->constructed(object);
  Battery *self = ADDITION_BATTERY(object);

  battery_update(self);
}

static void
battery_class_init(BatteryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->constructed = battery_constructed;
  object_class->set_property = battery_set_property;
  object_class->get_property = battery_get_property;

  props[PROP_NAME] = g_param_spec_string("name", NULL, NULL, NULL, G_PARAM_READABLE);

  props[PROP_STATUS] = g_param_spec_string("status", NULL, NULL, NULL, G_PARAM_READABLE);

  props[PROP_LEVEL] = g_param_spec_int("level", NULL, NULL, 0, INT_MAX, 0, G_PARAM_READABLE);

  props[PROP_CAPACITY] = g_param_spec_int("capacity", NULL, NULL, 0, INT_MAX, 0, G_PARAM_READABLE);

  props[PROP_LOCATION] = g_param_spec_string("location", NULL, NULL, NULL, G_PARAM_READABLE);

  g_object_class_install_properties(object_class, N_PROPERTIES, props);
}

static void
battery_init(Battery *self)
{
  BatteryPrivate *priv = battery_get_instance_private(self);
  priv->location = NULL;
  priv->name = NULL;
}

Battery *
battery_new(const char *path)
{
  Battery *object = g_object_new(ADDITION_TYPE_BATTERY, "location", path, NULL);

  return object;
}
