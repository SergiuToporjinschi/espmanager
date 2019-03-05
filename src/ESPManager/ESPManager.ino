

// by Sergiu Toporjinschi
// API: https://github.com/256dpi/arduino-mqtt

/**  INITIAL status
  //Sketch uses 407060 bytes (42%) of program storage space. Maximum is 958448 bytes.
  //Global variables use 30208 bytes (36%) of dynamic memory, leaving 51712 bytes for local variables. Maximum is 81920 bytes.
*/

#define DEBUGER //uncomment for Serial debugging statements

#ifdef DEBUGER
#define DBGB Serial.begin(115200)
#define DBGLN(x) Serial.println(x)
#define DBG(x) Serial.print(x)
#else
#define DBGLN(x)
#define DBG(x)
#define DBGB
#endif


#include <ArduinoJson.h>
#include "ESPManager.h"
#include "SettingsManager.h"

String readTemp(String const & topic, SettingsManager & settings);
SettingsManager conf;
ESPManager man;

void setup() {
  DBGB;
  conf.readSettings("/settings.json");
  JsonObject wlanConf = conf.getJsonObject("wlan");
  JsonObject mqttConf = conf.getJsonObject("mqtt");
  DBGLN("Start");
  man.createConnections(wlanConf, mqttConf);
//  man.addInputEventHandler(String("testEvent"), onCall);
  //  man.addOutputEventHandler(man->getStrSetting("mqtt.topic.submitData"), man->getLongSetting("mqtt.topic.submitDataInterval"), readTemp);
  //  man.addInputEventHandler(String("testEvent"), onCall);
}

void loop() {
  man.loopIt();
}

String readTemp(String const & topic, SettingsManager & settings) {
  DBGLN(settings.getInt("dht.pin"));
  DBGLN(settings.getString("dht.type"));
  return "{temp:25, humidity: 75}";
};

void onCall(String const & msg, SettingsManager & settings) {
  DBGLN(msg);
};
