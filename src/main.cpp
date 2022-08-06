#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHA.h>
//#include "Adafruit_INA219.h"  // also in platformio.ini
#include "secrets.h"

// pins
#define I2C_SDA 21
#define I2C_SCL 22
#define MOISTURE_ENABLE_PIN 4
#define PUMP_CONTROL_PIN 13
#define MOISTURE_SENSOR_PIN 35
#define WATER_LEVEL_SENSOR_PIN 14

// moisture/water stuff
#define MOISTURE_SCALING_FACTOR   4096  // set this to the resolution of the ADC; for esp32 the default value is 4096
#define MOISTURE_MAX_VALUE        2150
#define MOISTURE_MIN_VALUE        1550
#define THRESHOLD_WET             50
#define DEEP_SLEEP_DURATION_IN_S  5
#define LOOP_INTERVAL_MIN_IN_MS   10000

// Network / HA / MQTT stuff
#define HA_BROKER_ADDR        IPAddress(192,168,2,53)
#define HA_BROKER_PORT        1883
#define HA_DEVICE_NAME        "Watering bot"
#define HA_BROKER_DATA_PREFIX "esp/watering_bot"

//Adafruit_INA219 ina219;
unsigned long lastReadAt = millis();
WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
//byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4A};
//HABinarySensor sensor_pump("pump", "power", false);
HABinarySensor sensor_water_reservoir("water_reservoir", "power", false);  // TODO: This sets this sensor to false at boot, until the first update is performed
HABinarySensor sensor_pump("pump", "power", false);
//HABinarySensor *sensor_water_reservoir = NULL;
HASensor sensor_moisture("moisture");

enum enum_pump_states {PUMP_FORCE_ON, PUMP_AUTO, PUMP_FORCE_OFF};
int pump_mode = PUMP_AUTO;
bool current_pump_state = LOW;

bool water_level_ok(){
  Serial.print("water level sensor: "); Serial.println(digitalRead(WATER_LEVEL_SENSOR_PIN));
  return digitalRead(WATER_LEVEL_SENSOR_PIN) == LOW;
}

void turn_pump_on() {
  if(pump_mode == PUMP_FORCE_OFF) {
    Serial.println("Don't turn on pump, pump_mode is forced off");
    return;
  }

  if(water_level_ok()) {
    Serial.println("enable pump");
    sensor_pump.setState(true);
    current_pump_state = HIGH;
    digitalWrite(PUMP_CONTROL_PIN, HIGH);
  } else {
    Serial.println("Don't turn on pump, water level is not ok");
  }  
}

void turn_pump_off(bool initialize=false) {
  if (!initialize){
    if(pump_mode == PUMP_FORCE_ON && water_level_ok()) {
      Serial.println("Don't turn off pump, pump_mode is forced on and water level is ok");
      return;
    }
    if (pump_mode == PUMP_FORCE_ON) {
      Serial.println("Pump is forced on, but water level is not ok");
    }
  }

  Serial.println("disable pump");
  sensor_pump.setState(false);
  current_pump_state = LOW;
  digitalWrite(PUMP_CONTROL_PIN, LOW);
}

int get_moisture_level() {
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
  return 100 - sensor_value;
  // TODO: out of range stuff (eg sensor disconnected)
}

void setup() {
  Serial.begin(9600);

  // setup GPIOs
  pinMode(MOISTURE_ENABLE_PIN, OUTPUT);
  pinMode(PUMP_CONTROL_PIN, OUTPUT);
  pinMode(MOISTURE_SENSOR_PIN, INPUT);
  pinMode(WATER_LEVEL_SENSOR_PIN, INPUT);

  digitalWrite(MOISTURE_ENABLE_PIN, HIGH);  // can be high the whole time the esp is turned on. Either sensor or pump will be active
  turn_pump_off(true);
  

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
  digitalWrite(PUMP_CONTROL_PIN, current_pump_state);
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

    
    turn_pump_on();
  } else {
    turn_pump_off();

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