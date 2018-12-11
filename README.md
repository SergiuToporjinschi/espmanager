# ESPManager
A wrapper over Wifi and MQTT.

Dependencies:
* [arduino-mqtt](https://github.com/256dpi/arduino-mqtt)
* [ESP8266WiFi](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi)
* [ESP8266httpUpdate](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266httpUpdate)
* [SettingsManager](https://github.com/SergiuToporjinschi/settingsmanager)

For details using SettingsManager please see the [read.me](https://github.com/SergiuToporjinschi/settingsmanager) file.
## Constructor
`
ESPManager(<string spiffConfFile>, <int mqttQOS>, <boolean sendStatus>);
`
 * Initialise MQTT;
 * Reading settings from SPIFFS;
 * Configure WIFI as station;

## **addOutputEventHandler**
`
addOutputEventHandler(<string mqttTopic>, <long intervalToSubmit>, <function executeFn> )
`
Will execute the *executeFn* in interval of *intervalToSubmit* and submit the returned value to *mqttTopic* topic;
 * **mqttTopic** a string with mqtt topic;
 * **intervalToSubmit** time to delay until next function trigger;
 * **executeFn** a function which returns the string that will be sent over MQTT;

## **addInputEventHandler**
`
addInputEventHandler(<string mqttTopic>, <function onCall>)
`
Creates a listener on a specific MQTT topic and will execute the *onCall* when something has been submitted;
 * **mqttTopic** a string with MQTT topic;
 * **onCall** a function which returns void;

## createConnections;
`
createConnections()
`
Creates connections and starts the process.
