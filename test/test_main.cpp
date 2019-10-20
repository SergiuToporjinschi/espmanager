/* 

  ESPManager

  Copyright (C) 2017 by Sergiu Toporjinschi <sergiu dot toporjinschi at gmail dot com>

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
#include <unity.h>

#include "../src/ESPManager.h"
#include "SettingsManager.h"

ESPManager man;
SettingsManager conf;

void test_connection() {
  //Splitting settings in wlanConf and MqttConf
  JsonObject wlanConf = conf.getJsonObject("wlan");
  JsonObject mqttConf = conf.getJsonObject("mqtt");
  JsonObject nullWlanConf;
  JsonObject nullMqttConf;

  int beforeWainting = 0;
  int wainting = 0;
  int afterWainting = 0;

  man.onBeforeWaitingWiFiCon([&beforeWainting]() {
    beforeWainting++;
  });

  man.onWaitingWiFiCon([&wainting]() {
    wainting++;
  });
  man.onAfterWaitingWiFiCon([&afterWainting]() {
    afterWainting++;
  });

  TEST_ASSERT_MESSAGE(ESPManConnStatus::INVALID_WLAN_CONF == man.createConnections(nullWlanConf, mqttConf), "Wrong status return for invalind wlan configuration");
  TEST_ASSERT_MESSAGE(ESPManConnStatus::INVLIAD_MQTT_CONF == man.createConnections(wlanConf, nullMqttConf), "Wrong status return for invalind MQTT configuration");
  TEST_ASSERT_MESSAGE(ESPManConnStatus::CONNECTION_OK == man.createConnections(wlanConf, mqttConf), "Could not connect");
  
  TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(1, beforeWainting, "onBeforeWaitingWiFiCon method was not called");
  TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(1, wainting, "onWaitingWiFiCon method was not called");
  TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(1, afterWainting, "onAfterWaitingWiFiCon method was not called");
}

void test_connectionListeners() {
}

void test_incommingMessages() {
  int call = 0;
  man.addIncomingEventHandler("incommingTopic", [&call](const char *msg) {
    call++;
  });
  String topic = "incommingTopic";
  String msg = "msg";
  man.messageReceived(topic, msg);
  TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(1, call, "incommingTopic was not called");
}

void setup() {
  conf.readSettings("/settings.json");

  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_connection);
  RUN_TEST(test_connectionListeners);
  RUN_TEST(test_incommingMessages);
  UNITY_END();
}

void loop() {
  delay(500);
}