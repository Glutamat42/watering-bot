#include "sensor_moisture.h"

MovingAverage moisture_measurement = MovingAverage();
unsigned long last_moisture_update_at = 0;


std::tuple<bool, int, bool> get_moisture_level(ActorPump* pump) {
  bool updated = false;
  int sensor_value = 0;
  if (!moisture_measurement.get_moving_average_initialized() 
  || (millis() - last_moisture_update_at) >= MOISTURE_UPDATE_INTERVAL_IN_MS) {
    last_moisture_update_at = millis();
    updated = true;

    int iterations = moisture_measurement.get_moving_average_initialized() ? 3 : 30;
    pump->temporarily_force_pump_off();  // turn pump off because pump caused voltage drop -> moisture reading were garbage
    for (size_t i = 0; i < iterations; i++) {
      sensor_value += analogRead(MOISTURE_SENSOR_PIN);
      if (i < (iterations - 1)) {
        delay(100);
      }
    }
    sensor_value /= iterations;
    pump->resume_temporarily_forced_off();
    //Serial.println(sensor_value);
    sensor_value = round(moisture_measurement.set_value(sensor_value));
  } else {
    sensor_value = round(moisture_measurement.get_value());
  }

  sensor_value -= MOISTURE_MIN_VALUE;
  sensor_value *= 100;
  sensor_value /= MOISTURE_MAX_VALUE - MOISTURE_MIN_VALUE;
  sensor_value = 100 - sensor_value;
  return {sensor_value < 105 && sensor_value > -5, sensor_value, updated};
}