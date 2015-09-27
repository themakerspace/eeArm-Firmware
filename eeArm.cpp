/*
eeArm
by Chris Fraser <http://blog.chrosfraser.co.za>

https://github.com/themakerspace/eeArm-Firmware
*/

#include "EEArm.h"
//
#define _EEARM_DEBUG   // Enables general logging
#define _MOVEMENT_DEBUG   // Enables logging movement
#define _EEPROM_DEBUG   // Enables logging of eeprom info

// ----------------------------------------------------
// Public
// ----------------------------------------------------
armPosition EEArm::begin(uint8_t basePin, uint8_t bodyPin, uint8_t neckPin, uint8_t clawPin) {
  base.attach(basePin);
  body.attach(bodyPin);
  neck.attach(neckPin);
  claw.attach(clawPin);
  loadSteps();

  return getPosition();
}

armPosition EEArm::getPosition() {
  armPosition pos = {
    base.read() + 1,
    body.read() + 1,
    neck.read() + 1,
    claw.read() + 1
  };

  return pos;
}

armPosition EEArm::moveTo(armPosition position) {
  armPosition current = getPosition();
  move(current, position);
  return getPosition();
}

bool EEArm::addStep(armStep step) {
  _armSteps[_writeIndex++] = step;
}

bool EEArm::popStep() {
  _writeIndex--;
}

bool EEArm::clearSteps() {
  _writeIndex = 0;

  EEPROM.put<int>(500, _writeIndex);
  return EEPROM.commit();
}

armPosition EEArm::goToStart() {
  armStep step = _armSteps[0];
  armPosition current = getPosition();
  move(current, step.pos);

  return getPosition();
}

armPosition EEArm::play() {
  // previousStep initially set to start position
  armStep previousStep = {getPosition(), 0, 0};
  armStep currentStep;

  // Loop through steps
  for (int i = 0; i < _writeIndex; i++) {
    currentStep = _armSteps[i];

    move(previousStep.pos, currentStep.pos, currentStep.steps);

    delay(currentStep.delay);
    previousStep = currentStep;
  }

}

armPosition EEArm::loop(int delayBetween) {

}

armPosition EEArm::pause(int delay) {

}

armPosition EEArm::stop() {

}

// ----------------------------------------------------
// Private
// ----------------------------------------------------
void EEArm::move(armPosition previous, armPosition current, int steps) {

  int increments = 1;
#ifdef _MOVEMENT_DEBUG
  Serial.print("From: ");
  Serial.print(previous.base);
  Serial.print(", ");
  Serial.print(previous.body);
  Serial.print(", ");
  Serial.print(previous.neck);
  Serial.print(", ");
  Serial.print(previous.claw);
  Serial.print(" ===> ");
  Serial.print(current.base);
  Serial.print(", ");
  Serial.print(current.body);
  Serial.print(", ");
  Serial.print(current.neck);
  Serial.print(", ");
  Serial.print(current.claw);
  Serial.println();
#endif
  // If no steps are defined
  if (steps == 0) {
    increments = getIncrements(previous, current);
  }

  // Loop through the increments
  for (int i = 0; i < increments; i++) {
    moveServoIncrement(previous, current, i, increments);
    delay(_incrementDelay);
  }
}

void EEArm::moveServoIncrement(armPosition previous, armPosition current, int i, int increments) {
  int baseNext = interpolate(previous.base, current.base, i, increments);
  int bodyNext = interpolate(previous.body, current.body, i, increments);
  int neckNext = interpolate(previous.neck, current.neck, i, increments);
  int clawNext = interpolate(previous.claw, current.claw, i, increments);

#ifdef _MOVEMENT_DEBUG
  Serial.print("Next: ");
  Serial.print(baseNext);
  Serial.print(", ");
  Serial.print(bodyNext);
  Serial.print(", ");
  Serial.print(neckNext);
  Serial.print(", ");
  Serial.print(clawNext);
  Serial.println();
#endif

  base.write(baseNext);
  body.write(bodyNext);
  neck.write(neckNext);
  claw.write(clawNext);
}

int EEArm::interpolate (int previous, int current, int i, int increments) {
  int delta = current - previous;

  int x = previous + (int)floor(((float)delta / increments) * i);

  return x;

}

int EEArm::getIncrements(armPosition previous, armPosition current) {
  int maxMovement = abs(current.base  - previous.base);

  if (maxMovement < abs(current.body  - previous.body)) {
    maxMovement = abs(current.body - previous.body);
  }

  if (maxMovement < abs(current.neck  - previous.neck)) {
    maxMovement = abs(current.neck  - previous.neck);
  }

  if (maxMovement < abs(current.claw  - previous.claw)) {
    maxMovement = abs(current.claw  - previous.claw);
  }

  int increments = fmax((int)(ceil((float)maxMovement) / _maxIncrement), 1);
#ifdef _MOVEMENT_DEBUG
  Serial.print("increments: ");
  Serial.println(increments);
#endif

  return increments;
}

bool EEArm::saveSteps() {
#ifdef _EEPROM_DEBUG
  Serial.println("Saving Steps");
  Serial.print("Current _writeIndex: ");
  Serial.println(_writeIndex);
#endif

  int i, address = 504, armStepSize = sizeof(armStep);

  for (i = 0; i < _writeIndex; i++) {
#ifdef _EEPROM_DEBUG
    Serial.print("Saving step to address: ");
    Serial.println(address);
#endif

    EEPROM.put<armStep>(address, _armSteps[i]);
    address = address + armStepSize;
  }

  EEPROM.put<int>(500, _writeIndex);
  return EEPROM.commit();
}

bool EEArm::printSteps() {
#ifdef _EEARM_DEBUG
  Serial.print("Step count: ");
  Serial.println(_writeIndex);

  for (int i = 0; i < _writeIndex; i++) {
    printStep(&_armSteps[i]);
  }
#endif

  return true;
}

bool EEArm::loadSteps() {
  EEPROM.get<int>(500, _writeIndex);

#ifdef _EEPROM_DEBUG
  Serial.print("_writeIndex retrieved: ");
  Serial.println(_writeIndex);
#endif

  if (_writeIndex <= 0) {
    _writeIndex = 0;
    return false;
  }

  int i, address = 504, armStepSize = sizeof(armStep);

  for (i = 0; i < _writeIndex; i++) {

#ifdef _EEPROM_DEBUG
    Serial.print("Retrieving step from address: ");
    Serial.println(address);
#endif

    EEPROM.get<armStep>(address, _armSteps[i]);

    address = address + armStepSize;
  }
  return true;
}

bool EEArm::printStep(armStep *currentStep) {
#ifdef _EEARM_DEBUG
  Serial.print(currentStep->pos.base);
  Serial.print(',');
  Serial.print(currentStep->pos.body);
  Serial.print(',');
  Serial.print(currentStep->pos.neck);
  Serial.print(',');
  Serial.print(currentStep->pos.claw);
  Serial.print(',');
  Serial.print(currentStep->steps);
  Serial.print(',');
  Serial.println(currentStep->delay);
#endif
  return true;
}

