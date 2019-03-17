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

*/
//#define DEBUGER

#ifndef DEBUGER
#define DBGLN(x)
#define DBG(x)
#else 
#define DBGLN(x) Serial.println(x)
#define DBG(x) Serial.print(x)
#endif

#include "ESPManager.h"
ADC_MODE(ADC_VCC);

ESPManager::ESPManager() {
  mqttCli = *new MQTTClient(800);
  wifiMode = WIFI_STA;
  cbBind = new Binding<String &, String &>(*this, &ESPManager::messageReceived);
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
  if (wlanConf.isNull()){
    return INVALID_WLAN_CONF;
  } else if (mqttConf.isNull()) {
    return INVLIAD_MQTT_CONF;
  }
  
  _wlanConf = wlanConf;
  _mqttConf = mqttConf;
  createConnections();
  return CONNECTION_OK;
}

void ESPManager::createConnections() {
  retainMsg = !_mqttConf.getMember(F("retainMessage")).isNull() && _mqttConf.getMember(F("retainMessage")).as<bool>();

  if (!_mqttConf.getMember(F("qos")).isNull() && _mqttConf.getMember(F("qos")).is<int>()) {
    qos = _mqttConf.getMember(F("qos")).as<int>();
  }

  connectToWifi();
  setupMQTT();
  connectToMQTT();

  sendOfflineStatus = !_mqttConf.getMember(F("sendOfflineStatus")).isNull() && _mqttConf.getMember(F("sendOfflineStatus")).as<bool>();
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
  if (WiFi.status() == WL_CONNECTED) {
    DBG("Is already connected: "); DBGLN(WiFi.status());
    return;
  }
  debugWiFiStatus();
  WiFi.mode(wifiMode);
  WiFi.hostname(_wlanConf.getMember(F("hostName")).as<char*>());
  delay(100);
  WiFi.begin(_wlanConf.getMember(F("ssid")).as<char*>(), _wlanConf.getMember(F("password")).as<char*>());
  waitForWiFi();
}

/**
   Wait to be connected in wifi;
*/
void ESPManager::waitForWiFi() {
  int waitingTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - waitingTime  < 30000) {
    DBG("#");
    delay(100);
  }
  DBGLN("");
  if (WiFi.status() != WL_CONNECTED) {
    debugWiFiStatus();
    ESP.restart();
  }
  DBG("IP: "); DBGLN(WiFi.localIP().toString());
}

/**
   Is printing connection status to WiFi if is not connected;
*/
void ESPManager::debugWiFiStatus() {
  int wiFiStatus = WiFi.status();
  if (wiFiStatus != WL_CONNECTED) {
    DBG("WiFi status: "); DBGLN(wiFiStatus);
  }
}

/**
   Is setting-up the MQTT client baaed on settings.mqtt;
*/
void ESPManager::setupMQTT() {
  const char* mqttServer = _mqttConf.getMember(F("server")).as<const char*>();
  int mqttPort = _mqttConf.getMember(F("port")).as<int>();

  DBG("Setting MQTT: "); DBG(mqttServer); DBG("; port: "); DBGLN(mqttPort);
  mqttCli.begin(mqttServer, mqttPort, net);

  mqttCli.onMessage(cbBind->callback); //TODO de implementat

  DBG("SendOfflineStatus: "); DBGLN(sendOfflineStatus);
  if (sendOfflineStatus) {
    setOfflineStatusMessage();
  }

  DBGLN("Finish setupMQTT");
}

/**
   Configuring status message as offline
*/
void ESPManager::setOfflineStatusMessage() {
  const char * topicStatus = _mqttConf.getMember(F("topics")).getMember(F("status")).as<const char*>();
  char msg[100] = {0};

  snprintf_P(msg, 99, STATUS_FORMAT_P, _wlanConf.getMember(F("hostName")).as<char*>(), STATUS_OFFLINE_P);

  DBG("Setting offline message on topic: "); DBG(topicStatus); DBGLN(" content: ");
  DBGLN(msg);

  mqttCli.clearWill();
  mqttCli.setWill(topicStatus, msg, true, 2);
}

