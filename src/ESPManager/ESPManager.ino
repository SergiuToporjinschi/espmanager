

// by Sergiu Toporjinschi
// API: https://github.com/256dpi/arduino-mqtt

/**  INITIAL status
  //Sketch uses 407060 bytes (42%) of program storage space. Maximum is 958448 bytes.
  //Global variables use 30208 bytes (36%) of dynamic memory, leaving 51712 bytes for local variables. Maximum is 81920 bytes.
-1156
  Sketch uses 331936 bytes (34%) of program storage space. Maximum is 958448 bytes.
  Global variables use 29052 bytes (35%) of dynamic memory, leaving 52868 bytes for local variables. Maximum is 81920 bytes.
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

const char * readTemp(const char * msg);
SettingsManager conf;
ESPManager man;

void setup() {
  DBGB;
  conf.readSettings("/settings.json");
  JsonObject wlanConf = conf.getJsonObject("wlan");
  JsonObject mqttConf = conf.getJsonObject("mqtt");
  DBGLN("Start");
  man.createConnections(wlanConf, mqttConf);
  man.addIncomingEventHandler("IOT/espTest/inc", onCall);
//  man.addTimerOutputEventHandler("IOT/espTest/out", 2000, readTemp);
  man.sendMsg("IOT/espTest/out", "test");
}

void loop() {
  man.loopIt();
}

const char * readTemp(const char * msg) {
  return "{temp:39, humidity: 75}";
};

void onCall(const char * msg) {
  DBGLN(msg);
};
