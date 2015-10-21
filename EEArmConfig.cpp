#include "EEArmConfig.h"

bool EEArmConfig::getWifiConfig(wifiConfig *conf) {
  Serial.print("wifiConfig Size");
  Serial.println(sizeof(wifiConfig));

  EEPROM.get<wifiConfig>(50, *conf);

  // Check if config has been versioned and set appropriate default
  if (conf->version != _version) {
    String hostname = "eeArm_";
    hostname += String(ESP.getChipId(), HEX);

    strncpy(conf->name, hostname.c_str(), 32);
    strncpy(conf->ssid, "", 32);
    strncpy(conf->pass, "", 32);
    conf->mode = 0;
    conf->version = _version;

    saveWifiConfig(conf);
  }

  if (conf->mode != 1) {
    conf->mode = 0;
  }

  return true;
}
bool EEArmConfig::getArmConfig(armConfig *conf) {
  EEPROM.get<armConfig>(250, *conf);
  
  // Check if config has been versioned and set appropriate default
  if (conf->version != _version) {

      conf->version = _version;
      conf->speed = 10;
      conf->incrementDelay = 10;
      conf->baseCal = {600, 2400, 1500};
      conf->bodyCal = {600, 2400, 1500};
      conf->neckCal = {600, 2400, 1500};
      conf->clawCal = {600, 2400, 1500};
      conf->controlMin = 0;
      conf->controlMax = 180;

      saveArmConfig(conf);
    }

    return true;
  }

  bool EEArmConfig::saveWifiConfig(wifiConfig * conf) {
    EEPROM.put<wifiConfig>(50, *conf);
    return EEPROM.commit();
  }

  bool EEArmConfig::saveArmConfig(armConfig * conf) {
    EEPROM.put<armConfig>(250, *conf);
    return EEPROM.commit();
  }


