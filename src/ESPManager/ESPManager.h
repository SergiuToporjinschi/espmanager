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
    //    using eventHandler = std::function<void(String const&, SettingsManager &settings)>;
    //    using outputHandlerType = std::function<String(String const&, SettingsManager &settings)>;
    ESPManager ();
    ~ESPManager();
    void createConnections(JsonObject wlanConf, JsonObject mqttConf);
    void loopIt();
    const char * getVersion() {
      return version;
    }

    //    void addInputEventHandler(String topic, eventHandler handler);
    //    void addOutputEventHandler(String topic, long loopTime, outputHandlerType handler);
    //    String getStrSetting(String property);
    //    int getIntSetting(String property);
    //    long getLongSetting(String property);
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
    //    struct outputTimerHandler {
    //      outputHandlerType handler;
    //      long timing;
    //      long lastTime;
    //    };

    // command functions
    typedef void (ESPManager::*cmdFn)(const char *);
    std::map <const char *, cmdFn> commands;
    void cmdReconnect(const char * payload);
    void cmdConfig(const char * payload);
    void cmdRestart(const char * payload);
    void cmdReset(const char * payload);
    void cmdGetInfo(const char * payload);
    void subscribeTopics();
    
    void messageReceived(String &topic, String &payload);
    bool executeInteralTopics(const char * topic, const char * payload);
    bool executeRegisteredTopics(const char * topic, const char * payload);
    
    //    std::map <String, eventHandler> inputEvents;
    //    std::map <String, outputTimerHandler> outputEvents;
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
