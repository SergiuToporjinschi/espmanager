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

#include "debug_macro.h"

#include "ESPManager.h"

ADC_MODE(ADC_VCC);

ESPManager::ESPManager() {
  mqttCli = *new MQTTClient(MQTT_BUFFER);
  wifiMode = WIFI_STA;
  cbBind = new Binding<String &, String &>(*this, &ESPManager::messageReceived);
}

void ESPManager::addCommand(const char *cmd, cmdFunction handler) {
  commands[cmd] = handler;
}

/**
   ---==[ This needs to be call in setup function ]==---
   Creates connection to WiFi;
   Set-up MQTT;
   Wait to be connected to WiFi
   Connects to MQTT server
   Subscribe to MQTT topics set in config
*/
ESPManConnStatus ESPManager::createConnections(JsonObject wlanConf, JsonObject mqttConf) {
  if (wlanConf.isNull()) {
    return INVALID_WLAN_CONF;
  } else if (mqttConf.isNull()) {
    return INVLIAD_MQTT_CONF;
  }

  _wlanConf = wlanConf;
  _mqttConf = mqttConf;
  readconfiguration();

  createConnections();
  return CONNECTION_OK;
}

void ESPManager::readconfiguration() {
  sendOfflineStatus = !_mqttConf.getMember(F("sendOfflineStatus")).isNull() && _mqttConf.getMember(F("sendOfflineStatus")).as<bool>();

  JsonVariant mqttTopics = _mqttConf.getMember(F("topics"));
  if (!mqttTopics.isNull()) {
    JsonVariant mqttTopicsCmd = mqttTopics.getMember(F("cmd"));
    if (!mqttTopicsCmd.isNull()) {
      cmdTopic = mqttTopicsCmd.as<const char *>();
      DBGLN(cmdTopic);
      strcpy(cmdTopicResp, cmdTopic);
      strcat(cmdTopicResp, "/resp");
    }
  }
}
void ESPManager::createConnections() {
  retainMsg = !_mqttConf.getMember(F("retainMessage")).isNull() && _mqttConf.getMember(F("retainMessage")).as<bool>();

  if (!_mqttConf.getMember(F("qos")).isNull() && _mqttConf.getMember(F("qos")).is<int>()) {
    qos = _mqttConf.getMember(F("qos")).as<int>();
  }

  connectToWifi();
  setupMQTT();
  connectToMQTT();

  if (sendOfflineStatus) {
    setOnlineStatusMessage();
  }
  subscribeCMD();
}

/**
   Create connection to WiFi based on settings.wlan if curent status is not connected and waits for connection to be made;
*/
void ESPManager::connectToWifi() {
  DBGLN("Connecting WiFi...");
  debugWiFiStatus();
  if (WiFi.status() == WL_CONNECTED) {
    DBGLN("Is already connected: %i", WiFi.status());
    return;
  }
  WiFi.mode(wifiMode);
  WiFi.hostname(_wlanConf.getMember(F("hostName")).as<char *>());
  delay(100);
  WiFi.begin(_wlanConf.getMember(F("ssid")).as<char *>(), _wlanConf.getMember(F("password")).as<char *>());
  waitForWiFi();
#ifdef EM_UDP_DEBUG
  initDebugUDP();
#endif
}

/**
   Wait to be connected in wifi;
*/
void ESPManager::waitForWiFi() {
  DBGLN("waitForWiFi");
  if (this->beforeWaitingWiFiCon != nullptr) {
    DBGLN("hasBefore");
    this->beforeWaitingWiFiCon();
  }
  int waitingTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - waitingTime < 30000) {
    DBGLN("#");
    if (this->waitingWiFiCon != nullptr) {
      this->waitingWiFiCon();
    }
    delay(10);
  }
  if (this->afterWaitingWiFiCon != nullptr) {
    DBGLN("hasAfter");
    this->afterWaitingWiFiCon();
  }
  if (WiFi.status() != WL_CONNECTED) {
    debugWiFiStatus();
    ESP.restart();
  }
  DBGLN("IP: %s", WiFi.localIP().toString().c_str());
}

