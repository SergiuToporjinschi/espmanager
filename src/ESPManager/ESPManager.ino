

// by Sergiu Toporjinschi
// API: https://github.com/256dpi/arduino-mqtt

/**  INITIAL status
  //Sketch uses 407060 bytes (42%) of program storage space. Maximum is 958448 bytes.
  //Global variables use 30208 bytes (36%) of dynamic memory, leaving 51712 bytes for local variables. Maximum is 81920 bytes.
*/

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


#include <ArduinoJson.h>
#include "ESPManager.h"
#include "SettingsManager.h"

String readTemp(String const & topic, SettingsManager & settings);
SettingsManager conf;
ESPManager *man;

void setup() {
  DEBUG_BEGIN;
  //delay(5000);
  conf.readSettings("/settings.json");
  JsonObject wlanConf = conf.getJsonObject("wlan");
  JsonObject mqttConf = conf.getJsonObject("mqtt");
  man = new ESPManager(wlanConf, mqttConf);
//  man.addOutputEventHandler(man->getStrSetting("mqtt.topic.submitData"), man->getLongSetting("mqtt.topic.submitDataInterval"), readTemp);
//  man.addInputEventHandler(String("testEvent"), onCall);
//  man.createConnections();
}

void loop() {
//  man->loopIt();
}

String readTemp(String const & topic, SettingsManager & settings) {
  DEBUG_PRINTLN(settings.getInt("dht.pin"));
  DEBUG_PRINTLN(settings.getString("dht.type"));
  return "{temp:25, humidity: 75}";
};

void onCall(String const & msg, SettingsManager & settings) {
  DEBUG_PRINTLN(msg);
};
