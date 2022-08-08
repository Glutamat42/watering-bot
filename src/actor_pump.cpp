#include "actor_pump.h"

ActorPump::ActorPump(HABinarySensor* sensor_pump) {
  this->sensor_pump = sensor_pump;
}

void ActorPump::turn_pump_on() {
  if(pump_mode == PUMP_FORCE_OFF) {
    Serial.println("Don't turn on pump, pump_mode is forced off");
    return;
  }

  if(water_level_ok()) {
    Serial.println("enable pump");
    this->sensor_pump->setState(true);
    current_pump_state = HIGH;
    digitalWrite(PUMP_CONTROL_PIN, HIGH);
  } else {
    Serial.println("Don't turn on pump, water level is not ok");
  }  
}

void ActorPump::turn_pump_off(bool initialize) {
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
  this->sensor_pump->setState(false);
  current_pump_state = LOW;
  digitalWrite(PUMP_CONTROL_PIN, LOW);
}

enum_pump_states ActorPump::get_pump_mode() {
  return pump_mode;
}

bool ActorPump::get_current_pump_state() {
  return current_pump_state;
}

void ActorPump::set_pump_force_off(bool force_off, HASwitch* switch_force_pump_on) {
  if (force_off) {
    pump_mode = PUMP_FORCE_OFF;
    turn_pump_off();
    Serial.println("Pump mode set to FORCE_OFF.");
  } else {
    if (switch_force_pump_on->getState()) {
      pump_mode = PUMP_FORCE_ON;
      turn_pump_on();
      Serial.println("Pump mode set to FORCE_ON. Force off disabled, but force on is enabled");
    } else {
      pump_mode = PUMP_AUTO;
      Serial.println("Pump mode set to auto. Pump state should be updated during (next) loop");
    }
  }
}

void ActorPump::set_pump_force_on(bool force_on, HASwitch* switch_force_pump_off) {
  if (force_on) {
    if (switch_force_pump_off->getState()) {
      Serial.println("Pump mode left as FORCE_OFF. Force off has higher priority than force on");
    } else {
      pump_mode = PUMP_FORCE_ON;
      turn_pump_on();
      Serial.println("Pump mode set to FORCE_ON");
    }
  } else {
    if (pump_mode == PUMP_FORCE_OFF) {
      Serial.println("Pump mode left at FORCE_OFF.");
    } else {
      pump_mode = PUMP_AUTO;
      Serial.println("Pump mode set to auto. Pump state should be updated during (next) loop");
    }
  }
}

void ActorPump::temporarily_force_pump_off() {
  digitalWrite(PUMP_CONTROL_PIN, LOW);
  delay(25);
}

void ActorPump::resume_temporarily_forced_off() {
  digitalWrite(PUMP_CONTROL_PIN, this->current_pump_state);
}