#ifdef EM_UDP_DEBUG
void ESPManager::initDebugUDP() {
  DBGLN("UDP DEBUG:");
  JsonVariant debugUdp = _wlanConf.getMember(F("debugUDP"));
  if (WiFi.status() == WL_CONNECTED &&
      !debugUdp.isNull() &&
      !debugUdp.getMember(F("enabled")).isNull() &&
      debugUdp.getMember(F("enabled")).as<boolean>() &&
      !debugUdp.getMember(F("server")).isNull() &&
      !debugUdp.getMember(F("port")).isNull()) {

    udpDebugIP.fromString(debugUdp.getMember(F("server")).as<char *>());
    udpDebugPort = debugUdp.getMember(F("port")).as<unsigned short>();
    DBGLN("port: %i", udpDebugPort);
    DBGLN("ip: %s", udpDebugIP.toString().c_str());
    rst_info *resetInfo = ESP.getResetInfoPtr();
    char buff[200] = {0};
    sprintf_P(&buff[0], DEUBG_UDP_MASK_P, resetInfo->exccause, resetInfo->reason, (resetInfo->reason == 0 ? "DEFAULT" : resetInfo->reason == 1 ? "WDT" : resetInfo->reason == 2 ? "EXCEPTION" : resetInfo->reason == 3 ? "SOFT_WDT" : resetInfo->reason == 4 ? "SOFT_RESTART" : resetInfo->reason == 5 ? "DEEP_SLEEP_AWAKE" : resetInfo->reason == 6 ? "EXT_SYS_RST" : "???"), resetInfo->epc1, resetInfo->epc2, resetInfo->epc3, resetInfo->excvaddr, resetInfo->depc);
    if (Udp.beginPacket(udpDebugIP, udpDebugPort)) {
      Udp.write(buff);
      Udp.endPacket();
    }
  }
}
#endif

/**
   Is printing connection status to WiFi if is not connected;
*/
void ESPManager::debugWiFiStatus() {
#ifdef DEBUGGER
  int wiFiStatus = WiFi.status();
  DBGLN("WiFi status: %i, %s", wiFiStatus, wiFiStatus == WL_CONNECTED ? " CONNECTED" : (wiFiStatus == WL_CONNECT_FAILED ? " FAILED" : (wiFiStatus == WL_DISCONNECTED ? " DISCONNECTED" : (wiFiStatus == WL_CONNECTION_LOST ? " CONNECTION_LOST " : ""))));
#endif
}

/**
   Is setting-up the MQTT client baaed on settings.mqtt;
*/
void ESPManager::setupMQTT() {
  const char *mqttServer = _mqttConf.getMember(F("server")).as<const char *>();
  int mqttPort = _mqttConf.getMember(F("port")).as<int>();
  DBGLN("Setting MQTT: %s; port: %i ", mqttServer, mqttPort);

  mqttCli.begin(mqttServer, mqttPort, mqttNetClient);

  mqttCli.onMessage(cbBind->callback); //TODO de implementat

  DBGLN("SendOfflineStatus: %s", sendOfflineStatus ? "true" : "false");
  if (sendOfflineStatus) {
    setOfflineStatusMessage();
  }
  DBGLN("Finish setupMQTT");
}

/**
   Configuring status message as offline
*/
void ESPManager::setOfflineStatusMessage() {
  const char *topicStatus = _mqttConf.getMember(F("topics")).getMember(F("status")).as<const char *>();
  char msg[100] = {0};

  snprintf_P(msg, 99, STATUS_FORMAT_P, _wlanConf.getMember(F("hostName")).as<char *>(), STATUS_OFFLINE_P);

  DBGLN("Setting offline message on topic: %s;  content: %s", topicStatus, msg);

  mqttCli.clearWill();
  mqttCli.setWill(topicStatus, msg, true, 2);
}

