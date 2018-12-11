// by Sergiu Toporjinschi
// API: https://github.com/256dpi/arduino-mqtt
#define DEBUG_SERIAL //uncomment for Serial debugging statements

#ifdef DEBUG_SERIAL
#define DEBUG_BEGIN Serial.begin(115200)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#else
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT(x)
#define DEBUG_BEGIN
#endif

#include "ESPManager.h"
String readTemp(String const & topic, SettingsManager & settings);

ESPManager *man;
void setup() {
  DEBUG_BEGIN;
  //delay(5000);
  man = new ESPManager(String("/settings.cfg"), 2, true);
  man->addOutputEventHandler(man->getStrSetting("mqtt.topic.submitData"), man->getLongSetting("mqtt.topic.submitDataInterval"), readTemp);
  man->addInputEventHandler(String("testEvent"), onCall);
  man->createConnections();
}

void loop() {
  man->loopIt();
}

String readTemp(String const & topic, SettingsManager & settings) {
  DEBUG_PRINTLN(settings.getInt("dht.pin"));
  DEBUG_PRINTLN(settings.getString("dht.type"));
  return "{temp:25, humidity: 75}";
};

void onCall(String const & msg, SettingsManager & settings){
  DEBUG_PRINTLN(msg);
};