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

// ---------------------------
// begin
// ---------------------------
armPosition EEArm::begin(uint8_t basePin, uint8_t bodyPin, uint8_t neckPin, uint8_t clawPin) {
  base.attach(basePin);
  body.attach(bodyPin);
  neck.attach(neckPin);
  claw.attach(clawPin);

  // Load steps from EEPROM
  loadSteps();

  return getPosition();
}
// ---------------------------
// detach
// ---------------------------
void EEArm::detach() {
  base.detach();
  body.detach();
  neck.detach();
  claw.detach();
}
// ---------------------------
// attach
// ---------------------------
armPosition EEArm::attach(uint8_t basePin, uint8_t bodyPin, uint8_t neckPin, uint8_t clawPin) {
  base.attach(basePin);
  body.attach(bodyPin);
  neck.attach(neckPin);
  claw.attach(clawPin);
  
  return getPosition();
}

// ---------------------------
// getPosition
// ---------------------------
armPosition EEArm::getPosition() {
  armPosition pos = {
    base.read() + 1,
    body.read() + 1,
    neck.read() + 1,
    claw.read() + 1
  };

  return pos;
}

// ---------------------------
// moveTo
// ---------------------------
armPosition EEArm::moveTo(armPosition *pos) {
  // Calibration mapping
  pos->base = map(pos->base, 0, 180 ,config.baseCal.min, config.baseCal.max);
  pos->body = map(pos->body, 0, 180 ,config.bodyCal.min, config.bodyCal.max);
  pos->neck = map(pos->neck, 0, 180 ,config.neckCal.min, config.neckCal.max);
  pos->claw = map(pos->claw, 0, 180 ,config.clawCal.min, config.clawCal.max);
  
  armPosition current = getPosition();
  move(&current, pos);
  return getPosition();
}


// ---------------------------
// addStep: Adds a step and returns the number of steps saved
// ---------------------------
int EEArm::addStep(armStep step) {
  // Calibration mapping
  step.pos.base = map(step.pos.base, 0, 180 ,config.baseCal.min, config.baseCal.max);
  step.pos.body = map(step.pos.body, 0, 180 ,config.bodyCal.min, config.bodyCal.max);
  step.pos.neck = map(step.pos.neck, 0, 180 ,config.neckCal.min, config.neckCal.max);
  step.pos.claw = map(step.pos.claw, 0, 180 ,config.clawCal.min, config.clawCal.max);
  
  _armSteps[_writeIndex++] = step;
  return _writeIndex;
}

// ---------------------------
// popStep: Pops a step and returns the number of remaining steps
int EEArm::popStep() {
  _writeIndex--;
  return _writeIndex;
}

// ---------------------------
// clearSteps: Clears steps and returns if it was successful
// ---------------------------
bool EEArm::clearSteps() {
  _writeIndex = 0;

  EEPROM.put<int>(500, _writeIndex);
  return EEPROM.commit();
}

// ---------------------------
// saveSteps: Saves steps to EEPROM
// ---------------------------
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

// ---------------------------
// loadSteps: Loads steps from EEPROM
// ---------------------------
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

// ---------------------------
// goToStart: Clears steps and returns if it was successful
// ---------------------------
armPosition EEArm::goToStart() {
  armStep step = _armSteps[0];
  armPosition current = getPosition();
  move(&current, &step.pos);

  return getPosition();
}

// ---------------------------
// play
// ---------------------------
bool EEArm::play() {
#ifdef _MOVEMENT_DEBUG
  Serial.print("Playing...  Steps: ");
  Serial.println(_writeIndex);
#endif
  // Bail if there are no steps
  if(_writeIndex == 0){
    Serial.println("No steps to print");
    return false;
  }
  // previousStep initially set to start position
  armStep previousStep = {getPosition(), 0, 0};
  armStep currentStep;

  // Loop through steps
  for (int i = 0; i < _writeIndex; i++) {
    currentStep = _armSteps[i];
    Serial.println(i);
    move(&previousStep.pos, &currentStep.pos, currentStep.steps);

    delay(currentStep.delay);
    previousStep = currentStep;
  } 

  return true;
}

// ---------------------------
// loop
// ---------------------------
bool EEArm::loop(int delayBetween) {

  return false;
}

// ---------------------------
// pause
// ---------------------------
armPosition EEArm::pause(int delay) {

  return getPosition();
}

// ---------------------------
// stop
// ---------------------------
armPosition EEArm::stop() {

  return getPosition();
}

// ----------------------------------------------------
// Private
// ----------------------------------------------------
void EEArm::move(armPosition* previous, armPosition* current, int steps) {

  int increments = 1;
#ifdef _MOVEMENT_DEBUG
  Serial.print("From: ");
  Serial.print(previous->base);
  Serial.print(", ");
  Serial.print(previous->body);
  Serial.print(", ");
  Serial.print(previous->neck);
  Serial.print(", ");
  Serial.print(previous->claw);
  Serial.print(" ===> ");
  Serial.print(current->base);
  Serial.print(", ");
  Serial.print(current->body);
  Serial.print(", ");
  Serial.print(current->neck);
  Serial.print(", ");
  Serial.print(current->claw);
  Serial.println();
#endif
  // If no steps are defined
  if (steps == 0) {
    increments = getIncrements(previous, current);
  }

  // Loop through the increments
  for (int i = 0; i < increments; i++) {
    moveServoIncrement(previous, current, i, increments);
    delay(config.incrementDelay);
  }
}

void EEArm::moveServoIncrement(armPosition *previous, armPosition *current, int i, int increments) {
  int baseNext = interpolate(previous->base, current->base, i, increments);
  int bodyNext = interpolate(previous->body, current->body, i, increments);
  int neckNext = interpolate(previous->neck, current->neck, i, increments);
  int clawNext = interpolate(previous->claw, current->claw, i, increments);

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

int EEArm::getIncrements(armPosition *previous, armPosition *current) {
  int maxMovement = abs(current->base  - previous->base);

  if (maxMovement < abs(current->body  - previous->body)) {
    maxMovement = abs(current->body - previous->body);
  }

  if (maxMovement < abs(current->neck  - previous->neck)) {
    maxMovement = abs(current->neck  - previous->neck);
  }

  if (maxMovement < abs(current->claw  - previous->claw)) {
    maxMovement = abs(current->claw  - previous->claw);
  }

  int increments = fmax((int)(ceil((float)maxMovement) / config.speed), 1);
#ifdef _MOVEMENT_DEBUG
  Serial.print("increments: ");
  Serial.println(increments);
#endif

  return increments;
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

