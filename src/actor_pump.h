#ifndef ACTOR_PUMP_H
#define ACTOR_PUMP_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include "conf.h"
#include "util.h"

enum enum_pump_states {PUMP_FORCE_ON, PUMP_AUTO, PUMP_FORCE_OFF};

class ActorPump {
    private:
        enum_pump_states pump_mode = PUMP_AUTO;
        bool current_pump_state = LOW;
        HABinarySensor* sensor_pump;
    public:
        ActorPump(HABinarySensor* sensor_pump);
        void turn_pump_on();
        void turn_pump_off(bool initialize=false);
        enum_pump_states get_pump_mode();
        bool get_current_pump_state();
        void set_pump_force_off(bool force_off, HASwitch* switch_force_pump_on);
        void set_pump_force_on(bool force_on, HASwitch* switch_force_pump_off);

};

#endif