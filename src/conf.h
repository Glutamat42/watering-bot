#ifndef CONF_H
#define CONF_H

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
#define LOOP_INTERVAL_MIN_IN_MS   500
#define MOISTURE_UPDATE_INTERVAL_IN_MS 10000

// Network / HA / MQTT stuff
#define HA_BROKER_ADDR        IPAddress(192,168,2,53)
#define HA_BROKER_PORT        1883
#define HA_DEVICE_NAME        "Watering bot"
#define HA_BROKER_DATA_PREFIX "esp/watering_bot"

#endif