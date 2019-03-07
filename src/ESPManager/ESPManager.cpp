#define DBGLN(x) Serial.println(x)
#define DBG(x) Serial.print(x)


#include "ESPManager.h"
ADC_MODE(ADC_VCC);
/**
   param @fileName the name/path of the settings file;
   param @qos the quality of service level;
*/
ESPManager::ESPManager() {
  mqttCli = *new MQTTClient(800);
  wifiMode = WIFI_STA;
  cbBind = new Binding<String &, String &>(*this, &ESPManager::messageReceived);
  //  DBGLN("Last restart reson: " + ESP.getResetReason());
}

/**
   ---==[ This needs to be call in setup function ]==---
   Creates connection to WiFi;
   Set-up MQTT;
   Wait to be connected to WiFi
   Connects to MQTT server
   Subscribe to MQTT topics set in config
*/
void ESPManager::createConnections(JsonObject wlanConf, JsonObject mqttConf) {
  _wlanConf = wlanConf;
  _mqttConf = mqttConf;
  createConnections();
}

void ESPManager::createConnections() {
  sendOfflineStatus = _mqttConf.getMember(F("sendOfflineStatus")).as<bool>();
  retainMsg = _mqttConf.getMember(("retainMessage")).as<bool>();

  if (!_mqttConf.getMember(F("qos")).isNull() && _mqttConf.getMember(F("qos")).is<int>()) {
    qos = _mqttConf.getMember(F("qos")).as<int>();
  }

  connectToWifi();
  setupMQTT();
  connectToMQTT();
  if (sendOfflineStatus) {
    setOnlineStatusMessage();
  }
  subscribeTopics();
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
  WiFi.begin(_wlanConf.getMember("ssid").as<char*>(), _wlanConf.getMember("password").as<char*>());
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
  const char* mqttServer = _mqttConf.getMember("server").as<const char*>();
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

// ---==[ START Commands ]==---
/**
   CMD: reconnect
   Is disconnecting MQTT client, is disconnecting from WiFi, recreats all connections again;
*/
void ESPManager::cmdReconnect(const char * payload) {
  DBGLN("cmdReconnect");
  mqttCli.disconnect();
  disconnectWifi();
  createConnections();
}

///** // maybe is better if get this outside of manager not having the entire configuration file
//   CMD: config
//   Is publising all the settings on cmd channel;
//*/
//void ESPManager::cmdConfig(const char * payload) {
//  //delay(10);
//  auto allSettings = settings.getSettings();
//  mqttCli.publish(replacePlaceHolders(settings.getString("mqtt.topic.cmd")) + "/resp", SettingsManager::stringify(allSettings), false, qos);
//}

/**
   CMD: restart
   Is disconnecting MQTT client, is disconnecting from WiFi, restarts the entire ESP;
*/
void ESPManager::cmdRestart(const char * payload) {
  DBGLN("cmdRestart");
  mqttCli.disconnect();
  disconnectWifi();
  ESP.restart();
}

/**
   CMD: restart
   Is disconnecting MQTT client, is disconnecting from WiFi, resets the entire ESP;
*/
void ESPManager::cmdReset(const char * payload) {
  DBGLN("cmdReset");
  mqttCli.disconnect();
  disconnectWifi();
  ESP.reset();
}

/**
   CMD: getInfo
   Serializing the settings and submit them in mqtt;
*/
void ESPManager::cmdGetInfo(const char * payload) {//TODO move to char *
  DBGLN("cmdGetInfo");
  String coreVersion = ESP.getCoreVersion();
  coreVersion.replace("_", ".");
  String retVal = "{ \"chipId\": " + String(ESP.getChipId()) +
                  ", \"localIP\" : \"" + WiFi.localIP().toString() + "\"" +
                  ", \"macAddress\" : \"" + String(WiFi.macAddress()) + "\"" +
                  ", \"lastRestartReson\" : \"" + ESP.getResetReason() + "\"" +
                  ", \"flashChipId\" : " + String(ESP.getFlashChipId()) +
                  ", \"coreVersion\" : \"" + coreVersion + "\"" +
                  ", \"sdkVersion\" : \"" + ESP.getSdkVersion() + "\"" +
                  ", \"vcc\" : " + String(ESP.getVcc() / 1024.00f) +
                  ", \"flashChipSpeed\" : " + String(ESP.getFlashChipSpeed()) +
                  ", \"cycleCount\" : " + String(ESP.getCycleCount()) +
                  ", \"cpuFreq\" : " + String(ESP.getCpuFreqMHz() * 1000000) +
                  ", \"freeHeap\": " + String(ESP.getFreeHeap()) +
                  ", \"flashChipSize\" : " + String(ESP.getFlashChipSize()) +
                  ", \"sketchSize\" : " + String(ESP.getSketchSize()) + "" +
                  ", \"freeSketchSpace\" : " + String(ESP.getFreeSketchSpace()) + "" +
                  ", \"flashChipRealSize\" : " + String(ESP.getFlashChipRealSize()) + "" +
                  ", \"espManagerVersion\" : \"" + getVersion() + "\"" +
                  //                  ", \"sketchVersion\" : \"" + settings.getString("sketchVersion") + "\"" +
                  "}";
  const char * cmdTopic = _mqttConf.getMember(F("topics")).getMember(F("cmd")).as<const char*>();
  int qos = _mqttConf.getMember(F("qos")).as<int>();
  char topic[100] = {0};

  strcat(topic, cmdTopic);
  strcat(topic, "/resp");
  mqttCli.publish(topic, retVal, false, qos);
}

// ---==[ END Commands ]==---

/**
   Subscribes to topics specified in settings.mqtt.topic;
*/
void ESPManager::subscribeTopics() {
  DBGLN("subscribeTopics");
  //topics is not configured, do not subscribe any topics
  if (_mqttConf.getMember(F("topics")).isNull() || !_mqttConf.getMember(F("topics")).is<JsonObject>()) {
    return;
  }
  DBGLN("subscribeTopics: foundSettings");
  JsonObject topics = _mqttConf.getMember(F("topics")).as<JsonObject>();

  const char * cmdTopic = topics.getMember(F("cmd")).as<const char*>();
  const char * settingsTopic = topics.getMember(F("settings")).as<const char*>();
  const char * updateTopic = topics.getMember(F("update")).as<const char*>();

  if (strlen(cmdTopic) > 0) {
    DBGLN("subscribeTopics: cmd");
    mqttCli.subscribe(cmdTopic, qos);
  }

  if (strlen(settingsTopic) > 0) {
    DBGLN("subscribeTopics: settings");
    mqttCli.subscribe(settingsTopic, qos);
  }

  if (strlen(updateTopic) > 0) {
    DBGLN("subscribeTopics: update");
    mqttCli.subscribe(updateTopic, qos);
  }

  //  for (std::map<String, eventHandler>::iterator it = inputEvents.begin(); it != inputEvents.end(); ++it) {
  //    String key = it->first;
  //    DBGLN("Subscribing to topic: " + key);
  //    mqttCli.subscribe(key, qos);
  //  }
}

/**
   ---==[ This needs to be call in setup function ]==---
   Loops the entire process;
   It reconnects to MQTT if disconnects;
*/
void ESPManager::loopIt() {
  mqttCli.loop();
  delay(300);
  //delay(settings.getInt("esp.delayTime"));  // <- fixes some issues with WiFi stability

  if (WiFi.status() != WL_CONNECTED || !mqttCli.connected()) {
    DBGLN("Not connected to MQTT reconnect ...");
    cmdReconnect("");
  }

  //  for (std::map<String, outputTimerHandler>::iterator it = outputEvents.begin(); it != outputEvents.end(); ++it) {
  //    String key = it->first;
  //    if (millis() - outputEvents[key].lastTime > outputEvents[key].timing) {
  //      String output = outputEvents[key].handler(key, settings);
  //      DBGLN("publishing to topic: " + key + "- time:" + outputEvents[key].timing + "; output: " + output );
  //      output.trim();
  //      outputEvents[key].lastTime = millis();
  //      if (output.length() > 0) {
  //        mqttCli.publish(key, output, false,  qos);
  //      }
  //    }
  //  }
}

/**
   Called for every received message will call the specific function based on the topic
   Will call a specific command, if is coming on command chanel
   Will call saveSettings method if is coming on settings chanel
   param @topic = chanel that has the message;
   param @payload = payload, message
*/
void ESPManager::messageReceived(String & topic, String & payload) {
  DBG("Incoming: "); DBG(topic); DBG(" - "); DBGLN(payload);

  if (executeCMDInteralTopics(topic.c_str(), payload.c_str())) return;
  if (executeRegisteredTopics(topic.c_str(), payload.c_str())) return;

  DBG("No method found");
}

int ESPManager::findCmd(const char * cmd) {
  for (int i = 0; (i < sizeof(cmdFunctions) / sizeof(cmdFunctions[0])) && strlen(cmdFunctions[i].cmd) > 0; i++) {
    if (strcmp(cmdFunctions[i].cmd, cmd) == 0) {
      return i;
    }
  }
  return -1;
}

bool ESPManager::executeCMDInteralTopics(const char * topic, const char * payload) {
  DBG("executeInteralTopics: "); DBG(topic); DBG(" - "); DBGLN(payload);

  JsonVariant jsonTopics = _mqttConf.getMember(F("topics"));

  if (!jsonTopics.isNull() && jsonTopics.is<JsonObject>()) {
    JsonObject topics = jsonTopics.as<JsonObject>();
    const char * cmdTopic = topics.getMember(F("cmd")).as<const char*>();
    const char * settingsTopic = topics.getMember(F("settings")).as<const char*>();
    const char * updateTopic = topics.getMember(F("update")).as<const char*>();

    if (strcmp(topic, cmdTopic) == 0) {
      DBGLN("cmd");
      int poz = findCmd(payload);
      if (poz >= 0 && cmdFunctions[poz].func != nullptr) {
        (this->*cmdFunctions[poz].func)(payload);
      } else if (poz >= 0 && cmdFunctions[poz].customFunc != nullptr) {
        cmdFunctions[poz].customFunc(payload);
      }
      return true;
    } else if (strcmp(topic, settingsTopic) == 0) {
      //      saveSettings(payload); //TODO to implement
      return true;
    } else if (topic == updateTopic) {
      //      updateEsp(payload); //TODO to implement
      return true;
    }
  }
  return false;
}

bool ESPManager::executeRegisteredTopics(const char * topic, const char * payload) {
  DBG("executeRegisteredTopics: "); DBG(topic); DBG(" - "); DBGLN(payload);
  return false;
  //  if (inputEvents.find(topic) != inputEvents.end()) {
  //    eventHandler topicListener = inputEvents[topic];
  //    topicListener(payload, settings);
  //  }
}

void ESPManager::addIncomingEventHandler(const char * topic, eventHandler handler) {
#ifndef DEBUG_SERIAL
  if (topic != nullptr) {
    DBGLN("To subscribe, topic is mandatory");
  }
#endif
  inputEvents[topic] = handler;
};
//
//void ESPManager::addOutputEventHandler(String topic, long loopTime, outputHandlerType handler) {
//  outputEvents[replacePlaceHolders(topic)] = {handler, loopTime};
//};
//

//
///**
//    --------========[       Commands       ]========--------
//*/
///**
//   Save settings command;
//   Takes the payload and is saving it in settings file
//*/
//void ESPManager::saveSettings(String payload) {
//  payload.trim();
//  settings.writeSettings(settingsFileName, payload);
//}
//
///**
//   Serializing the settings and submit them in mqtt;
//*/
//void ESPManager::updateEsp(String payload) {
//  DBG("Update triggered");
//  String type = payload.substring(0, payload.indexOf(","));
//  String ver = payload.substring(payload.indexOf(",") + 1);
//  String updateLink = replacePlaceHolders(settings.getString("updateServer"));
//  t_httpUpdate_return ret;
//  if (type == "sketch") {
//    ret = ESPhttpUpdate.update(updateLink, ver);
//  } else if (type == "spiffs") {
//    ret = ESPhttpUpdate.updateSpiffs(updateLink, ver);
//  }
//  switch (ret) {
//    case HTTP_UPDATE_FAILED:
//      DBGLN("HTTP_UPDATE_FAILD Error: " + String(ESPhttpUpdate.getLastError()) + " - " + ESPhttpUpdate.getLastErrorString());
//      DBGLN();
//      break;
//
//    case HTTP_UPDATE_NO_UPDATES:
//      DBGLN("HTTP_UPDATE_NO_UPDATES");
//      break;
//
//    case HTTP_UPDATE_OK:
//      DBGLN("HTTP_UPDATE_OK");
//      if (type == "spiffs") {
//        mqttCli.disconnect();
//        disconnectWifi();
//        ESP.restart();
//      }
//      break;
//  }
//}





ESPManager::~ESPManager() {
  //  delete cbBind;
  delete &mqttCli;
}
