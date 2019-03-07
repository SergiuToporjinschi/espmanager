#ifndef ESPManager_h
#define ESPManager_h

#include <stdlib.h>
#include "Arduino.h"
#include <ArduinoJson.h>
#include <MQTTClient.h>
#include <ESP8266WiFi.h>
//#include <ESP8266httpUpdate.h>
#include <functional>
#include <map>
#include "SettingsManager.h"
#include <pgmspace.h>

static const char STATUS_FORMAT_P[] PROGMEM = "{\"name\":\"%s\", \"status\":\"%s\"}";
static const char STATUS_ONLINE_P[] PROGMEM = "online";
static const char STATUS_OFFLINE_P[] PROGMEM = "offline";

template<class... params> class Binding;
class ESPManager {
  public:
    using eventIncomingHandler = std::function<void(const char *)>;
    using outputTimerHandler = std::function<const char *(const char *)>;
    ESPManager ();
    ~ESPManager();
    void createConnections(JsonObject wlanConf, JsonObject mqttConf);
    void loopIt();

    const char * getVersion() {
      return version;
    }

    void addIncomingEventHandler(const char * topic, eventIncomingHandler handler);
    void addIncomingEventHandler(const String topic, eventIncomingHandler handler);
    void addTimerOutputEventHandler(const char * topic, long loopTime, outputTimerHandler handler);
    void addTimerOutputEventHandler(const String topic, long loopTime, outputTimerHandler handler);
  private:
    const char * version = "2.0.0";
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
      long timing;
      long lastTime;
    };

    //Structure for mapping commands to class functions
    typedef void (ESPManager::*cmdFn)(const char *);
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
    void cmdReconnect(const char * payload);
    void cmdConfig(const char * payload);
    void cmdRestart(const char * payload);
    void cmdReset(const char * payload);
    void cmdGetInfo(const char * payload);
    void cmdUpdate(const char * payload);
    

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
