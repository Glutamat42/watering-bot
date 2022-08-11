#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHA.h>
//#include "Adafruit_INA219.h"  // also in platformio.ini
#include "secrets.h"
#include "actor_pump.h"
#include "conf.h"
#include "util.h"
#include "sensor_battery_level.h"
#include "datastructures.h"
#include "sensor_moisture.h"


//Adafruit_INA219 ina219;
unsigned long started_watering_at;
unsigned long lastReadAt = millis();
unsigned long last_read_battery_level = 0;
WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
//byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4A};
//HABinarySensor sensor_pump("pump", "power", false);
HABinarySensor sensor_water_reservoir("water_reservoir", "power", false);  // TODO: This sets this sensor to false at boot, until the first update is performed
HABinarySensor sensor_pump("pump", "power", false);
HASensor sensor_moisture("moisture");
HASensor sensor_battery("battery");
HASwitch switch_force_pump_on("force_pump_on", false);
HASwitch switch_force_pump_off("force_pump_off", false);

ActorPump pump = ActorPump(&sensor_pump);


void onSwitchForcePumpOnStateChanged(bool state, HASwitch* s) {
  pump.set_pump_force_on(state, &switch_force_pump_off);
}

void on_switch_force_pump_off_state_changed(bool state, HASwitch* s) {
  pump.set_pump_force_off(state, &switch_force_pump_on);
}


void setup() {
  Serial.begin(9600);
  Serial.println("Device is starting");

  // setup GPIOs
  pinMode(MOISTURE_ENABLE_PIN, OUTPUT);
  pinMode(PUMP_CONTROL_PIN, OUTPUT);
  pinMode(MOISTURE_SENSOR_PIN, INPUT);
  pinMode(WATER_LEVEL_SENSOR_PIN, INPUT);
  pinMode(BATTERY_LEVEL_PIN, INPUT);

  digitalWrite(MOISTURE_ENABLE_PIN, HIGH);  // can be high the whole time the esp is turned on. Either sensor or pump will be active
  pump.turn_pump_off(true);

  // WiFi and HA
  // Unique ID must be set!
  byte mac[6];
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));
  // connect
  check_wifi_connection();

  // set device's details (optional)
  device.setName(HA_DEVICE_NAME);
  device.setSoftwareVersion("0.0.2");
  // Setup HA devices here
  sensor_water_reservoir.setName("Water reservoir");
  sensor_pump.setName("Pump");
//  sensor_water_reservoir = new HABinarySensor("water_reservoir", "None", !water_level_ok());
  sensor_moisture.setUnitOfMeasurement("%");
//  sensor_moisture.setDeviceClass("None");
  sensor_moisture.setIcon("mdi:water");
  sensor_moisture.setName("Moisture");
  sensor_moisture.setAvailability(false);
  
  sensor_battery.setUnitOfMeasurement("V");
  sensor_battery.setName("Battery level");
  sensor_battery.setIcon("mdi:battery");
  sensor_battery.setAvailability(false);

  switch_force_pump_on.onStateChanged(onSwitchForcePumpOnStateChanged);
  switch_force_pump_on.setName("Force pump on");
  switch_force_pump_off.onStateChanged(on_switch_force_pump_off_state_changed);
  switch_force_pump_off.setName("Force pump off");

  // begin MQTT
  mqtt.begin(HA_BROKER_ADDR, HA_BROKER_PORT, HA_BROKER_USER, HA_BROKER_PW);
  mqtt.setDataPrefix(HA_BROKER_DATA_PREFIX);
}


void loop() {
  check_wifi_connection();
  mqtt.loop();

  if (last_read_battery_level == 0 || last_read_battery_level + BATTERY_LEVEL_UPDATE_INTERVAL_IN_MS < millis()) {
    Serial.println("update battery level");
    std::tuple<bool, float> battery_level = read_battery_level(&pump, true);
    last_read_battery_level = millis();
    Serial.print("Battery level ("); Serial.print(std::get<0>(battery_level)); Serial.print("): "); Serial.println(std::get<1>(battery_level));
    if (std::get<0>(battery_level)) {
      sensor_battery.setValue(std::get<1>(battery_level));
    }
    sensor_battery.setAvailability(std::get<0>(battery_level));

  } else {
    check_wifi_connection();
  }

  std::tuple<bool, int, bool> moisture_sensor_value = get_moisture_level(&pump);

  if (std::get<2>(moisture_sensor_value)) {
    Serial.print("moisture (");  Serial.print(std::get<0>(moisture_sensor_value)); Serial.print("): "); Serial.println(std::get<1>(moisture_sensor_value));
    if (std::get<0>(moisture_sensor_value)){
      sensor_moisture.setValue(std::get<1>(moisture_sensor_value));
    }
    sensor_moisture.setAvailability(std::get<0>(moisture_sensor_value));
  }

  bool water_available = water_level_ok();
  Serial.print("water_available: "); Serial.println(water_available);
  sensor_water_reservoir.setState(water_available);

  bool go_to_deepsleep = false;
  if(std::get<0>(moisture_sensor_value) && std::get<1>(moisture_sensor_value) < THRESHOLD_WET && water_available) { 
    Serial.println("Moisture sensor read successfull and value is below threshold")   ;
    if (pump.get_current_pump_state() == HIGH) {
      Serial.println("Pump already HIGH");
      if (started_watering_at + MAX_WATERING_DURATION_IN_MS < millis()) {
        Serial.println("Pump time limit reached");
        go_to_deepsleep = true;
      }
    } else {
      Serial.println("Pump is LOW, turn on");
      started_watering_at = millis();
      pump.turn_pump_on();
    }
  } else {
    go_to_deepsleep = true;
  }

  if (go_to_deepsleep) {
    pump.turn_pump_off();

    mqtt.disconnect();
    digitalWrite(MOISTURE_ENABLE_PIN, LOW);
    Serial.println("go to sleep");
    esp_sleep_enable_timer_wakeup( (uint64_t) DEEP_SLEEP_DURATION_IN_S * 1000 * 1000);
    Serial.flush(); 
    esp_deep_sleep_start();
  }

  // delay
  unsigned long sinceLastRead = millis() - lastReadAt;
  if (sinceLastRead < LOOP_INTERVAL_MIN_IN_MS) {
    delay(LOOP_INTERVAL_MIN_IN_MS - sinceLastRead);
  }
  lastReadAt = millis();
}