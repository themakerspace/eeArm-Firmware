#include "EEArmConfig.h"

bool EEArmConfig::getWifiConfig(wifiConfig *conf) {
  
  //Serial.print("conf Size");
  //Serial.println(sizeof(config));
  
  EEPROM.get<wifiConfig>(100, *conf);
  
  // Defaults
  if ((uint32_t)(conf->name)[0] == 255) {
    
    String hostname = "eeArm_";
    hostname += String(ESP.getChipId(), HEX);
    
    strncpy(conf->name, hostname.c_str(), 32);
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
  
//  Serial.print("mode: ");
//  Serial.println(conf->mode);
//  Serial.print("name: ");
//  Serial.println(conf->name);
//  Serial.print("ssid: ");
//  Serial.println(conf->ssid);
//  Serial.print("pass: ");
//  Serial.println(conf->pass);

  return true;
}
bool EEArmConfig::getArmConfig(armConfig *conf) {
  
  //Serial.print("conf Size");
  //Serial.println(sizeof(config));
  
  EEPROM.get<armConfig>(100, *conf);

  return true;
}

bool EEArmConfig::saveWifiConfig(wifiConfig *conf) {
  EEPROM.put<wifiConfig>(100, *conf);
  return EEPROM.commit();
}

bool EEArmConfig::saveArmConfig(armConfig *conf) {
  EEPROM.put<armConfig>(250, *conf);
  return EEPROM.commit();
}


