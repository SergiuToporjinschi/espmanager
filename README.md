[![GitHub repo size in bytes](https://img.shields.io/github/repo-size/badges/shields.svg)](https://github.com/SergiuToporjinschi/espmanager)
[![GitHub last commit](https://img.shields.io/github/last-commit/SergiuToporjinschi/espmanager.svg)](https://github.com/SergiuToporjinschi/espmanager/commits/master)
[![GitHub stars](https://img.shields.io/github/stars/SergiuToporjinschi/espmanager.svg)](https://github.com/SergiuToporjinschi/espmanager/stargazers)
[![GitHub watchers](https://img.shields.io/github/watchers/SergiuToporjinschi/espmanager.svg)](https://github.com/SergiuToporjinschi/espmanager/watchers)
[![GitHub license](https://img.shields.io/github/license/SergiuToporjinschi/espmanager.svg)](https://github.com/SergiuToporjinschi/espmanager/blob/master/LICENSE)
[![Code Climate](https://codeclimate.com/github/codeclimate/codeclimate/badges/gpa.svg)](https://codeclimate.com/github/SergiuToporjinschi/espmanager)

# ESPManager
A wrapper over Wifi and MQTT.

Dependencies:
* [arduino-mqtt](https://github.com/256dpi/arduino-mqtt)
* [ESP8266WiFi](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi)
* [ESP8266httpUpdate](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266httpUpdate)

For details using SettingsManager please see the [read.me](https://github.com/SergiuToporjinschi/settingsmanager) file.
## Compile macro flags
 >**EM_UDP_DEBUG** - For sending aditional information via UDP, you configure ip and port in your configuration file under the wlan object;
```json
"debugUDP": {
  "enabled": true,
  "server": "192.168.1.1",
  "port": "4321"
}
```
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
 
## **createConnections**
```cpp
    void onBeforeWaitingWiFiCon(std::function<void()> func) { this->beforeWaitingWiFiCon = func; };
    void onWaitingWiFiCon(std::function<void()> func) { this->waitingWiFiCon = func; };
    void onAfterWaitingWiFiCon(std::function<void()> func) { this->afterWaitingWiFiCon = func; };
   
    void onBeforeWaitingMQTTCon(std::function<void()> func) { this->beforeWaitingMQTTCon = func; };
    void onWaitingMQTTCon(std::function<void()> func) { this->waitingMQTTCon = func; };
    void onAfterWaitingMQTTCon(std::function<void()> func) { this->afterWaitingMQTTCon = func; };
```
You can add events to be triggered on WiFi Connection or MQTT;
This methods will be executed before starting connection, while waiting for connection or after connection process has finisied; `onAfterWaiting` will be executed if the connection has been made or not.

## **addTimerOutputEventHandler**
```cpp
addTimerOutputEventHandler(<const [string] | [const char *] mqttTopic>, <long intervalToSubmit>, <function executeFn>)
````

Register a function to be executed on a time interval. The result of function `executeFn` will be send to `mqttTopic` once at `intervalToSubmit` milliseconds;

 * **mqttTopic** a (string/const char *) with mqtt topic;
 * **intervalToSubmit** time to delay until next trigger;
 * **executeFn** a function which returns the (const char *) that will be sent over MQTT;


```cpp
executeFn<char *(const char * topic)>
```
 * **topic** MQTT topic where the return value will be send;
 * can return nullptr for canceling submition;

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

## **addCommand**
```cpp
addCommand(<const [string] | [const char *] cmd>, <function cmdFunction>)
```
Register a function for a command; when the command is received the function will will be triggered with the parameters object that is coming with the command;

```cpp
function<char *(JsonVariant)>
```
 * **JsonVariant** of configuration recevied as 'params' in command;

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
  "wlan":{
    "hostName":"espTest",
    "ssid":"mySSid",
    "password":"myPassword"
  },
  "mqtt":{
    "clientId":"espCID",
    "server":"192.168.1.1",
    "port":"1888",
    "user":"mqttTestUser",
    "password":"mqttTestUser",
    "sendOfflineStatus":true,
    "retainMessage": true,
    "qos": 0,
    "topics":{
      "cmd":"IOT/espTest/cmd",
      "status":"IOT/espTest/status"
    }
  },
  "otherKeys":{
    "topic":"env/bedroom/sc",
    "interval":1000
  }
}
```

## **Example:**
```cpp

#include <ArduinoJson.h>
#include "ESPManager.h"
#include "SettingsManager.h"

char * readTemp(const char * msg);
SettingsManager conf;
ESPManager man;

void setup() {
  Serial.begin(115200);
  //Reading configuration from json file
  conf.readSettings("/settings.json");
  //Splitting settings in wlanConf and MqttConf
  JsonObject wlanConf = conf.getJsonObject("wlan");
  JsonObject mqttConf = conf.getJsonObject("mqtt");
  //Setting scketch ino verion
  man.setSketchVersion("1.0.0");
  //Creating connection to wlan and mqtt
  man.createConnections(wlanConf, mqttConf);
  //Add listener on IOT/espTest/inc
  man.addIncomingEventHandler("IOT/espTest/inc", onCall);
  //Adding timout trigger
  man.addTimerOutputEventHandler("IOT/espTest/out", 2000, readTemp);
  //Send instant message on IOT/espTest/out
  man.sendMsg("IOT/espTest/out", "test");
}

void loop() {
  man.loopIt();
}

char * readTemp(const char * msg) {
  //Allocate memory, will be freed by manager
  char * ret = (char *) malloc(25 * sizeof(char));
  strcpy(ret, "{temp:39, humidity: 75}");
  return ret;
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
```js
{  
   "hostName": "nodePlants",
   "chipId": 8963427,
   "localIP": "192.168.1.101",
   "macAddress": "68:C6:3A:88:C5:63",
   "RSSI": -63,  //Signal strength in dBm
   "lastRestartReson": "External System",
   "flashChipId": 1458208,
   "coreVersion": "2.5.2",
   "sdkVersion": "2.2.1(cfd48f3)",
   "vcc": 2.96,  //power in V
   "flashChipSpeed": 40, //MHz
   "cycleCount": 948668853,
   "cpuFreq": 80, //MHz
   "freeHeap": 44664, //Bytes
   "heapFrag": 1, //Heap fragmentation in %
   "maxFreeBlockSize": 44512, //Maximum free space that could be allocated in one block (Bytes)
   "flashChipSize": 4194304, //Bytes
   "sketchSize": 345888, //Bytes
   "freeSketchSpace": 2797568, //Bytes
   "flashChipRealSize": 4194304, //Bytes
   "espManagerVersion": "1.0.0",
   "sketchVersion": "1.0.0",
   "sketchMD5": "a4bf983f66d52ce63d1c186c27fdb6ad"
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
  "cmd":"update",
  "params":{
    "type":"sketch",
    "version":"4.0",
    "url":"http://myServer.com/api/url/update"
  }
}
```
