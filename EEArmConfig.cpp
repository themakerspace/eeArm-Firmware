#include "EEArmConfig.h"

bool EEArmConfig::getConfig(config *conf) {
  
  Serial.print("conf Size");
  Serial.println(sizeof(config));
  
  EEPROM.get<config>(100, *conf);

  if ((uint32_t)(conf->name)[0] == 255) {
    strncpy(conf->name, "Gwa Test eeArm", 32);
  }

  if ((uint32_t)(conf->ssid)[0] == 255) {
    strncpy(conf->ssid, "", 32);
    conf->mode = 0;
  }
  
  if ((uint32_t)(conf->pass)[0] == 255) {
    strncpy(conf->pass, "", 32);
  }

  if (conf->mode != 1) {
    conf->mode = 0;
  }
  
  Serial.print("mode: ");
  Serial.println(conf->mode);
  Serial.print("name: ");
  Serial.println(conf->name);
  Serial.print("ssid: ");
  Serial.println(conf->ssid);
  Serial.print("pass: ");
  Serial.println(conf->pass);
  Serial.print("maxIncrement: ");
  Serial.println(conf->speed);
  Serial.print("incrementDelay: ");
  Serial.println(conf->incrementDelay);

  return true;
}

bool EEArmConfig::saveConfig(config *conf) {
  EEPROM.put<config>(100, *conf);
  return EEPROM.commit();
}


