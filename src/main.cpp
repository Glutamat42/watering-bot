#include <Arduino.h>
#include "Adafruit_INA219.h"

#define moisture_enable_pin 2
#define moisture_sensor_pin 36
#define pump_pin 0
#define threshold_wet 1000

Adafruit_INA219 ina219;

void setup() {
  Serial.begin(9600);
  pinMode(moisture_sensor_pin, INPUT);
  pinMode(moisture_enable_pin, OUTPUT);
  pinMode(pump_pin, OUTPUT);

  digitalWrite(moisture_enable_pin, HIGH);
  digitalWrite(pump_pin, LOW);

  Serial.print("BV"); Serial.print("\t"); // Bus Voltage
  Serial.print("SV"); Serial.print("\t"); // Shunt Voltage
  Serial.print("LV"); Serial.print("\t"); // Load Voltage
  Serial.print("C"); Serial.print("\t");  // Current
  Serial.println("P");  // Power
}

void loop() {

  digitalWrite(moisture_enable_pin, HIGH);
  delay(100);
  int sensor_value = 0;
  for (size_t i = 0; i < 5; i++) {
    sensor_value += analogRead(moisture_sensor_pin)/5;
    delay(30);
  }
  
  digitalWrite(moisture_enable_pin, LOW);

  // Serial.print("moisture level: ");
  // Serial.println(sensor_value);

  if(sensor_value < threshold_wet+5000) {
    if (! ina219.begin()) {
      Serial.println("Failed to find INA219 chip");
      while (1) { delay(10); }
    }

    float shuntvoltage = ina219.getShuntVoltage_mV();
    float busvoltage = ina219.getBusVoltage_V();
    float current_mA = ina219.getCurrent_mA();
    float power_mW = ina219.getPower_mW();
    float loadvoltage = busvoltage + (shuntvoltage / 1000);

    Serial.print(busvoltage); Serial.print("\t"); 
    Serial.print(shuntvoltage); Serial.print("\t");
    Serial.print(loadvoltage); Serial.print("\t");
    Serial.print(current_mA); Serial.print("\t");
    Serial.println(power_mW);

    
    digitalWrite(pump_pin, HIGH);
    // pumping

  } else {
    digitalWrite(pump_pin, LOW);
    Serial.println("go to sleep");
  }

}