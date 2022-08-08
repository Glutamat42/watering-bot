#ifndef SENSOR_BATTERY_LEVEL_H
#define SENSOR_BATTERY_LEVEL_H

#include <Arduino.h>
#include "actor_pump.h"


/**
 * @brief 
 * 
 * @param pump 
 * @param disable_and_restart_wifi 
 * @return std::tuple<bool, int> bool: false if reading was (obviously) not successful; int: sensor value
 */
std::tuple<bool, int> read_battery_level(ActorPump* pump, bool disable_and_restart_wifi=false);

#endif