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

const char * readTemp(const char * msg);
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
  "chipId": 5078804,                     
  "localIP": "192.168.100.17",           
  "macAddress": "2C:3A:E8:4D:7F:14",     
  "lastRestartReson": "External System", 
  "flashChipId": 1323036,                
  "coreVersion": "2.5.0",                
  "sdkVersion": "3.0.0-dev(c0f7b44)",    
  "vcc": "3.38 V",                       
  "flashChipSpeed":"40 MHz",             
  "cycleCount": 3554582419,             
  "cpuFreq": "80 MHz",                   
  "freeHeap": 44768,                     
  "flashChipSize": 1048576,              
  "sketchSize": 339056,                  
  "freeSketchSpace": 622592,            
  "flashChipRealSize": 1048576,          
  "espManagerVersion": "2.0.0",          
  "sketchVersion": "1.0"                 
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
