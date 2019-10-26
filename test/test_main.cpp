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

static int beforeWainting = 0;
static int wainting = 0;
static int afterWainting = 0;

void test_connection() {
  //Splitting settings in wlanConf and MqttConf
  JsonObject wlanConf = conf.getJsonObject("wlan");
  JsonObject mqttConf = conf.getJsonObject("mqtt");
  JsonObject nullWlanConf;
  JsonObject nullMqttConf;

  man.onBeforeWaitingWiFiCon([&]() mutable {
    beforeWainting++;
  });

  man.onWaitingWiFiCon([&]() mutable {
    wainting++;
  });
  man.onAfterWaitingWiFiCon([&]() mutable {
    afterWainting++;
  });

  TEST_ASSERT_MESSAGE(ESPManConnStatus::INVALID_WLAN_CONF == man.createConnections(nullWlanConf, mqttConf), "Wrong status return for invalind wlan configuration");
  TEST_ASSERT_MESSAGE(ESPManConnStatus::INVLIAD_MQTT_CONF == man.createConnections(wlanConf, nullMqttConf), "Wrong status return for invalind MQTT configuration");
  TEST_ASSERT_MESSAGE(ESPManConnStatus::CONNECTION_OK == man.createConnections(wlanConf, mqttConf), "Could not connect");
}

void test_connectionListeners() {
  TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(1, beforeWainting, "onBeforeWaitingWiFiCon method was not called");
  TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(1, wainting, "onWaitingWiFiCon method was not called");
  TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(1, afterWainting, "onAfterWaitingWiFiCon method was not called");
}

void test_incommingMessages() {
  int call = 0;

  man.addIncomingEventHandler("unitTest/incommingTopic", [&](const char *msg) mutable {
    call++;
  });
  String topic = "unitTest/incommingTopic";
  String msg = "msg";
  man.messageReceived(topic, msg);
  TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(1, call, "incommingTopic was not called");
}

//Ignorred
void test_sendMessage() {
  if (!man.mqttCli.connected()) {
    TEST_FAIL_MESSAGE("mqtt not connected");
  }
  TEST_IGNORE_MESSAGE("Test NOT working");
  char *returnedMessage = nullptr;
  char sentMsg[30];
  sprintf(sentMsg, "untiTestMessage %lu", random(10, 1000));
  man.addIncomingEventHandler("untiTest/sendMessage", [&](const char *msg) mutable {
    returnedMessage = strdup(msg);
  });

  man.sendMsg("untiTest/sendMessage", sentMsg);
  man.mqttCli.publish("untiTest/sendMessage", "msg");
  // TEST_ASSERT_MESSAGE(strcmp(sentMsg, returnedMessage) == 0, "sentMessage is not working");
}

void test_timeoutMessages() {
  int call = 0;
  //add listener that will come from server side that will be at topic unitTest/response/<event>
  man.addIncomingEventHandler("unitTest/response/timerTestEventTopic", [](const char *msg) {
    TEST_ASSERT_MESSAGE(strcmp("methodCalled", msg) == 0, "Not the expected response");
  });

  //set timing for sending a message for timerTestEventTopic event
  man.addTimerOutputEventHandler("unitTest/timerTestEventTopic", 1000, [&](const char *value) mutable -> char * {
    call++;
    char *ret = (char *)malloc(25 * sizeof(char));
    strcpy(ret, "methodCalled");
    return ret;
  });
  //trigger function in offline mode this should not send the message because the timing did not past yet
  man.executeTimingOutputEvents();
  TEST_ASSERT_LESS_OR_EQUAL_INT_MESSAGE(0, call, "timerOutEventHandler called too soon");

  String topic = "incommingTopic";
  String msg = "msg";
  delay(1010);
  //trigger function in offline mode this should send the message because the timing did past
  man.executeTimingOutputEvents();
  TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(1, call, "timerOutEventHandler was not called");
}

void test_reconnect() {
  man.reconnect();
  TEST_ASSERT_MESSAGE(man.mqttCli.connected(), "Could not reconnect");
}

void test_sendInfo() {
  man.reconnect();
  TEST_IGNORE_MESSAGE("Test NOT working");
  char *returnedMessage = nullptr;
  JsonVariant nullObj;
  man.cmdGetInfo("unitTest/getInfoText", nullObj);
  man.addIncomingEventHandler("unitTest/response/getInfoText", [&](const char *msg) mutable -> void {
    returnedMessage = strdup(msg);
    TEST_ASSERT_MESSAGE(strcmp("methodCalled", msg) == 0, "Not the expected response");
  });
  for (int i = 0; i < 2000; i++) {
    delay(10);
    man.mqttCli.loop();
  }
  TEST_ASSERT_MESSAGE(true, "test");
}

void setup() {
  conf.readSettings("/settings.json");

  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_connection);
  RUN_TEST(test_connectionListeners);
  RUN_TEST(test_sendMessage);
  RUN_TEST(test_incommingMessages);
  RUN_TEST(test_timeoutMessages);
  RUN_TEST(test_reconnect);
  RUN_TEST(test_sendInfo);
  UNITY_END();
}

void loop() {
  delay(500);
}