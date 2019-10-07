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

#ifndef ESPManager_h
#define ESPManager_h
#include "Macro.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <MQTTClient.h>
#include <functional>
#include <map>
#include <pgmspace.h>
#include <stdlib.h>

#ifdef EM_UDP_DEBUG
#  include <WiFiUdp.h>
static const char DEUBG_UDP_MASK_P[] PROGMEM = "{\"Exception\":%d,\"flag\":%d,\"flagText\":\"%s\",\"epc1\":\"0x%08x\",\"epc2\":\"0x%08x\",\"epc3\":\"0x%08x\",\"excvaddr\":\"0x%08x\",\"depc\":\"0x%08x\"}";
#endif

static const char STATUS_FORMAT_P[] PROGMEM = "{\"name\":\"%s\", \"status\":\"%s\"}";
static const char STATUS_ONLINE_P[] PROGMEM = "online";
static const char STATUS_OFFLINE_P[] PROGMEM = "offline";

static const char UPDATE_SKETCH_P[] PROGMEM = "sketch";
static const char UPDATE_SPIFFS_P[] PROGMEM = "spiffs";

static const char SKETCH_VERSION_PATTERN_P[] = ",\"sketchVersion\":\"%s\",\"sketchMD5\":\"%s\" ";
static const char INFO_PATTERN_P[] PROGMEM = "{\"hostName\":\"%s\",\"chipId\":%i,\"localIP\":\"%s\",\"macAddress\":\"%s\",\"RSSI\":%i,\"lastRestartReson\":\"%s\",\"flashChipId\":%u,\"coreVersion\":\"%s\",\"sdkVersion\":\"%s\",\"vcc\":%1.2f,\"flashChipSpeed\":%u,\"cycleCount\":%u,\"cpuFreq\":%u,\"freeHeap\":%u,\"heapFrag\":%i,\"maxFreeBlockSize\":%i,\"flashChipSize\":%u,\"sketchSize\":%u,\"freeSketchSpace\":%u,\"flashChipRealSize\":%u,\"espManagerVersion\":\"%s\"%s}";

enum ESPManConnStatus {
  CONNECTION_OK,
  INVALID_WLAN_CONF,
  INVLIAD_MQTT_CONF
};

template <class... params>
class Binding;
class ESPManager {
 public:
  using eventIncomingHandler = std::function<void(const char *)>;
  using outputTimerHandler = std::function<char *(const char *)>;
  using cmdFunction = std::function<char *(JsonVariant)>;

  ESPManager();
  ~ESPManager();
  ESPManConnStatus createConnections(JsonObject wlanConf, JsonObject mqttConf);
  void loopIt();
  void setSketchVersion(String ver) {
    sketchVersion = ver.c_str();
  };
  void setSketchVersion(const char *ver) {
    sketchVersion = ver;
  };

  void addIncomingEventHandler(const char *topic, eventIncomingHandler handler);
  void addIncomingEventHandler(const String topic, eventIncomingHandler handler) {
    addIncomingEventHandler(topic.c_str(), handler);
  }
  void addTimerOutputEventHandler(const char *topic, unsigned long loopTime, outputTimerHandler handler);
  void addTimerOutputEventHandler(const String topic, unsigned long loopTime, outputTimerHandler handler) {
    addTimerOutputEventHandler(topic.c_str(), loopTime, handler);
  }
  void sendMsg(const String topic, const String msg) {
    sendMsg(topic, msg, false, qos);
  };
  void sendMsg(const String topic, const String msg, bool retain, int qos) {
    sendMsg(topic.c_str(), msg.c_str(), retain, qos);
  };
  void sendMsg(const char *topic, const char *msg) {
    sendMsg(topic, msg, false, qos);
  };
  void sendMsg(const char *topic, const char *msg, bool retain, int qos);

  void onBeforeWaitingWiFiCon(std::function<void()> func) { this->beforeWaitingWiFiCon = func; };
  void onWaitingWiFiCon(std::function<void()> func) { this->waitingWiFiCon = func; };
  void onAfterWaitingWiFiCon(std::function<void()> func) { this->afterWaitingWiFiCon = func; };

  void onBeforeWaitingMQTTCon(std::function<void()> func) { this->beforeWaitingMQTTCon = func; };
  void onWaitingMQTTCon(std::function<void()> func) { this->waitingMQTTCon = func; };
  void onAfterWaitingMQTTCon(std::function<void()> func) { this->afterWaitingMQTTCon = func; };

