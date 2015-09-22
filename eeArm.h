/* 
eeArm
by Chris Fraser <http://blog.chrosfraser.co.za>

https://github.com/themakerspace/eeArm-Firmware
*/
#ifndef EEARM_h
#define EEARM_h

#include <Servo.h>
#include <EEPROM.h>

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

typedef struct {
  int min;
  int max;
} calibration;

class EEArmClass {
  public:
    armStep _armSteps[50];
    int _writeIndex = 0;
    armPosition begin(uint8_t basePin, uint8_t bodyPin, uint8_t neckPin, uint8_t clawPin);
    armPosition getPosition();
    armPosition moveTo(armPosition position);
    bool addStep(armStep step);
    bool popStep();
    bool clearSteps();
    bool saveSteps();    
    bool loadSteps();
    armPosition goToStart();
    armPosition play();
    armPosition loop(int delayBetween);
    armPosition pause(int delay);
    armPosition stop();
    bool printSteps();

  private:
    Servo base;
    Servo body;
    Servo neck;
    Servo claw;
    calibration _servoLimits[4];  
    int _maxIncrement = 1;
    int _incrementDelay = 10;
    void moveServoIncrement(armPosition previous, armPosition current, int i, int increments);
    int interpolate (int previous, int current, int i, int increments);
    int getIncrements(armPosition previous, armPosition current);
    void move(armPosition previous, armPosition current, int steps = 0);
    bool printStep(armStep *currentStep);
};

extern EEArmClass EEArm;

#endif
