#include "sensor_battery_level.h"

std::tuple<bool, int> read_battery_level(ActorPump* pump, bool disable_and_restart_wifi) {
  if (disable_and_restart_wifi) {
    WiFi.disconnect();
  }
  pump->temporarily_force_pump_off();
  
  int battery_raw_value = analogRead(BATTERY_LEVEL_PIN);
  
  pump->resume_temporarily_forced_off();
  if (disable_and_restart_wifi) {
    check_wifi_connection();
  }

  return {true, battery_raw_value};
}
