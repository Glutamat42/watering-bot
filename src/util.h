#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>
#include <WiFi.h>
#include "conf.h"
#include "secrets.h"

bool water_level_ok();

bool check_wifi_connection();

#endif