# ESPManager
A wrapper over Wifi and MQTT.

Dependencies:
* [arduino-mqtt](https://github.com/256dpi/arduino-mqtt)
* [ESP8266WiFi](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi)
* [ESP8266httpUpdate](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266httpUpdate)
* [SettingsManager 2.0.1V](https://github.com/SergiuToporjinschi/settingsmanager)

For details using SettingsManager please see the [read.me](https://github.com/SergiuToporjinschi/settingsmanager) file.
## Constructor
`
ESPManager();
`
 * Initialize WiFi;
 * Initialize MQTT;
  
## **createConnections**
```cpp
createConnections(<JsonObject wlanConf>, <JsonObject mqttConf>); 
```
 * Creates connection on WiFi;
 * Creates connection on MQTT;
 * Setting lastWill message;
 * Subscribe to cmd topic;

## **addTimerOutputEventHandler**
```cpp
addTimerOutputEventHandler(<const [string] | [const char *] mqttTopic>, <long intervalToSubmit>, <function executeFn>)
````

Register a function to be executed on a time interval. The result of function `executeFn` will be send to `mqttTopic` once at `intervalToSubmit` milliseconds;

 * **mqttTopic** a (string/const char *) with mqtt topic;
 * **intervalToSubmit** time to delay until next trigger;
 * **executeFn** a function which returns the (const char *) that will be sent over MQTT;


```cpp 
executeFn<const char *(const char * topic)>
```
 * **topic** MQTT topic where the return value will be send;

## **addIncomingEventHandler**
```cpp
addIncomingEventHandler(<const [string] | [const char *] mqttTopic>, <function onCall>)
```

Creates a listener on a specific MQTT topic and will execute the `onCall` when something has been submitted on `mqttTopic`;
 * **mqttTopic** a (string/const char *) with MQTT topic;
 * **onCall** function to call;

```cpp
onCall<void(const char * msg)>
```

## **sendMsg**
```cpp
sendMsg(<const [string] | [const char *] mqttTopic>, <const [string] | [const char *] msg>)
```

On calling Sends the `msg` on `mqttTopic`;
 * **mqttTopic** a (string/const char *) with MQTT topic;
 * **msg** a (string/const char *) message that will be send;

## **Configuration example:**
```json
{
  "wlan":{                                //WiFi settings
    "hostName":"espTest",
    "ssid":"mySSid",
    "password":"myPassword"
  },
  "mqtt":{                                //MQTT Settings
    "clientId":"espCID",
    "server":"192.168.100.100",
    "port":"1888",
    "user":"mqttTestUser",
    "password":"mqttTestUser",
    "sendOfflineStatus":true,             //Set lastwill message
    "retainMessage": true,
    "qos": 0,
    "topics":{                            //Internal topics
      "settings":"IOT/espTest/settings",
      "cmd":"IOT/espTest/cmd",
      "status":"IOT/espTest/status"
    }
  },
  "sketchVersion": "1.0",
  "otherKeys":{
    "topic":"env/bedroom/sc",
    "interval":1000
  }
}
```
<br>
## **Example:**
```cpp
#include <ArduinoJson.h>
#include "ESPManager.h"
#include "SettingsManager.h"

const char * readTemp(const char * msg);

SettingsManager conf;
ESPManager man;

void setup() {
  conf.readSettings("/settings.json");
  JsonObject wlanConf = conf.getJsonObject("wlan");
  JsonObject mqttConf = conf.getJsonObject("mqtt");

  man.createConnections(wlanConf, mqttConf);
  man.addIncomingEventHandler("IOT/espTest/inc", onCall);
  man.addTimerOutputEventHandler("IOT/espTest/out", 2000, readTemp);
  man.sendMsg("IOT/espTest/out", "test");
}

void loop() {
  man.loopIt();
}

const char * readTemp(const char * msg) {
  return "{temp:39, humidity: 75}";
};

void onCall(const char * msg) {
  Serial.println(msg);
};
```

## **Commands examples**

#### **Getting information**
```json
{ "cmd":"getInfo" }
```
Will return json with following info in topic `configurationJSON.mqtt.topics.cmd + '/resp'`
```json
{  
  "chipId": 5078804,                     //Chip id
  "localIP": "192.168.100.17",           //IP obtained on connecting to WiFi
  "macAddress": "2C:3A:E8:4D:7F:14",     //WiFi MAC Address
  "lastRestartReson": "External System", //reason of last restart
  "flashChipId": 1323036,                //Flash chip id
  "coreVersion": "2.5.0",                //ESP firmware version
  "sdkVersion": "3.0.0-dev(c0f7b44)",    //ESP SDK Version
  "vcc": "3.38 V",                       //Power voltaj
  "flashChipSpeed":"40 MHz",             //Falsh chip speed
  "cycleCount": 3554582419,              //Number of cycles from powering
  "cpuFreq": "80 MHz",                   //CPU speed
  "freeHeap": 44768,                     //Free heap
  "flashChipSize": 1048576,              //Free chip size
  "sketchSize": 339056,                  //Free chip size
  "freeSketchSpace": 622592,             //Free sketch space
  "flashChipRealSize": 1048576,          //Flash chip size
  "espManagerVersion": "2.0.0",          //espManager version
  "sketchVersion": "1.0"                 //Your sketchVersion
}
```
#### **Ask for a reset**
```json
{ "cmd":"reset" }
```

#### **Ask for a reconnect**
```json
{ "cmd":"reconnect" }
```

#### **Update over the air**
```json
{
  "cmd":"update",                              //Command
  "params":{                                   //Parameters for command
    "type":"sketch",                           //What to update <sketch|spiffs>
    "version":"4.0",                           //What's the version that needs to be installed
    "url":"http://myServer.com/api/url/update" //From where the code can be requested
  }
}
```