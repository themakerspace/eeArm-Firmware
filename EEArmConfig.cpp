/*
eeArm
by Chris Fraser <http://blog.chrosfraser.co.za>
visit http://eearm.com for more info

https://github.com/themakerspace/eeArm-Firmware
*/

#include "EEArmConfig.h"

bool EEArmConfig::getWifiConfig(WifiConfig *conf) {
  EEPROM.get<WifiConfig>(50, *conf);

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
bool EEArmConfig::getArmConfig(ArmConfig *conf) {
  EEPROM.get<ArmConfig>(250, *conf);

  // Check if config has been versioned and set appropriate default
  if (conf->version != _version) {
    conf->speed = 10;
    conf->incrementDelay = 10;
    conf->controlMin = 0;
    conf->controlMax = 180;

    setDefaultCalibration(conf);

    saveArmConfig(conf);
  }

  return true;
}

bool EEArmConfig::saveWifiConfig(WifiConfig *conf) {
  conf->version = _version;
  EEPROM.put<WifiConfig>(50, *conf);
  return EEPROM.commit();
}

bool EEArmConfig::saveArmConfig(ArmConfig *conf) {
  conf->version = _version;
  EEPROM.put<ArmConfig>(250, *conf);
  return EEPROM.commit();
}

void EEArmConfig::setDefaultCalibration(ArmConfig *conf){
    conf->baseCal = {600, 2400, 1500};
    conf->bodyCal = {600, 2400, 1500};
    conf->neckCal = {600, 2400, 1500};
    conf->clawCal = {600, 2400, 1500};
}



