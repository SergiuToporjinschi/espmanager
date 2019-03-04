#ifndef ESPManager_h
#define ESPManager_h
#include <stdlib.h>
#include "Arduino.h"
#include <ArduinoJson.h>
#include <MQTTClient.h>
#include <ESP8266WiFi.h>
//#include <ESP8266httpUpdate.h>
//#include <functional>
//#include <map>
#include "SettingsManager.h"

//template<class... params> class Binding;
class ESPManager {
  public:
    //    using eventHandler = std::function<void(String const&, SettingsManager &settings)>;
    //    using outputHandlerType = std::function<String(String const&, SettingsManager &settings)>;
    ESPManager ();
    ~ESPManager();
    void createConnections(JsonObject wlanConf, JsonObject mqttConf);
    const char * getVersion() {
      return version;
    }
    //    void loopIt();
    //    void addInputEventHandler(String topic, eventHandler handler);
    //    void addOutputEventHandler(String topic, long loopTime, outputHandlerType handler);
    //    String getStrSetting(String property);
    //    int getIntSetting(String property);
    //    long getLongSetting(String property);
  private:
    const char * version = "2.0.0";
    bool retainMsg = false;
    WiFiClient net;
    JsonObject _wlanConf;
    JsonObject _mqttConf;
    MQTTClient mqttCli;
    WiFiMode wifiMode;
    bool sendOfflineStatus; //Sends a retain message for registering stauts
    void connectToWifi();
    void waitForWiFi();
    void debugWiFiStatus();
    void setupMQTT();
    void setOfflineStatusMessage();
    void setOnlineStatusMessage();
    void connectToMQTT();
    void disconnectWifi();

    //    Binding<String &, String &> *cbBind = nullptr;
    //    typedef void (ESPManager::*cmdFn)(String);
    //    struct outputTimerHandler {
    //      outputHandlerType handler;
    //      long timing;
    //      long lastTime;
    //    };

    //    SettingsManager settings;
    //    std::map <String, cmdFn> commands;
    //    std::map <String, eventHandler> inputEvents;
    //    std::map <String, outputTimerHandler> outputEvents;
    //
    //    int ltpm = 0; //Last time publish message
    //
    //    void connect(String payload);
    //
    //    void subscribeTopics();
    //    void messageReceived(String &topic, String &payload);
    //
    //    void saveSettings(String payload);
    //    void updateEsp(String payload);
    //    void cmdReconnect(String payload);
    //    void cmdConfig(String payload);
    //    void cmdRestart(String payload);
    //    void cmdReset(String payload);
    //    void getInfo(String payload);
};
//template<class... paramTypes>
//class Binding {
//  public:
//    typedef void (ESPManager::*methType)(paramTypes...);
//    Binding(ESPManager& obj, methType meth)
//      : obj(&obj), meth(meth) {
//      this_ = this;
//    }
//
//    void invoke(paramTypes... params) {
//      (obj->*meth)(params...);
//    }
//
//    static void callback(paramTypes... params) {
//      this_->invoke(params...);
//    }
//
//  private:
//    static Binding<paramTypes...> *this_;
//    ESPManager* obj;
//    methType meth;
//};
//
//template<class... paramTypes>
//Binding<paramTypes...>* Binding<paramTypes...>::this_ = nullptr;


#endif