/**
   Configuring status message as online and publish it
*/
void ESPManager::setOnlineStatusMessage() {
  const char *topicStatus = _mqttConf.getMember(F("topics")).getMember(F("status")).as<const char *>();
  char msg[100] = {0};

  snprintf(msg, 99, STATUS_FORMAT_P, _wlanConf.getMember(F("hostName")).as<char *>(), STATUS_ONLINE_P);

  DBGLN("Setting offline message on topic: %s; content: %s", topicStatus, msg);
  mqttCli.publish(topicStatus, msg, true, 2);
}

/**
   Connects on MQTT with credentials from settings.mqtt;
*/
void ESPManager::connectToMQTT() {
  DBGLN("Connecting mqtt...");
  const char *clientId;
  JsonVariant client = _mqttConf.getMember(F("clientId"));
  if (!client.isNull()) {
    clientId = _mqttConf.getMember(F("clientId")).as<const char *>();
  } else {
    clientId = _wlanConf.getMember(F("hostName"));
  }
  const char *user = _mqttConf.getMember(F("user")).as<const char *>();
  const char *password = _mqttConf.getMember(F("password")).as<const char *>();

  DBGLN("Client: %s; User: %s; Password: %s ", clientId, user, password);
  if (this->beforeWaitingMQTTCon != nullptr) {
    this->beforeWaitingMQTTCon();
  }
  while (!mqttCli.connect(clientId, user, password)) {
    DBGLN("_");
    if (this->waitingMQTTCon != nullptr) {
      this->waitingMQTTCon();
    }
    delay(10);
  }
  if (this->afterWaitingMQTTCon != nullptr) {
    this->afterWaitingMQTTCon();
  }
  DBGLN("MQTT connected!");
}

/**
   Disconnects wifi and is putting it on sleep;
*/
void ESPManager::disconnectWifi() {
  DBGLN("disconnectWifi");
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(100);
};

/**
   Subscribes to topics specified in settings.mqtt.topic;
*/
void ESPManager::subscribeCMD() {
  DBGLN("subscribeTopics");
  //topics is not configured, do not subscribe any topics
  if (_mqttConf.getMember(F("topics")).isNull() || !_mqttConf.getMember(F("topics")).is<JsonObject>()) {
    return;
  }
  DBGLN("subscribeCMD");
  JsonObject topics = _mqttConf.getMember(F("topics")).as<JsonObject>();
  const char *cmdTopic = topics.getMember(F("cmd")).as<const char *>();

  if (strlen(cmdTopic) > 0) {
    DBGLN("subscribeTopics: cmd");
    mqttCli.subscribe(cmdTopic, qos);
  }
}

/**
   ---==[ This needs to be call in setup function ]==---
   Loops the entire process;
   It reconnects to MQTT if disconnects;
*/
void ESPManager::loopIt() {
  mqttCli.loop();
  delay(10);

  if (WiFi.status() != WL_CONNECTED || !mqttCli.connected()) {
    DBGLN("Not connected to MQTT reconnect ...");
    reconnect();
  }
  executeTimingOutputEvents();
}

void ESPManager::executeTimingOutputEvents() {
  for (std::map<const char *, outputTimerItem>::iterator it = outputEvents.begin(); it != outputEvents.end(); ++it) {
    const char *key = it->first;
    if (millis() - outputEvents[key].lastTime > outputEvents[key].timing) {
      char *output = outputEvents[key].handler(key);
      outputEvents[key].lastTime = millis();
      if (output != nullptr && strlen(output) > 0) {
        mqttCli.publish(key, output, false, qos);

        // DBG("publishing to topic: ");
        // DBG(key);
        // DBG(" time:");
        // DBG(outputEvents[key].timing);
        // DBG("; output: ");
        // DBGLN(output);
      }
      free(output);
    }
  }
}

