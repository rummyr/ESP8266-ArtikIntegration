#ifndef settings_h
#warning settings_h NOT yet defined
#else
#warning settings_h DEFINED
#endif

#ifndef settings_h
#define settings_h
#include <Arduino.h>

void doWiFiWaitDHCP();


#define S Serial


#define WEBSOCKET_URL "wss://api.artik.cloud/v1.1/websocket?ack=true"
#define ARTIK_WS_HOST "api.artik.cloud"
#define ARTIK_WS_PORT 443
#define ARTIK_WS_PATH "/v1.1/websocket?ack=true"

#endif
