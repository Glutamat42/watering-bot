#include "util.h"

bool water_level_ok(){
  //Serial.print("water level sensor: "); Serial.println(digitalRead(WATER_LEVEL_SENSOR_PIN));
  return digitalRead(WATER_LEVEL_SENSOR_PIN) == HIGH;
}

bool check_wifi_connection() {
  int wifi_status = WiFi.status();
  if (wifi_status != WL_CONNECTED) {
    Serial.print("wifi no connected, status: "); Serial.println(wifi_status);
    WiFi.begin(WIFI_SSID, WIFI_PW);

    for (size_t i = 0; i < 15; i++) {
      wifi_status = WiFi.status();
      if (wifi_status != WL_CONNECTED) {
        delay(250);
      } else {
        Serial.println("WiFi reconnected");
        Serial.println(WiFi.localIP());
        //delay(250);
        return true;
      }
    }
  } else {
    return true;
  }
  Serial.println("failed to reconnect to wifi");
  return false;
}
