/*
eeArm
by Chris Fraser <http://blog.chrosfraser.co.za>
visit http://eearm.com for more info

https://github.com/themakerspace/eeArm-Firmware
*/

#include "EEArm.h"
#include "EEArmConfig.h"
#include "ArduinoJson.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Servo.h>
#include <EEPROM.h>

#define _DEBUG   // Enables general logging

/* Set these to your desired credentials. */
WifiConfig deviceConfig = {};

ESP8266WebServer server(80);

EEArm eeArm;
EEArmConfig eeArmConfig;
bool play = false;

// --------------------------------------------------------
// Setup
// --------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

#ifdef _DEBUG
  Serial.println();
  Serial.println(F("Setup......."));
#endif

  EEPROM.begin(4096);

  if (!eeArmConfig.getWifiConfig(&deviceConfig)) {
    Serial.println("eeArmConfig getConfig failed!");
  }

  SetupAP();

#ifdef _DEBUG
  Serial.println("HTTP server starting");
  Serial.println("----------------------------");
#endif

  server.begin();

  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/add", handleAdd);
  server.on("/pop", handlePop);
  server.on("/clear", handleClear);
  server.on("/getsteps", handleGetSteps);
  server.on("/loadsteps", handleLoadSteps);
  server.on("/savesteps", handleSaveSteps);
  server.on("/loop", handleLoop);
  server.on("/play", handlePlay);
  server.on("/pause", handlePause);
  server.on("/stop", handleStop);
  server.on("/gostart", handleGoToStart);
  server.on("/restart", handleRestart);
  server.on("/settings", handleSettings);
  server.on("/setdefaultcalibration", handleSetDefaultCalibration);
  server.on("/armcalibration", handleArmCalibration);
  server.on("/armstartposition", handleArmStartPosition);

  //eeArm.begin(D1,D2,D3,D4);
  eeArm.begin();

  randomSeed(analogRead(0));
}

