#ifndef SENSOR_VALUE_H
#define SENSOR_VALUE_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include "conf.h"
#include "util.h"


class SensorValue {
    private:
    public:
        SensorValue(HABinarySensor* sensor_pump);
};

#endif