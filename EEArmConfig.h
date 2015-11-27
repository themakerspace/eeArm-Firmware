/*
eeArm
by Chris Fraser <http://blog.chrosfraser.co.za>
visit http://eearm.com for more info

https://github.com/themakerspace/eeArm-Firmware
*/

#ifndef EEArmConfig_h
#define EEArmConfig_h

#include <Arduino.h>
#include <EEPROM.h>

typedef struct {
  int version;
  uint8_t mode; // AP - 0, STA - 1
  char name[32];
  char ssid[32];
  char pass[64];
} WifiConfig;

typedef struct {
  int min;
  int max;
  int start;
} Calibration;

typedef struct {
  int version;
  int speed;
  int incrementDelay;
  Calibration baseCal;
  Calibration bodyCal;
  Calibration neckCal;
  Calibration clawCal;
  int controlMin;
  int controlMax;
} ArmConfig;

class EEArmConfig {
  public:
    bool getWifiConfig(WifiConfig *conf);
    bool saveWifiConfig(WifiConfig *conf);
    bool getArmConfig(ArmConfig *conf);
    bool saveArmConfig(ArmConfig *conf);
    void setDefaultCalibration(ArmConfig *conf);
 private:
    int _version = 106;
};

#endif