// --------------------------------------------------------
// Loop
// --------------------------------------------------------
void loop() {
  server.handleClient();

  if (play) {
    eeArm.play();
    play = false;
  }
  if (Serial.available())
  {
    char command = Serial.read();

    switch (command)
    {
      case 'p':
        eeArm.play();
        Serial.println("Play");
        break;
      case 'a':
        eeArm.addStep({random(0, 180), random(0, 180), random(0, 180), random(0, 180), 0, 0});
        Serial.println("Added");
        break;
      case 's':
        eeArm.saveSteps();
        Serial.println("Saved");
        break;
      case 'c':
        eeArm.clearSteps();
        Serial.println("Cleared");
        break;
      case 'l':
        Serial.println("Printing steps");
        eeArm.printSteps();
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
  armPosition pos = {server.arg("base").toInt(),
                     server.arg("body").toInt(),
                     server.arg("neck").toInt(),
                     server.arg("claw").toInt()
                    };

  eeArm.moveTo(&pos);

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

  eeArm.addStep(step);

  return returnOk("handleAdd");
}

void handlePop() {
#ifdef _DEBUG
  Serial.println("handlePop called: ");
#endif
  eeArm.popStep();
  return returnOk("handlePop");
}

void handleClear() {
#ifdef _DEBUG
  Serial.println("handleClear called: ");
#endif
  eeArm.clearSteps();
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
  eeArm.loadSteps();
  return returnOk("handleSaveSteps");
}

void handleSaveSteps() {
#ifdef _DEBUG
  Serial.println("handleSaveSteps called: ");
#endif
  eeArm.saveSteps();
  return returnOk("handleSaveSteps");
}

void handleGoToStart() {
#ifdef _DEBUG
  Serial.println("handleGoToStart called");
#endif
  eeArm.goToStart();
  return returnArmDetails(true);
}

void handlePlay() {
#ifdef _DEBUG
  Serial.println("handlePlay called");
#endif

  play = true;

  return returnOk("played");
}

void handleLoop() {
#ifdef _DEBUG
  Serial.println("handlePlay called");
#endif
  int delayBetween = server.arg("delayBetween").toInt();
  eeArm.loop(delayBetween);
  return returnArmDetails(true);
}

void handlePause() {
#ifdef _DEBUG
  Serial.println("handlePause called");
#endif
  // Get delay from args
  int delay = server.arg("delay").toInt();
  eeArm.pause(delay);
  return returnArmDetails(true);
}

void handleStop() {
#ifdef _DEBUG
  Serial.println("handlePlay called");
#endif

  eeArm.stop();
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

  armPosition pos =  eeArm.getPosition();
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

void handleSettings() {
#ifdef _DEBUG
  Serial.print("handleSettings called: ");
#endif
  bool restart = false;
  if (server.method() == HTTP_POST ) {
    if (server.args() > 1) {
      deviceConfig.mode = server.arg("mode").toInt();
      strncpy(deviceConfig.name, server.arg("name").c_str(), 32);
      strncpy(deviceConfig.ssid, server.arg("ssid").c_str(), 32);
      strncpy(deviceConfig.pass, server.arg("pass").c_str(), 64);

      if (!eeArmConfig.saveWifiConfig(&deviceConfig)) {
        Serial.println("eeArmConfig getConfig failed!");
      }
      restart = true;
    }
  }

  returnSettings();

  if (restart) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    ESP.restart();
  }
}


void returnSettings() {
#ifdef _DEBUG
  Serial.println("returnSettings called");
#endif

  StaticJsonBuffer<300> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();

  root["mode"] = deviceConfig.mode;
  root["name"] = deviceConfig.name;
  root["ssid"] = deviceConfig.ssid;

  char resp[300];

  root.printTo(resp, sizeof(resp));

  server.send(200, "application/json", resp);
}

void handleSetDefaultCalibration() {
#ifdef _DEBUG
  Serial.print("handleSetDefaultCalibration called: ");
#endif

  eeArmConfig.setDefaultCalibration(&eeArm.config);

  return returnArmSettings();
}

void handleArmCalibration() {
#ifdef _DEBUG
  Serial.print("handleArmCalibration called: ");
#endif

  if (server.method() == HTTP_POST) {
    if (server.args() == 1 && bool(server.arg("default")) == true) {
      // TODO: Set defaults
      return returnArmSettings();
    }

    eeArm.config.baseCal = {
      mapToMillis(server.arg("ba_min").toInt()),
      mapToMillis(server.arg("ba_max").toInt()),
      eeArm.config.baseCal.start
    };
    eeArm.config.bodyCal = {
      mapToMillis(server.arg("bo_min").toInt()),
      mapToMillis(server.arg("bo_max").toInt()),
      eeArm.config.bodyCal.start
    };
    eeArm.config.neckCal = {
      mapToMillis(server.arg("n_min").toInt()),
      mapToMillis(server.arg("n_max").toInt()),
      eeArm.config.neckCal.start
    };
    eeArm.config.clawCal = {
      mapToMillis(server.arg("c_min").toInt()),
      mapToMillis(server.arg("c_max").toInt()),
      eeArm.config.clawCal.start
    };

    eeArm.detach();
    if (!eeArmConfig.saveArmConfig(&eeArm.config)) {
      Serial.println("eeArmConfig getConfig failed!");
    }
    eeArm.attach();
  }

  return returnArmSettings();
}


void handleArmStartPosition() {
#ifdef _DEBUG
  Serial.print("handleArmStartPosition called: ");
#endif

  if (server.method() == HTTP_POST) {
    if (server.args() <= 1) {
      return returnArmSettings();
    }

    eeArm.config.baseCal.start = mapToMillis(server.arg("ba_start").toInt());
    eeArm.config.bodyCal.start = mapToMillis(server.arg("bo_start").toInt());
    eeArm.config.neckCal.start = mapToMillis(server.arg("n_start").toInt());
    eeArm.config.clawCal.start = mapToMillis(server.arg("c_start").toInt());

    eeArm.detach();
    if (!eeArmConfig.saveArmConfig(&eeArm.config)) {
      Serial.println("eeArmConfig getConfig failed!");
    }
    eeArm.attach();
  }

  return returnArmSettings();
}

void returnArmSettings() {
#ifdef _DEBUG
  Serial.println("returnSettings called");
#endif

  StaticJsonBuffer<800> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();

  root["speed"] = eeArm.config.speed;
  root["incrementDelay"] = eeArm.config.incrementDelay;

  JsonObject& base = root.createNestedObject("base");
  base["min"] = eeArm.config.baseCal.min;
  base["max"] = eeArm.config.baseCal.max;
  base["start"] = eeArm.config.baseCal.start;

  JsonObject& body = root.createNestedObject("body");
  body["min"] = eeArm.config.bodyCal.min;
  body["max"] = eeArm.config.bodyCal.max;
  body["start"] = eeArm.config.bodyCal.start;

  JsonObject& neck = root.createNestedObject("neck");
  neck["min"] = eeArm.config.neckCal.min;
  neck["max"] = eeArm.config.neckCal.max;
  neck["start"] = eeArm.config.neckCal.start;

  JsonObject& claw = root.createNestedObject("claw");
  claw["min"] = eeArm.config.clawCal.min;
  claw["max"] = eeArm.config.clawCal.max;
  claw["start"] = eeArm.config.clawCal.start;

  char resp[800];

  root.printTo(resp, sizeof(resp));

  server.send(200, "application/json", resp);
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
  pinMode(5, INPUT_PULLUP);
  // Set Hostname.
  WiFi.hostname(deviceConfig.name);
  delay(50);

  // Print hostname.
  Serial.print("hostname: ");
  Serial.println(WiFi.hostname());

  if (deviceConfig.mode == 1 && digitalRead(5) != 0) {

    // Check WiFi connection
    // ... check mode
    if (WiFi.getMode() != WIFI_STA)
    {
      WiFi.mode(WIFI_STA);
      delay(10);
    }

    // ... Compare file config with sdk config.
    if (WiFi.SSID() != deviceConfig.ssid || WiFi.psk() !=  deviceConfig.pass )
    {
      Serial.println("WiFi config changed.");

      // ... Try to connect to WiFi station.
      WiFi.begin(deviceConfig.ssid, deviceConfig.pass);

      // ... Pritn new SSID
      Serial.print("new SSID: ");
      Serial.println(WiFi.SSID());
    }
    else
    {
      // ... Begin with sdk config.
      WiFi.begin();
    }

    Serial.println("Wait for WiFi connection.");

    // ... Give ESP 10 seconds to connect to station.
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
    {
      Serial.write('.');
      //Serial.print(WiFi.status());
      delay(500);
    }
    Serial.println();
  }

  // If not connected start the soft ap
  if (WiFi.status() != WL_CONNECTED) {

    Serial.print("Startup AP Mode: ");
    Serial.println(deviceConfig.name);
    WiFi.mode(WIFI_AP);
    delay(10);

    // Setup AP
    WiFi.softAP(deviceConfig.name, "aaaabbbb");
    delay(50);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }

  // Initialize mDNS service.
  MDNS.begin(deviceConfig.name);

  // Add MDNS service.
  MDNS.addService("http", "tcp", 80);
}

int mapToMillis(int deg) {
  return map(deg, 0, 180, 600, 2400);
}

