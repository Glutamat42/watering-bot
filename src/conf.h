#ifndef CONF_H
#define CONF_H

// pins
#define I2C_SDA 21
#define I2C_SCL 22
#define MOISTURE_ENABLE_PIN 4
#define PUMP_CONTROL_PIN 13                 // TODO 13, disable: pin 19
#define MOISTURE_SENSOR_PIN 35
#define WATER_LEVEL_SENSOR_PIN 14
#define BATTERY_LEVEL_PIN 36

// moisture/water stuff
#define MOISTURE_SCALING_FACTOR             4096  // set this to the resolution of the ADC; for esp32 the default value is 4096
#define MOISTURE_MAX_VALUE                  2150
#define MOISTURE_MIN_VALUE                  1550
#define THRESHOLD_WET                       24          // TODO: 22 or something like that
#define DEEP_SLEEP_DURATION_IN_S            60 * 60      // 60 * 60
#define LOOP_INTERVAL_MIN_IN_MS             1000
#define MOISTURE_UPDATE_INTERVAL_IN_MS      10000
#define BATTERY_LEVEL_UPDATE_INTERVAL_IN_MS 10000
#define MAX_WATERING_DURATION_IN_MS         3 * 60 * 1000   // maybe 3 * 60 * 1000

// Network / HA / MQTT stuff
#define HA_BROKER_ADDR        IPAddress(192,168,2,53)
#define HA_BROKER_PORT        1883
#define HA_DEVICE_NAME        "Watering bot"
#define HA_BROKER_DATA_PREFIX "esp/watering_bot"

#endif