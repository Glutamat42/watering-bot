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

  //Serial.print("raw value: "); Serial.println(battery_raw_value);
  battery_raw_value = battery_raw_value / 4095 * 3.3 * 2.17;
  battery_raw_value = round(battery_raw_value * 10.0) / 10.0;

  if (battery_raw_value < 2 || battery_raw_value > 5) {
    return {false, battery_raw_value};
  }
  return {true, battery_raw_value};

  /** sensor reading | voltage
   * 2303 3,91V
   * 
   * 
   * 2130/2200 3,85V
   * 2100 3,67V
   * 1920 3,64V
   * 1900/1950 3,45V
   * 1635 3,05V (cut off, Spannung ungenau)
   * 
   * 
   * charging
   * 2255 3,94
   * 2270 3,83V
   * 
   */
}