  void addCommand(const char *cmd, cmdFunction handler);

 private:
  const char *version = VER;
  const char *sketchVersion = nullptr;
  bool retainMsg = false;
  int qos = 0;
#ifdef EM_UDP_DEBUG
  WiFiUDP Udp;
  IPAddress udpDebugIP;
  uint16_t udpDebugPort;
#endif

  std::function<void()> beforeWaitingWiFiCon;
  std::function<void()> waitingWiFiCon;
  std::function<void()> afterWaitingWiFiCon;

  std::function<void()> beforeWaitingMQTTCon;
  std::function<void()> waitingMQTTCon;
  std::function<void()> afterWaitingMQTTCon;

  WiFiClient mqttNetClient; //TCP client for MQTT
  JsonObject _wlanConf;     //WLAN settings
  JsonObject _mqttConf;     //MQTT settings
  MQTTClient mqttCli;       //MQTT client engine
  WiFiMode wifiMode;        //WiFi Engine

  bool sendOfflineStatus;         //Sends a retain message for registering stauts
  const char *cmdTopic = nullptr; //topic for receving commands
  char cmdTopicResp[100] = {0};   //topic for sending reponse for commands

  //binding definition for connecting onMessage from mqtt to local method
  Binding<String &, String &> *cbBind = nullptr;

  //Structure for keeping the handler and timing for executing
  struct outputTimerItem {
    outputTimerHandler handler;
    unsigned long timing;
    unsigned long lastTime;
  };

  //Structure for mapping commands to class functions
  typedef void (ESPManager::*cmdFn)(const char *respTopic, JsonVariant params);
  struct FunctionMap {
    char cmd[20];
    cmdFn func;
  };

  //Command list key and execution method
  FunctionMap cmdFunctions[6] = {
      {"reconnect", &ESPManager::cmdReconnect},
      {"restart", &ESPManager::cmdRestart},
      {"reset", &ESPManager::cmdReset},
      {"update", &ESPManager::cmdUpdate},
      {"status", &ESPManager::cmdStatus},
      {"getInfo", &ESPManager::cmdGetInfo}};

  struct cmp_str {
    bool operator()(char const *a, char const *b) const {
      return strcmp(a, b) < 0;
    }
  };

  std::map<const char *, cmdFunction, cmp_str> commands;

  std::map<const char *, eventIncomingHandler, cmp_str> inputEvents;
  std::map<const char *, outputTimerItem, cmp_str> outputEvents;

  //conectivity functions
  void createConnections();

#ifdef EM_UDP_DEBUG
  void initDebugUDP();
#endif

  void connectToWifi();
  void waitForWiFi();
  void debugWiFiStatus();
  void setupMQTT();
  void setOfflineStatusMessage();
  void setOnlineStatusMessage();
  void connectToMQTT();
  void disconnectWifi();

  // command functions
  void subscribeCMD();
  int findCmd(const char *cmd);
  void cmdReconnect(const char *respTopic, JsonVariant params);
  void cmdConfig(const char *respTopic, JsonVariant params);
  void cmdRestart(const char *respTopic, JsonVariant params);
  void cmdReset(const char *respTopic, JsonVariant params);
  void cmdGetInfo(const char *respTopic, JsonVariant params);
  void cmdUpdate(const char *respTopic, JsonVariant params);
  void cmdStatus(const char *respTopic, JsonVariant params);

  void readconfiguration();
  void reconnect();
  void messageReceived(String &topic, String &payload);
  void executeCommands(const char *topic, const char *payload);
  void executeTimingOutputEvents();
};

template <class... paramTypes>
class Binding {
 public:
  typedef void (ESPManager::*methType)(paramTypes...);
  Binding(ESPManager &obj, methType meth)
      : obj(&obj), meth(meth) {
    this_ = this;
  }

  void invoke(paramTypes... params) {
    (obj->*meth)(params...);
  }

  static void callback(paramTypes... params) {
    this_->invoke(params...);
  }

 private:
  static Binding<paramTypes...> *this_;
  ESPManager *obj;
  methType meth;
};

template <class... paramTypes>
Binding<paramTypes...> *Binding<paramTypes...>::this_ = nullptr;

#endif
