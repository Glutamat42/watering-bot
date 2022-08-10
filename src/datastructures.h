#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include "conf.h"
#include "util.h"

class RingArray {
    private:
        int index = 0;
        int array_length;
        float* array;
        bool all_indexes_used = false;
    public:
        RingArray(int array_length=3);
        float set_value(float value);
        float* get_array();
        float get_avg();

};

class MovingAverage {
    private:
        float value;
        float factor;
        bool moving_average_initialized=false;
    public:
        MovingAverage(float factor=0.8);
        float set_value(float value);
        float get_value();
        bool get_moving_average_initialized();

};


#endif