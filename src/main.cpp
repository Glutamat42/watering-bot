#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHA.h>
//#include "Adafruit_INA219.h"  // also in platformio.ini
#include "secrets.h"
#include "actor_pump.h"
#include "conf.h"
#include "util.h"



//Adafruit_INA219 ina219;
unsigned long lastReadAt = millis();
unsigned long last_moisture_update_at = millis() - MOISTURE_UPDATE_INTERVAL_IN_MS;
WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
//byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4A};
//HABinarySensor sensor_pump("pump", "power", false);
HABinarySensor sensor_water_reservoir("water_reservoir", "power", false);  // TODO: This sets this sensor to false at boot, until the first update is performed
HABinarySensor sensor_pump("pump", "power", false);
//HABinarySensor *sensor_water_reservoir = NULL;
HASensor sensor_moisture("moisture");
HASwitch switch_force_pump_on("force_pump_on", false);
HASwitch switch_force_pump_off("force_pump_off", false);

ActorPump pump = ActorPump(&sensor_pump);


void onSwitchForcePumpOnStateChanged(bool state, HASwitch* s) {
  pump.set_pump_force_on(state, &switch_force_pump_off);
}

void on_switch_force_pump_off_state_changed(bool state, HASwitch* s) {
  pump.set_pump_force_off(state, &switch_force_pump_on);
}


int last_moisture_value = -9999;

int get_moisture_level() {
  if (last_moisture_value == -9999 || (millis() - last_moisture_update_at) >= MOISTURE_UPDATE_INTERVAL_IN_MS) {
    last_moisture_update_at = millis();

    int sensor_value = 0;
    int iteraions = 2;
    for (size_t i = 0; i < iteraions; i++) {
      sensor_value += analogRead(MOISTURE_SENSOR_PIN)/iteraions;
      delay(25);
    }
    Serial.println(sensor_value);
    //return sensor_value * 100 / MOISTURE_SCALING_FACTOR;
    sensor_value -= MOISTURE_MIN_VALUE;
    sensor_value *= 100;
    sensor_value /= MOISTURE_MAX_VALUE - MOISTURE_MIN_VALUE;
    //sensor_value = (sensor_value > 105 || sensor_value < -5) ? (-10) : sensor_value;
    last_moisture_value = 100 - sensor_value;
    // TODO: out of range stuff (eg sensor disconnected)
  } 
  return last_moisture_value;
}

void setup() {
  Serial.begin(9600);

  // setup GPIOs
  pinMode(MOISTURE_ENABLE_PIN, OUTPUT);
  pinMode(PUMP_CONTROL_PIN, OUTPUT);
  pinMode(MOISTURE_SENSOR_PIN, INPUT);
  pinMode(WATER_LEVEL_SENSOR_PIN, INPUT);

  digitalWrite(MOISTURE_ENABLE_PIN, HIGH);  // can be high the whole time the esp is turned on. Either sensor or pump will be active
  pump.turn_pump_off(true);

  

  // WiFi and HA
  // Unique ID must be set!
  byte mac[6];
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));
  // connect
  WiFi.begin(WIFI_SSID, WIFI_PW);
  Serial.println(WIFI_SSID);
  Serial.println(WIFI_PW);

  for (size_t i = 0; i < 10; i++) {
    int wifi_status = WiFi.status();
    if (wifi_status != WL_CONNECTED) {
      Serial.print("WiFi not connected: ");
      Serial.println(wifi_status);
      delay(250);
    } else {
      Serial.println("WiFi connected");
      Serial.println(WiFi.localIP());
      break;
    }
  }
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

  switch_force_pump_on.onStateChanged(onSwitchForcePumpOnStateChanged);
  switch_force_pump_on.setName("Force pump on");
  switch_force_pump_off.onStateChanged(on_switch_force_pump_off_state_changed);
  switch_force_pump_off.setName("Force pump off");

  // begin MQTT
  mqtt.begin(HA_BROKER_ADDR, HA_BROKER_PORT, HA_BROKER_USER, HA_BROKER_PW);
  mqtt.setDataPrefix(HA_BROKER_DATA_PREFIX);


/**
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  */
}


void loop() {
  int wifi_status = WiFi.status();
  if (wifi_status != WL_CONNECTED) {
    Serial.print("wifi connection problem, status: "); Serial.println(wifi_status);
    WiFi.begin(WIFI_SSID, WIFI_PW);

    for (size_t i = 0; i < 10; i++) {
      wifi_status = WiFi.status();
      if (wifi_status != WL_CONNECTED) {
        delay(250);
      } else {
        Serial.println("WiFi reconnected");
        Serial.println(WiFi.localIP());
        break;
      }
    }
  }



  mqtt.loop();


  digitalWrite(PUMP_CONTROL_PIN, LOW);  // turn pump off because pump caused voltage drop -> moisture reading were garbage
  delay(100);
  int sensor_value = get_moisture_level();
  digitalWrite(PUMP_CONTROL_PIN, pump.get_current_pump_state());
  Serial.print("moisture level: ");
  Serial.println(sensor_value);
  sensor_moisture.setValue(sensor_value);

  bool water_available = water_level_ok();
  Serial.print("water_available: "); Serial.println(water_available);
  sensor_water_reservoir.setState(water_available);
  // TODO: log water_available

  if(sensor_value < THRESHOLD_WET && water_available) {
    /**
    float shuntvoltage = ina219.getShuntVoltage_mV();
    float busvoltage = ina219.getBusVoltage_V();
    float current_mA = ina219.getCurrent_mA();
    float power_mW = ina219.getPower_mW();
    float loadvoltage = busvoltage + (shuntvoltage / 1000);
    */

    
    pump.turn_pump_on();
  } else {
    pump.turn_pump_off();

    /**
    mqtt.disconnect();
    digitalWrite(MOISTURE_ENABLE_PIN, LOW);

    Serial.println("go to sleep");
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION_IN_S * 1000 * 1000);
    Serial.flush(); 
    esp_deep_sleep_start();
    */
  }

  // delay
  unsigned long sinceLastRead = millis() - lastReadAt;
  if (sinceLastRead < LOOP_INTERVAL_MIN_IN_MS) {
    delay(LOOP_INTERVAL_MIN_IN_MS - sinceLastRead);
  }
  lastReadAt = millis();
}