/**
   Configuring status message as online and publish it
*/
void ESPManager::setOnlineStatusMessage() {
  const char * topicStatus = _mqttConf.getMember(F("topics")).getMember(F("status")).as<const char*>();
  char msg[100] = {0};

  snprintf(msg, 99, STATUS_FORMAT_P, _wlanConf.getMember(F("hostName")).as<char*>(), STATUS_ONLINE_P);

  DBG("Setting offline message on topic: "); DBG(topicStatus); DBGLN(" content: ");
  DBGLN(msg);

  mqttCli.publish(topicStatus, msg, true, 2);
}

/**
   Connects on MQTT with credentials from settings.mqtt;
*/
void ESPManager::connectToMQTT() {
  DBGLN("Connecting mqtt...");
  const char * clientId = _mqttConf.getMember(F("clientId")).as<const char*>();
  const char * user = _mqttConf.getMember(F("user")).as<const char*>();
  const char * password = _mqttConf.getMember(F("password")).as<const char*>();

  DBG("Client: "); DBG(clientId); DBG("; User: ");  DBG(user); DBG("; Password: "); DBGLN(password);
  while (!mqttCli.connect(clientId, user, password)) {
    DBG("_");
    delay(1000);
  }
  DBGLN("");
  DBGLN("Connected!");
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
  const char * cmdTopic = topics.getMember(F("cmd")).as<const char*>();

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
  //delay(settings.getInt("esp.delayTime"));  // <- fixes some issues with WiFi stability

  if (WiFi.status() != WL_CONNECTED || !mqttCli.connected()) {
    DBGLN("Not connected to MQTT reconnect ...");
    reconnect();
  }
  executeTimingOutputEvents();
}

