/*
  ESPManager 2.0.0

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

#include <ArduinoJson.h>
#include "ESPManager.h"
#include "SettingsManager.h"

const char * readTemp(const char * msg);
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
  //Creating connection to wlan and mqtt
  man.createConnections(wlanConf, mqttConf);
  //Add listener on IOT/espTest/inc
  man.addIncomingEventHandler("IOT/espTest/inc", onCall);
  //Adding timout trigger
  man.addTimerOutputEventHandler("IOT/espTest/out", 2000, readTemp);
  //Send instant message on IOT/espTest/out
  man.sendMsg("IOT/espTest/out", "test");
}

void loop() {
  man.loopIt();
}

const char * readTemp(const char * msg) {
  return "{temp:39, humidity: 75}";
};

void onCall(const char * msg) {
  Serial.println(msg);
};
