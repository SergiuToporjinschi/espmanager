/*
  ESPManager

  Copyright (C) 2018 by Sergiu Toporjinschi <sergiu dot toporjinschi at gmail dot com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Ctrl+k > open data folder, edit settings.json from data folder
  Set values from wlan and mqtt

*/

#include "ESPManager.h"
#include "SettingsManager.h"
#include <ArduinoJson.h>

char *readTemp(const char *msg);
void onCall(const char *msg);

SettingsManager conf;
ESPManager man;

void setup() {
  Serial.begin(115200);

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
    Serial.println("#");
  });
  man.onAfterWaitingWiFiCon([]() {
    Serial.println("onAfterWaitingWiFiCon");
  });

  //adding events for waiting connection at MQTT
  man.onBeforeWaitingMQTTCon([]() {
    Serial.println("onBeforeWaitingMQTTCon");
  });
  man.onWaitingMQTTCon([]() {
    Serial.println("-");
  });
  man.onAfterWaitingMQTTCon([]() {
    Serial.println("onAfterWaitingMQTTCon");
  });

  //Creating connection to wlan and mqtt
  man.createConnections(wlanConf, mqttConf);

  // Add listener on IOT/espTest/inc
  man.addIncomingEventHandler("IOT/espTest/inc", onCall);
  //Adding timout trigger
  man.addTimerOutputEventHandler("IOT/espTest/out", 2000, readTemp);
  //Send instant message on IOT/espTest/out
  man.sendMsg("IOT/espTest/out", "test");
}

void loop() {
  man.loopIt();
}

char * readTemp(const char * msg) {
  //Allocate memory, will be freed by manager
  char * ret = (char *) malloc(25 * sizeof(char));
  strcpy(ret, "{temp:39, humidity: 75}");
  return ret;
};

void onCall(const char * msg) {
  Serial.println(msg);
};
