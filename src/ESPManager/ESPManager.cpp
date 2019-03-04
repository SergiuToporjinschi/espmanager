#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)


#include "ESPManager.h"
ADC_MODE(ADC_VCC);
/**
   param @fileName the name/path of the settings file;
   param @qos the quality of service level;
*/
ESPManager::ESPManager(JsonObject wlanConf, JsonObject mqttConf) : _wlanConf(wlanConf), _mqttConf(mqttConf){
  //  cbBind = new Binding<String &, String &>(*this, &ESPManager::messageReceived);
  //  mqttCli = *new MQTTClient(800);
  //  DEBUG_PRINTLN("Last restart reson: " + ESP.getResetReason());
  //  wifiMode = WIFI_STA;
  //  commands["reconnect"] = &ESPManager::cmdReconnect;
  //  commands["config"] = &ESPManager::cmdConfig;
  //  commands["restart"] = &ESPManager::cmdRestart;
  //  commands["reset"] = &ESPManager::cmdReset;
  //  commands["getInfo"] = &ESPManager::getInfo;
  //  readSettings();
}
//String ESPManager::getVersion() {
//  return version;
//}
///**
//   ---==[ This needs to be call in setup function ]==---
//   Creates connection to WiFi;
//   Set-up MQTT;
//   Wait to be connected to WiFi
//   Connects to MQTT server
//   Subscribe to MQTT topics set in config
//
//*/
//void ESPManager::createConnections() {
//  connectToWifi();
//  setupMQTT();
//  if (sendStatus) {
//    setOfflineStatusMessage(settings.getString("mqtt.status.format"));
//  }
//  connectToMQTT();
//  if (sendStatus) {
//    setOnlineStatusMessage(settings.getString("mqtt.status.format"));
//  }
//  subscribeTopics();
//}
//
///**
//   ---==[ This needs to be call in setup function ]==---
//   Loops the entire process;
//   It reconnects to MQTT if disconnects;
//*/
//void ESPManager::loopIt() {
//  mqttCli.loop();
//  delay(settings.getInt("esp.delayTime"));  // <- fixes some issues with WiFi stability
//
//  if (WiFi.status() != WL_CONNECTED || !mqttCli.connected()) {
//    DEBUG_PRINTLN("Not connected to MQTT reconnect ...");
//    cmdReconnect("");
//  }
//
//  for (std::map<String, outputTimerHandler>::iterator it = outputEvents.begin(); it != outputEvents.end(); ++it) {
//    String key = it->first;
//    if (millis() - outputEvents[key].lastTime > outputEvents[key].timing) {
//      String output = outputEvents[key].handler(key, settings);
//      DEBUG_PRINTLN("publishing to topic: " + key + "- time:" + outputEvents[key].timing + "; output: " + output );
//      output.trim();
//      outputEvents[key].lastTime = millis();
//      if (output.length() > 0) {
//        mqttCli.publish(key, output, false,  qos);
//      }
//    }
//  }
//}
//
///**
//   Create connection to WiFi based on settings.wlan if curent status is not connected and waits for connection to be made;
//*/
//void ESPManager::connectToWifi() {
//  DEBUG_PRINTLN("Connecting WiFi...");
//  if (WiFi.status() == WL_CONNECTED) {
//    DEBUG_PRINTLN("Is already connected: " + String(WiFi.status()));
//    return;
//  }
//  debugWiFiStatus();
//  WiFi.mode(wifiMode);
//  WiFi.hostname(settings.getString("wlan.hostName"));
//  delay(100);
//  WiFi.begin(settings.getChar("wlan.ssid"), settings.getChar("wlan.password"));
//  waitForWiFi();
//}
//
///**
//   Wait to be connected in wifi;
//*/
//void ESPManager::waitForWiFi() {
//  int waitingTime = millis();
//  while (WiFi.status() != WL_CONNECTED && millis() - waitingTime  < 30000) {
//    DEBUG_PRINT("#");
//    delay(100);
//  }
//  DEBUG_PRINTLN("");
//  if (WiFi.status() != WL_CONNECTED) {
//    debugWiFiStatus();
//    ESP.restart();
//  }
//}
//
///**
//   Is printing connection status to WiFi if is not connected;
//*/
//void ESPManager::debugWiFiStatus() {
//  int wiFiStatus = WiFi.status();
//  if (wiFiStatus != WL_CONNECTED) {
//    DEBUG_PRINTLN("WiFi status: " +  wiFiStatus);
//  }
//}
//
///**
//   Is setting-up the MQTT client baaed on settings.mqtt;
//*/
//void ESPManager::setupMQTT() {
//  DEBUG_PRINTLN("Setting MQTT: " + String(settings.getChar("mqtt.server")) + "; port: " + String(settings.getInt("mqtt.port")));
//  mqttCli.begin(settings.getChar("mqtt.server"), settings.getInt("mqtt.port"), net);
//  mqttCli.onMessage(cbBind->callback);
//  DEBUG_PRINTLN("Finish setupMQTT");
//}
//
///**
//   Connects on MQTT with credentials from settings.mqtt;
//*/
//void ESPManager::connectToMQTT() {
//  DEBUG_PRINTLN("Connecting mqtt...");
//  DEBUG_PRINTLN("Client: " + String(settings.getChar("mqtt.clientId")) + "; User: " + String(settings.getChar("mqtt.user")) + "; Password: " + String(settings.getChar("mqtt.password")));
//  while (!mqttCli.connect(settings.getChar("mqtt.clientId"), settings.getChar("mqtt.user"), settings.getChar("mqtt.password"))) {
//    DEBUG_PRINT("_");
//    delay(1000);
//  }
//  DEBUG_PRINTLN("");
//  DEBUG_PRINTLN("Connected!");
//}
//
///**
//   Configuring status message as offline
//*/
//void ESPManager::setOfflineStatusMessage(String messageFormat) {
//  static String top = replacePlaceHolders(settings.getString("mqtt.topic.status"));
//  static String msg = replacePlaceHolders(messageFormat);
//  msg.replace("<MQTTStatus>", "offline");
//  DEBUG_PRINTLN("Setting offline message on topic: " + top + " content: " + msg);
//  mqttCli.clearWill();
//  mqttCli.setWill(top.c_str(), msg.c_str(), true, qos);
//}
//
///**
//   Configuring status message as online and publish it
//*/
//void ESPManager::setOnlineStatusMessage(String messageFormat) {
//  String topic = replacePlaceHolders(settings.getString("mqtt.topic.status"));
//  messageFormat = replacePlaceHolders(messageFormat);
//  messageFormat.replace("<MQTTStatus>", "online");
//  DEBUG_PRINTLN("Setting online message on topic: " + topic + " content: " + messageFormat);
//  mqttCli.publish(topic, messageFormat, true, qos);
//}
//
///**
//   Subscribes to topics specified in settings.mqtt.topic;
//*/
//void ESPManager::subscribeTopics() {
//  String cmdTopic = replacePlaceHolders(settings.getString("mqtt.topic.cmd"));
//  String settingsTopic = replacePlaceHolders(settings.getString("mqtt.topic.settings"));
//  String updateTopic = replacePlaceHolders(settings.getString("mqtt.topic.update"));
//
//  if (cmdTopic.length() > 0) {
//    mqttCli.subscribe(cmdTopic, qos);
//  };
//  if (settingsTopic.length() > 0) {
//    mqttCli.subscribe(settingsTopic, qos);
//  };
//  if (updateTopic.length() > 0) {
//    mqttCli.subscribe(updateTopic, qos);
//  };
//  for (std::map<String, eventHandler>::iterator it = inputEvents.begin(); it != inputEvents.end(); ++it) {
//    String key = it->first;
//    DEBUG_PRINTLN("Subscribing to topic: " + key);
//    mqttCli.subscribe(key, qos);
//  }
//}
//
///**
//   Called for every received message will call the specific function based on the topic
//   Will call a specific command, if is coming on command chanel
//   Will call saveSettings method if is coming on settings chanel
//   param @topic = chanel that has the message;
//   param @payload = payload, message
//*/
//void ESPManager::messageReceived(String &topic, String &payload) {
//  DEBUG_PRINTLN("Incoming: " + topic + " - " + payload);
//  Serial.flush();
//  String cmdTopic = replacePlaceHolders(settings.getString("mqtt.topic.cmd"));
//  String settingsTopic = replacePlaceHolders(settings.getString("mqtt.topic.settings"));
//  String updateTopic = replacePlaceHolders(settings.getString("mqtt.topic.update"));
//  if (topic == cmdTopic) {
//    (this->*commands[payload])(payload);
//  } else if (topic == settingsTopic) {
//    saveSettings(payload);
//  } else if (topic == updateTopic) {
//    updateEsp(payload);
//  } else if (inputEvents.find(topic) != inputEvents.end()) {
//    eventHandler topicListener = inputEvents[topic];
//    topicListener(payload, settings);
//  }
//}
//
//String ESPManager::replacePlaceHolders(String stringToReplace) {
//  String hostName = settings.getString("wlan.hostName");
//  stringToReplace.replace("<hostName>", hostName);
//  return stringToReplace;
//};
///**
//   Reads settings from settings file;
//*/
//void ESPManager::readSettings() {
//  settings.readSettings(settingsFileName);
//}
//
//String ESPManager::getStrSetting(String property) {
//  return settings.getString(property);
//};
//
//int ESPManager::getIntSetting(String property) {
//  return settings.getInt(property);
//};
//
//long ESPManager::getLongSetting(String property) {
//  return settings.getLong(property);
//};
//
//void ESPManager::addInputEventHandler(String topic, eventHandler handler) {
//#ifndef DEBUG_SERIAL
//  if (topic.length() <= 0) {
//    DEBUG_PRINTLN("To subscribe, topic is mandatory");
//  }
//#endif
//  inputEvents[replacePlaceHolders(topic)] = handler;
//};
//
//void ESPManager::addOutputEventHandler(String topic, long loopTime, outputHandlerType handler) {
//  outputEvents[replacePlaceHolders(topic)] = {handler, loopTime};
//};
//
///**
//   Disconnects wifi and is putting it on sleep;
//*/
//void ESPManager::disconnectWifi() {
//  WiFi.disconnect();
//  WiFi.mode(WIFI_OFF);
//  WiFi.forceSleepBegin();
//};
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
//  DEBUG_PRINT("Update triggered");
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
//      DEBUG_PRINTLN("HTTP_UPDATE_FAILD Error: " + String(ESPhttpUpdate.getLastError()) + " - " + ESPhttpUpdate.getLastErrorString());
//      DEBUG_PRINTLN();
//      break;
//
//    case HTTP_UPDATE_NO_UPDATES:
//      DEBUG_PRINTLN("HTTP_UPDATE_NO_UPDATES");
//      break;
//
//    case HTTP_UPDATE_OK:
//      DEBUG_PRINTLN("HTTP_UPDATE_OK");
//      if (type == "spiffs") {
//        mqttCli.disconnect();
//        disconnectWifi();
//        ESP.restart();
//      }
//      break;
//  }
//}
//
///**
//   CMD: reconnect
//   Is disconnecting MQTT client, is disconnecting from WiFi, recreats all connections again;
//*/
//void ESPManager::cmdReconnect(String payload) {
//  mqttCli.disconnect();
//  disconnectWifi();
//  delay(100);
//  createConnections();
//}
//
//
///**
//   CMD: config
//   Is publising all the settings on cmd channel;
//*/
//void ESPManager::cmdConfig(String payload) {
//  delay(10);
//  auto allSettings = settings.getSettings();
//  mqttCli.publish(replacePlaceHolders(settings.getString("mqtt.topic.cmd")) + "/resp", SettingsManager::stringify(allSettings), false, qos);
//}
//
///**
//   CMD: restart
//   Is disconnecting MQTT client, is disconnecting from WiFi, restarts the entire ESP;
//*/
//void ESPManager::cmdRestart(String payload) {
//  mqttCli.disconnect();
//  disconnectWifi();
//  ESP.restart();
//}
//
///**
//   CMD: restart
//   Is disconnecting MQTT client, is disconnecting from WiFi, resets the entire ESP;
//*/
//void ESPManager::cmdReset(String payload) {
//  mqttCli.disconnect();
//  disconnectWifi();
//  ESP.reset();
//}
//
///**
//   CMD: getInfo
//   Serializing the settings and submit them in mqtt;
//*/
//void ESPManager::getInfo(String payload) {
//  String coreVersion = ESP.getCoreVersion();
//  coreVersion.replace("_", ".");
//  String retVal = "{ \"chipId\": " + String(ESP.getChipId()) +
//                  ", \"localIP\" : \"" + WiFi.localIP().toString() + "\"" +
//                  ", \"macAddress\" : \"" + String(WiFi.macAddress()) + "\"" +
//                  ", \"lastRestartReson\" : \"" + ESP.getResetReason() + "\"" +
//                  ", \"flashChipId\" : " + String(ESP.getFlashChipId()) +
//                  ", \"coreVersion\" : \"" + coreVersion + "\"" +
//                  ", \"sdkVersion\" : \"" + ESP.getSdkVersion() + "\"" +
//                  ", \"vcc\" : " + String(ESP.getVcc() / 1024.00f) +
//                  ", \"flashChipSpeed\" : " + String(ESP.getFlashChipSpeed()) +
//                  ", \"cycleCount\" : " + String(ESP.getCycleCount()) +
//                  ", \"cpuFreq\" : " + String(ESP.getCpuFreqMHz() * 1000000) +
//                  ", \"freeHeap\": " + String(ESP.getFreeHeap()) +
//                  ", \"flashChipSize\" : " + String(ESP.getFlashChipSize()) +
//                  ", \"sketchSize\" : " + String(ESP.getSketchSize()) + "" +
//                  ", \"freeSketchSpace\" : " + String(ESP.getFreeSketchSpace()) + "" +
//                  ", \"flashChipRealSize\" : " + String(ESP.getFlashChipRealSize()) + "" +
//                  ", \"espManagerVersion\" : \"" + getVersion() + "\"" +
//                  ", \"sketchVersion\" : \"" + settings.getString("sketchVersion") + "\"" +
//                  "}";
//  mqttCli.publish(replacePlaceHolders(settings.getString("mqtt.topic.cmd")) + "/resp", retVal, false, qos);
//}

ESPManager::~ESPManager() {
//  delete cbBind;
//  delete &mqttCli;
}
