//=================[ DEBUGER ]=================

#ifndef DEBUGGER
#  define DBGLN(x)
#  define DBG(x)
#else
#  define DBGLN(x) Serial.println(x)
#  define DBG(x) Serial.print(x)
#endif

//=================[ REVISION ]================
#ifndef REV
#  define REV (char *)"NONE"
#endif

//=================[ VERSION ]=================
#ifndef VER
#  define VER (char *)"0.0.0"
#endif

//===================[ MQTT ]==================
#ifndef MQTT_BUFFER
#  define MQTT_BUFFER 800
#endif