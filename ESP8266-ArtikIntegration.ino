/* really simple WebSockets conection for an ESP8266 with Artik Cloud
 *  There are a number of things you need to do first, I'll be adding documentation to the GitHub Readme
 *  in due course
 */
 
 



 
 // code to integrate into ARTIC so that this ESP can be controlled via WiFi

#include "settings.h"
#include <ESP8266WiFi.h>

//#define DEBUG_ESP_PORT
//#define DEBUG_WEBSOCKETS(...) os_printf( __VA_ARGS__ )
#include <WebSockets.h>
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#define DEVICE_ID YOUR_DEVICE_ID
#define DEVICE_TOKEN YOUR_DEVICE_TOKEN

#if (DEVICE_ID == YOUR_DEVICE_ID)
/*IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT
/*  for privacy reasons I keep my sensitive info in a header file that isn't included
 *  You should provide your device id and token 
 **************************************************************************************/
  #include "nogit_artik.h"
#endif
 


WebSocketsClient webSocket;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW); // turn it on
  Serial.begin(BAUD);
  Serial.setDebugOutput(true);

  // open a SECURE websockets connection
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  doWiFiWaitDHCP();


      // Handshake with the server
  webSocket.beginSSL(ARTIK_WS_HOST, ARTIK_WS_PORT, ARTIK_WS_PATH);
  webSocket.onEvent(webSocketEvent);
  
  
}


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {


    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
            {
                Serial.printf("[WSc] Connected to url: %s\n",  payload);
                // REGISTER aka Authenticate and Register
                 webSocket.sendTXT("{ \"sdid\": \""  DEVICE_ID  "\"," 
                  "\"Authorization\": \"bearer " DEVICE_TOKEN "\"," 
                  "\"type\": \"register\"," 
                  "\"cid\": \"1234567890\"" 
                  "}");
              sendStateToArtik(true, 100);
            }
            break;
        case WStype_TEXT:
            Serial.printf("[WSc] get text: %s\n", payload);
            handleMsg(payload);

      // send message to server
      // webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            Serial.printf("[WSc] get binary length: %u\n", length);
            hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
    }
  //S.println("Done");
}

    
void handleMsg(uint8* payload) {
      DynamicJsonBuffer jsonBuffer;

      JsonObject& root = jsonBuffer.parseObject((char *)payload);
      String type = root["type"];
      if (type == "") {
        S.println("Type is empty");
      }
      else if (type == NULL) {
        S.println("Type is null");
      }
      else if (type == "ping") {
        S.println("Ping Received");
      }
      else if (type == "action") {
        S.println("Action Received");
        String action = root["data"]["actions"][0]["name"];
        S.println("Action is " + action);
        if (action == "setOn") {
          actionSetOn(); // no parameters
        }
        else if(action == "setOff") {
          actionSetOff(); // no parameters
        }
        else if (action == "setLevel") {
          int pct = root["data"]["actions"][0]["parameters"]["level"];
          actionSetLevel(pct);
        }
      }
      root.prettyPrintTo(S);

}

void actionSetOn() {
    analogWrite(LED_BUILTIN, 0); /// perhaps update ARTIK?
    sendStateToArtik(true, 100);
}

void actionSetOff() {
    analogWrite(LED_BUILTIN, PWMRANGE); /// perhaps update ARTIK?
    sendStateToArtik(false, 0);
}

void actionSetLevel(int pct) {
    // max is 1000 not 1024 (which it really is!)
    analogWrite(LED_BUILTIN, PWMRANGE - PWMRANGE*pct/100); /// perhaps update ARTIK?
    sendStateToArtik(true, pct);
}

void sendStateToArtik(boolean state,int level) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    // no need to send authorization for WebSockets, we did that when we registered
    root["sdid"] = DEVICE_ID;
    root["data"] = jsonBuffer.createObject();
    root["data"]["state"] = RawJson(state ? "true" : "false");
    root["data"]["level"] = level;
    S.println("sending:");

    String output;
    root.printTo(output);
    S.println(output);
    webSocket.sendTXT(output);
}

void loop() {
  // put your main code here, to run repeatedly:
  webSocket.loop();
}
