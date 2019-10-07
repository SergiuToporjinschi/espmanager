/* 

  espManager

  Copyright (C) 2018 by Sergiu Toporjinschi <sergiu dot toporjinschi at gmail dot com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 3.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://spdx.org/licenses/GPL-3.0-only.html>.

  All rights reserved

*/

#ifdef DEBUGGER
#  include "debug_macro.h"
Print *dbg = &Serial;
#endif

#include "ESPManager.h"
#include "SettingsManager.h"
#include "SoftwareSerial.h"
#include <Arduino.h>
#include <ArduinoJson.h>

char *readTemp(const char *msg);
void onCall(const char *msg);

SettingsManager conf;
ESPManager man;

char *onGetConf(JsonVariant params);
char *onSetConf(JsonVariant params);
void onCall(const char *msg);
char *readTemp(const char *msg);

void setup() {
  Serial.begin(115200);

#ifdef DEBUGGER
  dbg = &Serial;
#endif

  //Reading configuration from json file
  conf.readSettings("/settings.json");

  //Splitting settings in wlanConf and MqttConf
  JsonObject wlanConf = conf.getJsonObject("wlan");
  JsonObject mqttConf = conf.getJsonObject("mqtt");

  //Setting scketch ino verion
  man.setSketchVersion("1.0.0");

  //adding events for waiting connection at WiFi
  man.onBeforeWaitingWiFiCon([]() {
    Serial.println("onBeforeWaitingWiFiCon");
  });
  man.onWaitingWiFiCon([]() {
    Serial.print("#");
  });
  man.onAfterWaitingWiFiCon([]() {
    Serial.println("onAfterWaitingWiFiCon");
  });

  //adding events for waiting connection at MQTT
  man.onBeforeWaitingMQTTCon([]() {
    Serial.println("onBeforeWaitingMQTTCon");
  });
  man.onWaitingMQTTCon([]() {
    Serial.print("-");
  });
  man.onAfterWaitingMQTTCon([]() {
    Serial.println("onAfterWaitingMQTTCon");
  });

  //Creating connection to wlan and mqtt
  man.createConnections(wlanConf, mqttConf);

  // Add listener on IOT/espTest/inc
  man.addIncomingEventHandler("IOT/espTest/inc", onCall);

  man.addCommand("setConf", onSetConf);

  // Add custom command
  man.addCommand("getConf", onGetConf);

  // Add listener for changing configuration
  // man.addIncomingEventHandler("IOT/espTest/getconf", onGetConf);

  //Adding timout trigger
  man.addTimerOutputEventHandler("IOT/espTest/out", 2000, readTemp);

  //Send instant message on IOT/espTest/out
  man.sendMsg("IOT/espTest/out", "test");
  Serial.println("SetupFinish");
}

void loop() {
  man.loopIt();
}

char *readTemp(const char *msg) {
  //Allocate memory, will be freed by manager
  char *ret = (char *)malloc(25 * sizeof(char));
  strcpy(ret, "{temp:39, humidity: 75}");
  return ret;
};

void onCall(const char *msg) {
  Serial.println(msg);
};

/**
 * Set the entire configuration file,
 * I'm serializing the conf and set it as root for current settings
 * A restart in necessary to load the new settings
 **/
char *onSetConf(JsonVariant params) {
  conf.writeSettings("/settings.json", params);
  return nullptr;
};

/**
 * Get the entire configuration file
 **/
char *onGetConf(JsonVariant params) {
  char settings[1024] = {0};
  JsonObject root = conf.getRoot();
  serializeJson(root, settings);
  char *retSettings = (char *)malloc(1000 * sizeof(char));
  strcpy(retSettings, settings);
  return retSettings;
};