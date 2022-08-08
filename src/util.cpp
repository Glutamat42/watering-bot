#include "util.h"

bool water_level_ok(){
  Serial.print("water level sensor: "); Serial.println(digitalRead(WATER_LEVEL_SENSOR_PIN));
  return digitalRead(WATER_LEVEL_SENSOR_PIN) == LOW;
}