/**
   Called for every received message will call the specific function based on the topic
   Will call a specific command, if is coming on command chanel
   Will call saveSettings method if is coming on settings chanel
   param @topic = chanel that has the message;
   param @payload = payload, message
*/
void ESPManager::messageReceived(String &topic, String &payload) {
  if (cmdTopic != nullptr && strcmp(topic.c_str(), cmdTopic) == 0) {
    executeCommands(topic.c_str(), payload.c_str());
  } else if (inputEvents.find(topic.c_str()) != inputEvents.end()) {
    inputEvents[topic.c_str()](payload.c_str());
  } else {
    DBGLN("messageReceived: No method found ===>>> topic: %s; payload: %s", topic.c_str(), payload.c_str());
  }
}

int ESPManager::findCmd(const char *cmd) {
  for (unsigned int i = 0; (i < sizeof(cmdFunctions) / sizeof(cmdFunctions[0])) && strlen(cmdFunctions[i].cmd) > 0; i++) {
    if (strcmp(cmdFunctions[i].cmd, cmd) == 0) {
      return i;
    }
  }
  return -1;
}

void ESPManager::executeCommands(const char *topic, const char *payload) {
  DBGLN("Executing CMD: %s", payload);
  StaticJsonDocument<1500> payloadDoc;
  DeserializationError err = deserializeJson(payloadDoc, payload);
  if (err) {
    DBGLN("Could not deserialie json: %s", err.c_str());
    payloadDoc.clear();
    return;
  }

  const char *cmd = payloadDoc[F("cmd")].as<JsonVariant>().as<char *>();
  JsonVariant params = payloadDoc[F("params")].as<JsonVariant>();
  DBGLN("CMD: %s", cmd);
  // look in registered commands
  if (!commands.empty() && commands.find(cmd) != commands.end()) {
    char *output = commands[cmd](params);
    if (output != nullptr) {
      DBGLN("Response to: %s", cmdTopicResp);
      mqttCli.publish(cmdTopicResp, output, false, qos);
    }
    free(output);
    payloadDoc.clear();
    return;
  }

  // look in internal commands list
  int poz = findCmd(cmd);
  if (poz >= 0 && cmdFunctions[poz].func != nullptr) {
    (this->*cmdFunctions[poz].func)(cmdTopicResp, params);
    payloadDoc.clear();
    return;
  }
}

void ESPManager::addIncomingEventHandler(const char *topic, eventIncomingHandler handler) {
#ifndef DEBUGER
  if (topic == nullptr) {
    DBGLN("To subscribe, topic is mandatory");
  }
#endif
  if (topic == nullptr || strlen(topic) <= 0)
    return;
  DBGLN("addIncomingEventHandler: %s", topic);
  inputEvents[topic] = handler;
  mqttCli.subscribe(topic, qos);
};

void ESPManager::addTimerOutputEventHandler(const char *topic, unsigned long loopTime, outputTimerHandler handler) {
  outputEvents[topic] = {handler, loopTime, 0};
}

void ESPManager::sendMsg(const char *topic, const char *msg, bool retain, int qos) {
  mqttCli.publish(topic, msg, retain, qos);
}

void ESPManager::reconnect() {
  mqttCli.disconnect();
  disconnectWifi();
  createConnections();
}

// ---==[ START Commands ]==---
/**
   CMD: reconnect
   Is disconnecting MQTT client, is disconnecting from WiFi, recreats all connections again;
*/
void ESPManager::cmdReconnect(const char *respTopic, JsonVariant params) {
  DBGLN("cmdReconnect");
  reconnect();
}
/**
   CMD: restart
   Is disconnecting MQTT client, is disconnecting from WiFi, restarts the entire ESP;
*/
void ESPManager::cmdRestart(const char *respTopic, JsonVariant params) {
  DBGLN("cmdRestart");
  mqttCli.disconnect();
  disconnectWifi();
  ESP.restart();
}

