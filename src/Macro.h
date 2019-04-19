//=================[ DEBUGER ]=================
//#define DEBUGER
#ifndef DEBUGER
#define DBGLN(x)
#define DBG(x)
#else 
#define DBGLN(x) Serial.println(x)
#define DBG(x) Serial.print(x)
#endif

//=================[ VERSION ]=================
#ifndef VER
#define VER "2.0.3"
#endif

//=================[ REVISION ]================
#ifndef REV
#define REV "NONE"
#endif

//===================[ MQTT ]==================
#ifndef MQTT_BUFFER
#define MQTT_BUFFER 800
#endif