#include "eeArm.h"
#include "ArduinoJson.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Servo.h>
#include <EEPROM.h>

#define _DEBUG   // Enables general logging

/* Set these to your desired credentials. */
const char *deviceName = "GwaTest Arm";

ESP8266WebServer server(80);

EEArmClass EEArm;

// --------------------------------------------------------
// Setup
// --------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);
#ifdef _DEBUG
  Serial.println();
  Serial.println(F("setup......."));
#endif

  EEPROM.begin(4096);
  
  SetupAP();
  
#ifdef _DEBUG
  Serial.println("HTTP server starting");
  Serial.println("----------------------------");
#endif

  server.begin();

  server.on("/", handleRoot);
  server.on("/EEArm", handleArm);
  server.on("/add", handleAdd);
  server.on("/pop", handlePop);
  server.on("/clear", handleClear);
  server.on("/getSteps", handleGetSteps);
  server.on("/loadSteps", handleLoadSteps);
  server.on("/saveSteps", handleSaveSteps);
  server.on("/loop", handleLoop);
  server.on("/go", handlePlay);
  server.on("/play", handlePlay);
  server.on("/pause", handlePause);
  server.on("/stop", handleStop);
  server.on("/gostart", handleGoToStart);
  server.on("/restart", handleRestart);


  EEArm.begin(13, 12, 14, 16);
  
  randomSeed(analogRead(0));
}

// --------------------------------------------------------
// Loop
// --------------------------------------------------------
void loop() {
  server.handleClient();

  if (Serial.available())
  {
    char command = Serial.read();

    switch (command)
    {
      case 'p':
        EEArm.play();
        Serial.println("Play");
        break;
      case 'a':
        EEArm.addStep({random(50, 120), random(40, 110), random(40, 110), random(10, 60), 0, 0});
        Serial.println("Added");
        break;
      case 's':
        EEArm.saveSteps();
        Serial.println("Added");
        break;
      case 'c':
        EEArm.clearSteps();
        Serial.println("Cleared");
        break;
      case 'l':
        EEArm.printSteps();
        break;
    }
  }
}

// --------------------------------------------------------
// Methods
// --------------------------------------------------------
void handleRoot() {
  server.send(200, "text/html", F("<h1>You are connected</h1>"));
}

void handleArm() {
#ifdef _DEBUG
  Serial.print("handleArm called: ");
#endif

  if (server.args() == 0 && server.method() == HTTP_GET) {
    return returnArmDetails(false);
  }

#ifdef _DEBUG
  Serial.print(server.arg("base"));
  Serial.print(", ");
  Serial.print(server.arg("body"));
  Serial.print(", ");
  Serial.print(server.arg("neck"));
  Serial.print(", ");
  Serial.println(server.arg("claw"));
#endif

  EEArm.moveTo({server.arg("base").toInt(),
              server.arg("body").toInt(),
              server.arg("neck").toInt(),
              server.arg("claw").toInt()
             });

  return returnArmDetails(true);
}

void handleAdd() {
#ifdef _DEBUG
  Serial.println("handleAdd called: ");
#endif

  armStep step = {
    server.arg("base").toInt(),
    server.arg("body").toInt(),
    server.arg("neck").toInt(),
    server.arg("claw").toInt(),
    server.arg("steps").toInt(),
    server.arg("delay").toInt()
  };

  EEArm.addStep(step);

  return returnOk("handleAdd");
}

void handlePop() {
#ifdef _DEBUG
  Serial.println("handlePop called: ");
#endif
  EEArm.popStep();
  return returnOk("handlePop");
}

void handleClear() {
#ifdef _DEBUG
  Serial.println("handleClear called: ");
#endif
  EEArm.clearSteps();
  return returnOk("handleClear");
}

void handleGetSteps() {
#ifdef _DEBUG
  Serial.println("handleGetSteps called: ");
#endif

  return returnOk("20,20,20,20,20|30,30,30,30,30|1,2,3,4,5");
}

void handleLoadSteps() {
#ifdef _DEBUG
  Serial.println("handleSaveSteps called: ");
#endif
  EEArm.loadSteps();
  return returnOk("handleSaveSteps");
}

void handleSaveSteps() {
#ifdef _DEBUG
  Serial.println("handleSaveSteps called: ");
#endif
  EEArm.saveSteps();
  return returnOk("handleSaveSteps");
}

void handleGoToStart() {
#ifdef _DEBUG
  Serial.println("handleGoToStart called");
#endif
  EEArm.goToStart();
  return returnArmDetails(true);
}

void handlePlay() {
#ifdef _DEBUG
  Serial.println("handlePlay called");
#endif
  EEArm.play();

  //   EEArm.play(); returns the position. Wasteful call below
  return returnArmDetails(true);
}

void handleLoop() {
#ifdef _DEBUG
  Serial.println("handlePlay called");
#endif
  int delayBetween = server.arg("delayBetween").toInt();
  EEArm.loop(delayBetween);
  return returnArmDetails(true);
}

void handlePause() {
#ifdef _DEBUG
  Serial.println("handlePause called");
#endif
  // Get delay from args
  int delay = server.arg("delay").toInt();
  EEArm.pause(delay);
  return returnArmDetails(true);
}

void handleStop() {
#ifdef _DEBUG
  Serial.println("handlePlay called");
#endif

  EEArm.stop();
  return returnArmDetails(true);
}

void returnArmDetails(bool moved) {
#ifdef _DEBUG
  Serial.println("returnArmDetails called");
#endif

  StaticJsonBuffer<300> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();

  if (moved) {
    root["moved"] = true;
  }

  armPosition pos =  EEArm.getPosition();
  root["base"] = pos.base;
  root["body"] = pos.body;
  root["neck"] = pos.neck;
  root["claw"] = pos.claw;

  char resp[300];

  root.printTo(resp, sizeof(resp));

  server.send(200, "application/json", resp);
}

void handleDetails() {
#ifdef _DEBUG
  Serial.println("handleDetails called");
#endif
  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  root["freeHeap"] = ESP.getFreeHeap();
  root["chipId"] = ESP.getChipId();
  root["flashChipId"] = ESP.getFlashChipId();
  root["flashChipSize"] = ESP.getFlashChipSize();

  char response[200];

  root.printTo(response, sizeof(response));

  server.send(200, "application/json", response);
}

void handleRestart() {
#ifdef _DEBUG
  Serial.println("Restarting...");
#endif
  ESP.restart();
}

void handleSetIncrementDelay() {
#ifdef _DEBUG
  Serial.println("handleSetIncrementDelay...");
#endif
  return returnOk("handleSetIncrementDelay");
}

void handleSetMaxDegrees() {
#ifdef _DEBUG
  Serial.println("handleSetMaxDegrees...");
#endif
  return returnOk("handleSetMaxDegrees");
}

void returnOk(String response) {
#ifdef _DEBUG
  Serial.println("returnOk called");
#endif
  server.send(200, "text/plain", response);
}

void SetupAP() {
#ifdef _DEBUG
  Serial.println("Configuring access point...");
#endif

  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(deviceName);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}

