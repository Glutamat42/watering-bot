#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHA.h>
#include "conf.h"
#include "secrets.h"
#include "datastructures.h"
#include "actor_pump.h"


std::tuple<bool, int, bool> get_moisture_level(ActorPump* pump);