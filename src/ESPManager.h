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
#ifndef ESPManager_h
#define ESPManager_h

#include <stdlib.h>
#include "Arduino.h"
#include <ArduinoJson.h>
#include <MQTTClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <functional>
#include <map>
#include <pgmspace.h>

static const char STATUS_FORMAT_P[] PROGMEM = "{\"name\":\"%s\", \"status\":\"%s\"}";
static const char STATUS_ONLINE_P[] PROGMEM = "online";
static const char STATUS_OFFLINE_P[] PROGMEM = "offline";

static const char UPDATE_SKETCH_P[] PROGMEM = "sketch";
static const char UPDATE_SPIFFS_P[] PROGMEM = "spiffs";

static const char SKETCH_VERSION_PATTERN_P[] = ",\"sketchVersion\":\"%s\"";
static const char INFO_PATTERN_P[] PROGMEM = "{\"chipId\":%i,\"localIP\":\"%s\",\"macAddress\":\"%s\",\"lastRestartReson\":\"%s\",\"flashChipId\":%u,\"coreVersion\":\"%s\",\"sdkVersion\":\"%s\",\"vcc\":\"%1.2f V\",\"flashChipSpeed\":\"%u MHz\",\"cycleCount\":%u,\"cpuFreq\":\"%u MHz\", \"freeHeap\":%u,\"flashChipSize\":%u,\"sketchSize\":%u,\"freeSketchSpace\":%u,\"flashChipRealSize\":%u,\"espManagerVersion\":\"%s\"%s}";

enum ESPManConnStatus {
    CONNECTION_OK,
    INVALID_WLAN_CONF,
    INVLIAD_MQTT_CONF
};

template<class... params> class Binding;
class ESPManager {
  public:
    using eventIncomingHandler = std::function<void(const char *)>;
    using outputTimerHandler = std::function<const char *(const char *)>;
    ESPManager ();
    ~ESPManager();
    ESPManConnStatus createConnections(JsonObject wlanConf, JsonObject mqttConf);
    void loopIt();
    void setSketchVersion(String ver) {
      sketchVersion = ver.c_str();
    };
    void setSketchVersion(const char * ver) {
      sketchVersion = ver;
    };

    void addIncomingEventHandler(const char * topic, eventIncomingHandler handler);
    void addIncomingEventHandler(const String topic, eventIncomingHandler handler) {
      addIncomingEventHandler(topic.c_str(), handler);
    }
    void addTimerOutputEventHandler(const char * topic, long loopTime, outputTimerHandler handler);
    void addTimerOutputEventHandler(const String topic, long loopTime, outputTimerHandler handler) {
      addTimerOutputEventHandler(topic.c_str(), loopTime, handler);
    }
    void sendMsg(const String topic, const String msg) {
      sendMsg(topic, msg, false, qos);
    };
    void sendMsg(const String topic, const String msg, bool retain, int qos) {
      sendMsg(topic.c_str(), msg.c_str(), retain, qos);
    };
    void sendMsg(const char * topic, const char * msg) {
      sendMsg(topic, msg, false, qos);
    };
    void sendMsg(const char * topic, const char * msg, bool retain, int qos);
  private:
    const char * version = "2.0.0";
    const char * sketchVersion = nullptr;
    bool retainMsg = false;
    int qos = 0;

    WiFiClient net;
    JsonObject _wlanConf; //WLAN settings
    JsonObject _mqttConf; //MQTT settings
    MQTTClient mqttCli;   //MQTT client engine
    WiFiMode wifiMode;    //WiFi Engine

    bool sendOfflineStatus; //Sends a retain message for registering stauts

    //binding definition for connecting onMessage from mqtt to local method
    Binding<String &, String &> *cbBind = nullptr;

    //Structure for keeping the handler and timing for executing
    struct outputTimerItem {
      outputTimerHandler handler;
      unsigned long timing;
      unsigned long lastTime;
    };

    //Structure for mapping commands to class functions
    typedef void (ESPManager::*cmdFn)(JsonVariant params);
    struct FunctionMap {
      char cmd[20];
      cmdFn func;
    };

    //Command list key and execution method
    FunctionMap cmdFunctions[4] = {
      {"reconnect", &ESPManager::cmdReconnect},
      //{"restart", &ESPManager::cmdRestart}, --> currently disabled, it crashes the esp see https://github.com/SergiuToporjinschi/espmanager/issues/3
      {"reset", &ESPManager::cmdReset},
      {"update", &ESPManager::cmdUpdate},
      {"getInfo", &ESPManager::cmdGetInfo}
    };

    struct cmp_str {
      bool operator()(char const *a, char const *b) const {
        return strcmp(a, b) < 0;
      }
    };

    std::map <const char *, eventIncomingHandler, cmp_str> inputEvents;
    std::map <const char *, outputTimerItem, cmp_str> outputEvents;

    //conectivity functions
    void createConnections();
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
    int findCmd(const char * cmd);
    void cmdReconnect(JsonVariant params);
    void cmdConfig(JsonVariant params);
    void cmdRestart(JsonVariant params);
    void cmdReset(JsonVariant params);
    void cmdGetInfo(JsonVariant params);
    void cmdUpdate(JsonVariant params);

    void reconnect();
    void messageReceived(String & topic, String & payload);
    bool executeInteralTopics(const char * topic, const char * payload);
    bool executeRegisteredTopics(const char * topic, const char * payload);
    void executeTimingOutputEvents();
};

template<class... paramTypes>
class Binding {
  public:
    typedef void (ESPManager::*methType)(paramTypes...);
    Binding(ESPManager& obj, methType meth)
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
    ESPManager* obj;
    methType meth;
};

template<class... paramTypes>
Binding<paramTypes...>* Binding<paramTypes...>::this_ = nullptr;


#endif
