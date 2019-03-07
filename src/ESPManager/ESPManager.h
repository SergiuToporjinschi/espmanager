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
    void addTimerOutputEventHandler(const char * topic, long loopTime, outputTimerHandler handler);
  private:
    const char * version = "2.0.0";
    bool retainMsg = false;
    int qos = 0;

    WiFiClient net;
    JsonObject _wlanConf;
    JsonObject _mqttConf;
    MQTTClient mqttCli;
    WiFiMode wifiMode;
    bool sendOfflineStatus; //Sends a retain message for registering stauts

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

    Binding<String &, String &> *cbBind = nullptr;
    struct outputTimerItem {
      outputTimerHandler handler;
      long timing;
      long lastTime;
    };

    // command functions
    typedef void (ESPManager::*cmdFn)(const char *);
    struct FunctionMap {
      char cmd[20];
      cmdFn func;
    };

    FunctionMap cmdFunctions[3] = {
      {"reconnect", &ESPManager::cmdReconnect},
      //{"restart", &ESPManager::cmdRestart}, --> currently disabled, it crashes the esp see https://github.com/SergiuToporjinschi/espmanager/issues/3
      {"reset", &ESPManager::cmdReset},
      {"getInfo", &ESPManager::cmdGetInfo}
    };

    /**
       Finds a command given as parameter and returns position in cmdFunctions
    */
    int findCmd(const char * cmd);
    //    std::map <const char *, cmdFn> commands;
    void cmdReconnect(const char * payload);
    void cmdConfig(const char * payload);
    void cmdRestart(const char * payload);
    void cmdReset(const char * payload);
    void cmdGetInfo(const char * payload);
    void subscribeTopics();

    void messageReceived(String & topic, String & payload);
    bool executeCMDInteralTopics(const char * topic, const char * payload);
    bool executeRegisteredTopics(const char * topic, const char * payload);

    struct cmp_str {
      bool operator()(char const *a, char const *b) const {
        return strcmp(a, b) < 0;
      }
    };
    std::map <const char *, eventIncomingHandler, cmp_str> inputEvents;
    std::map <const char *, outputTimerItem, cmp_str> outputEvents;
    //
    //    int ltpm = 0; //Last time publish message
    //
    //    void connect(String payload);
    //

    //
    //    void saveSettings(String payload);
    //    void updateEsp(String payload);

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
