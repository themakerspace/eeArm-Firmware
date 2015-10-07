/*
eeArm
by Chris Fraser <http://blog.chrosfraser.co.za>

https://github.com/themakerspace/eeArm-Firmware
*/
#ifndef EEARM_h
#define EEARM_h

#include <Servo.h>
#include <EEPROM.h>
#include "EEArmConfig.h"

typedef struct {
  int base;
  int body;
  int neck;
  int claw;
} armPosition;

typedef struct {
  armPosition pos;
  int steps;
  int delay;
} armStep;

class EEArm {
  public:
    armConfig config;
    armPosition begin(uint8_t basePin = 13, uint8_t bodyPin = 12, uint8_t neckPin = 14, uint8_t clawPin = 16);
    void detach();
    armPosition attach(uint8_t basePin = 13, uint8_t bodyPin = 12, uint8_t neckPin = 14, uint8_t clawPin = 16);
    armPosition getPosition();
    armPosition moveTo(armPosition* position);
    int addStep(armStep step);
    int popStep();
    bool clearSteps();
    bool saveSteps();
    bool loadSteps();
    armPosition goToStart();
    bool play();
    bool loop(int delayBetween);
    armPosition pause(int delay);
    armPosition stop();
    bool printSteps();

  private:
    Servo base;
    Servo body;
    Servo neck;
    Servo claw;
    armStep _armSteps[50];
    int _writeIndex = 0;
    void moveServoIncrement(armPosition* previous, armPosition* current, int i, int increments);
    int interpolate (int previous, int current, int i, int increments);
    int getIncrements(armPosition* previous, armPosition* current);
    void move(armPosition *previous, armPosition *current, int steps = 0);
    bool printStep(armStep *currentStep);
};

#endif