void ESPManager::executeTimingOutputEvents() {
  for (std::map<const char *, outputTimerItem>::iterator it = outputEvents.begin(); it != outputEvents.end(); ++it) {
    const char * key = it->first;
    if (millis() - outputEvents[key].lastTime > outputEvents[key].timing) {
      const char * output = outputEvents[key].handler(key);
      DBG("publishing to topic: "); DBG(key); DBG("- time:"); DBG(outputEvents[key].timing); DBG("; output: "); DBGLN(output);
      outputEvents[key].lastTime = millis();
      if (strlen(output) > 0) {
        mqttCli.publish(key, output, false,  qos);
      }
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
void ESPManager::messageReceived(String & topic, String & payload) {
  if (executeInteralTopics(topic.c_str(), payload.c_str())) return;
  if (executeRegisteredTopics(topic.c_str(), payload.c_str())) return;
  DBGLN("messageReceived: No method found");
}

int ESPManager::findCmd(const char * cmd) {
  for (int i = 0; (i < sizeof(cmdFunctions) / sizeof(cmdFunctions[0])) && strlen(cmdFunctions[i].cmd) > 0; i++) {
    if (strcmp(cmdFunctions[i].cmd, cmd) == 0) {
      return i;
    }
  }
  return -1;
}

bool ESPManager::executeInteralTopics(const char * topic, const char * payload) {
  JsonVariant jsonTopics = _mqttConf.getMember(F("topics"));

  if (!jsonTopics.isNull() && jsonTopics.is<JsonObject>()) {
    JsonObject topics = jsonTopics.as<JsonObject>();
    const char * cmdTopic = topics.getMember(F("cmd")).as<const char*>();

    if (strcmp(topic, cmdTopic) == 0) {
      DBG("cmd: "); DBG(topic); DBG(" - "); DBGLN(payload);

      const size_t capacity = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 110;
      StaticJsonDocument<capacity> doc;
      DeserializationError err = deserializeJson(doc, payload);
      if (err) {
        DBGLN("Invalid CMD JSON:");
        return false;
      }
      int poz = findCmd(doc[F("cmd")]);
      if (poz >= 0 && cmdFunctions[poz].func != nullptr) {
        JsonVariant params = doc[F("params")].as<JsonVariant>();
        (this->*cmdFunctions[poz].func)(params);
      }
      return true;
    }
  }
  return false;
}

bool ESPManager::executeRegisteredTopics(const char * topic, const char * payload) {
  DBG("executeRegisteredTopics: "); DBG(topic); DBG(" - "); DBGLN(payload);
  if (inputEvents.find(topic) != inputEvents.end() && inputEvents[topic] != nullptr)
    inputEvents[topic](payload);
  return true;
}

void ESPManager::addIncomingEventHandler(const char * topic, eventIncomingHandler handler) {
#ifndef DEBUG_SERIAL
  if (topic == nullptr) {
    DBGLN("To subscribe, topic is mandatory");
  }
#endif
  if (topic == nullptr || strlen(topic) <= 0 ) return;
  DBG("addIncomingEventHandler: "); DBGLN(topic);
  inputEvents[topic] = handler;
  mqttCli.subscribe(topic, qos);
};

void ESPManager::addTimerOutputEventHandler(const char * topic, long loopTime, outputTimerHandler handler) {
  outputEvents[topic] = {handler, loopTime, 0};
}

void ESPManager::sendMsg(const char * topic, const char * msg, bool retain, int qos) {
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
void ESPManager::cmdReconnect(JsonVariant params) {
  DBGLN("cmdReconnect");
  reconnect();
}

/**
   CMD: restart
   Is disconnecting MQTT client, is disconnecting from WiFi, restarts the entire ESP;
*/
void ESPManager::cmdRestart(JsonVariant params) {
  DBGLN("cmdRestart");
  mqttCli.disconnect();
  disconnectWifi();
  ESP.restart();
}

/**
   CMD: restart
   Is disconnecting MQTT client, is disconnecting from WiFi, resets the entire ESP;
*/
void ESPManager::cmdReset(JsonVariant params) {
  DBGLN("cmdReset");
  mqttCli.disconnect();
  disconnectWifi();
  ESP.reset();
}

/**
   CMD: getInfo
   Serializing the settings and submit them in mqtt;
  Sketch uses 335936 bytes (35%) of program storage space. Maximum is 958448 bytes.
  Global variables use 30076 bytes (36%) of dynamic memory, leaving 51844 bytes for local variables. Maximum is 81920 bytes.

*/
void ESPManager::cmdGetInfo(JsonVariant params) {
  DBGLN("cmdGetInfo");
  String coreVersion = ESP.getCoreVersion();
  coreVersion.replace("_", ".");

  char skVerBuf[30] = {0};
  if (sketchVersion != nullptr) {
    snprintf_P(skVerBuf, 30, SKETCH_VERSION_PATTERN_P, sketchVersion);
  }
  
  char retVal[500] = {0};
  snprintf_P(retVal, 500, INFO_PATTERN_P, ESP.getChipId(), WiFi.localIP().toString().c_str(), String(WiFi.macAddress()).c_str(), ESP.getResetReason().c_str(), ESP.getFlashChipId(), coreVersion.c_str(),
             ESP.getSdkVersion(), ESP.getVcc() / 1024.00f, ESP.getFlashChipSpeed() / 1000000, ESP.getCycleCount(), ESP.getCpuFreqMHz(), ESP.getFreeHeap(), ESP.getFlashChipSize(), ESP.getSketchSize(),
             ESP.getFreeSketchSpace(), ESP.getFlashChipRealSize(), version, skVerBuf);

  const char * cmdTopic = _mqttConf.getMember(F("topics")).getMember(F("cmd")).as<const char*>();
  int qos = _mqttConf.getMember(F("qos")).as<int>();

  char topic[100] = {0};
  strcat(topic, cmdTopic);
  strcat(topic, "/resp");

  mqttCli.publish(topic, retVal, false, qos);
}

void ESPManager::cmdUpdate(JsonVariant params) {
  DBGLN("Update triggered");
  if (params.isNull()) return;

  const char * type = params[F("type")].as<const char *>();
  const char * ver = params[F("version")].as<const char *>();
  const char * url = params[F("url")].as<const char *>();
  
  DBG("type: "); DBGLN(type);
  DBG("ver: "); DBGLN(ver);
  DBG("url: "); DBGLN(url);

  ESPhttpUpdate.setLedPin(LED_BUILTIN, HIGH);
  t_httpUpdate_return ret = HTTP_UPDATE_OK;
  if (strcmp_P(type, UPDATE_SKETCH_P) == 0) {
    DBGLN("updateTYPE");
    ret = ESPhttpUpdate.update(net, url, ver);
  } else if (strcmp_P(type, UPDATE_SPIFFS_P) == 0) {
    DBGLN("updateSPIFFS");
    ret = ESPhttpUpdate.updateSpiffs(net, url, ver);
  }
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      DBG("HTTP_UPDATE_FAILD Error: "); DBG(ESPhttpUpdate.getLastError()); DBG(" - "); DBGLN(ESPhttpUpdate.getLastErrorString());
      DBGLN();
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