/**
   CMD: restart
   Is disconnecting MQTT client, is disconnecting from WiFi, resets the entire ESP;
*/
void ESPManager::cmdReset(const char *respTopic, JsonVariant params) {
  DBGLN("cmdReset");
  mqttCli.disconnect();
  disconnectWifi();
  ESP.reset();
}

/**
 * CMD: status
 * Is seding current online message, if it receives the request then it'a alive; 
 */
void ESPManager::cmdStatus(const char *respTopic, JsonVariant params) {
  DBGLN("Status triggered");
  setOnlineStatusMessage();
}
/**
   CMD: getInfo
   Serializing the settings and submit them in mqtt;
  Sketch uses 335936 bytes (35%) of program storage space. Maximum is 958448 bytes.
  Global variables use 30076 bytes (36%) of dynamic memory, leaving 51844 bytes for local variables. Maximum is 81920 bytes.

*/
void ESPManager::cmdGetInfo(const char *respTopic, JsonVariant params) {
  DBGLN("cmdGetInfo");
  String coreVersion = ESP.getCoreVersion();
  coreVersion.replace("_", ".");

  char skVerBuf[90] = {0};
  if (sketchVersion != nullptr) {
    snprintf_P(skVerBuf, 90, SKETCH_VERSION_PATTERN_P, sketchVersion, ESP.getSketchMD5().c_str());
  }

  char retVal[600] = {0};
  snprintf_P(retVal, 600, INFO_PATTERN_P, WiFi.hostname().c_str(), ESP.getChipId(), WiFi.localIP().toString().c_str(), String(WiFi.macAddress()).c_str(), WiFi.RSSI(), ESP.getResetReason().c_str(), ESP.getFlashChipId(), coreVersion.c_str(),
             ESP.getSdkVersion(), ESP.getVcc() / 1024.00f, ESP.getFlashChipSpeed() / 1000000, ESP.getCycleCount(), ESP.getCpuFreqMHz(), ESP.getFreeHeap(), ESP.getHeapFragmentation(), ESP.getMaxFreeBlockSize(), ESP.getFlashChipSize(), ESP.getSketchSize(),
             ESP.getFreeSketchSpace(), ESP.getFlashChipRealSize(), version, skVerBuf);

  int qos = _mqttConf.getMember(F("qos")).as<int>();
  mqttCli.publish(respTopic, retVal, false, qos);
}

void ESPManager::cmdUpdate(const char *respTopic, JsonVariant params) {
  DBGLN("Update triggered");
  if (params.isNull())
    return;

  const char *type = params[F("type")].as<const char *>();
  String ver = params[F("version")].as<String>();
  String url = params[F("url")].as<String>();

  DBGLN("type: %s, ver: %s, url: %s", type, ver.c_str(), url.c_str());

  ESPhttpUpdate.rebootOnUpdate(true);
  ESPhttpUpdate.setLedPin(LED_BUILTIN, HIGH);
  t_httpUpdate_return ret = HTTP_UPDATE_OK;
  if (strcmp_P(type, UPDATE_SKETCH_P) == 0) {
    DBGLN("updateTYPE");
    WiFiClient updateNetCli;
    ret = ESPhttpUpdate.update(updateNetCli, url, ver);
  } else if (strcmp_P(type, UPDATE_SPIFFS_P) == 0) {
    DBGLN("updateSPIFFS");
    WiFiClient updateNetCli;
    ret = ESPhttpUpdate.updateSpiffs(updateNetCli, url, ver);
  }
  switch (ret) {
  case HTTP_UPDATE_FAILED:
    DBGLN("HTTP_UPDATE_FAILD Error: %i - %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    DBGLN("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    DBGLN("HTTP_UPDATE_OK");
    if (strcmp_P(type, UPDATE_SPIFFS_P) == 0) {
      mqttCli.disconnect();
      disconnectWifi();
      ESP.restart();
    }
    break;
  }
}
// ---==[ END Commands ]==---

ESPManager::~ESPManager() {
  //  delete cbBind;
  delete &mqttCli;
}
