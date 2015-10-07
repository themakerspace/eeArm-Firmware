/*
eeArm
by Chris Fraser <http://blog.chrosfraser.co.za>

https://github.com/themakerspace/eeArm-Firmware
*/
#ifndef EEARMCONFIG_h
#define EEARMCONFIG_h

#include <Arduino.h>
#include <EEPROM.h>

typedef struct {
  uint8_t mode; // AP - 0, STA - 1
  char name[32];
  char ssid[32];
  char pass[64];
} wifiConfig;

typedef struct {
  int min;
  int max;
  int start;
} calibration;

typedef struct {
  int speed;
  int incrementDelay;
  calibration baseCal;
  calibration bodyCal;
  calibration neckCal;
  calibration clawCal;
} armConfig;

class EEArmConfig {
  public:
    bool getWifiConfig(wifiConfig *conf);
    bool saveWifiConfig(wifiConfig *conf);
    bool getArmConfig(armConfig *conf);
    bool saveArmConfig(armConfig *conf);
};

#endif
