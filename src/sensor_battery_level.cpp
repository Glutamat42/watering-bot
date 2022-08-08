#include "sensor_battery_level.h"

std::tuple<bool, float> read_battery_level(ActorPump* pump, bool disable_and_restart_wifi) {
  disable_and_restart_wifi=false; // wifi disconnect is causing too many problems

  if (disable_and_restart_wifi) {
    WiFi.disconnect();
  }
  pump->temporarily_force_pump_off();
  
  float battery_raw_value = analogRead(BATTERY_LEVEL_PIN);
  
  pump->resume_temporarily_forced_off();
  if (disable_and_restart_wifi) {
    check_wifi_connection();
  }

  Serial.print("raw value: "); Serial.println(battery_raw_value);
  battery_raw_value = battery_raw_value * 2 / 4095 * 3.3;

  if (battery_raw_value == 0 || battery_raw_value > 4.5) {
    return {false, battery_raw_value};
  }
  return {true, battery_raw_value};

  /** sensor reading | voltage
   * 2303 3,91V
   * 
   */
}
