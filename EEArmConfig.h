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
  int speed;
  int incrementDelay;
} config;

class EEArmConfig {
  public:
    bool getConfig(config *conf);
    bool saveConfig(config *conf);
};

#endif
