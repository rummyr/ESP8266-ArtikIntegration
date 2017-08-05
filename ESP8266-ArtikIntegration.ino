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


#define DEVICE_TOKEN // YOUR_DEVICE_ID
#define DEVICE_ID // YOUR_DEVICE_TOKEN
#define BAUD 74880

/* IMPORTANT, if this line is left in, delete it! */
#include "nogit_artik.h"



WebSocketsClient webSocket;
String cid = "12345678";

String LAST_SENT_MSG_TYPE_NONE = "No known requests in flight";
String LAST_SENT_MSG_TYPE_REGISTRATION = "registration request in flight";
String LAST_SENT_MSG_TYPE_STATUS_UPDATE = "status update request in flight";
String expectingResponseFor = LAST_SENT_MSG_TYPE_NONE;


/* prototypes */
void setInFlightRequestType(String s);
String getInFlightRequestType();
void clearInFlightRequestType();

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
  // Connect to server .. the onEvent WStype_CONNECTED will perform the initial handshake
  webSocket.beginSSL(ARTIK_WS_HOST, ARTIK_WS_PORT, ARTIK_WS_PATH, 
  // !!IMPORTANT .. verify this fingerprint yourself using https://www.grc.com/fingerprints.htm -- other ways are also available of course.
       /*Fingerprint*/     "E1:6B:EB:1B:21:61:94:6A:21:1B:48:97:DA:D5:67:93:98:B0:43:C8");
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
                Serial.printf("[WSc] sending registration message");
                // REGISTER aka Authenticate and Register
                setInFlightRequestType(LAST_SENT_MSG_TYPE_REGISTRATION);
                webSocket.sendTXT("{ \"sdid\": \""  DEVICE_ID  "\"," 
                  "\"Authorization\": \"bearer " DEVICE_TOKEN "\"," 
                  "\"type\": \"register\"," 
                  "\"cid\": \" " + cid + "\"" 
                  "}");
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

      S.println("Handling Message>>");
      root.prettyPrintTo(S);
      S.println("<<");
      S.println();
           
      String type = root["type"];

      
      if (type == "" && getInFlightRequestType() == LAST_SENT_MSG_TYPE_REGISTRATION) {
        clearInFlightRequestType();
        S.println("Type is empty, expecting a response from registration");
        String code = root["data"]["code"];
        String message = root["data"]["message"];
        S.println("Code recieved:" + code + " Message:" + message);
        String cid = root["data"]["cid"];
        S.println("cid recieved:" + cid);

        if (code == "200" && message == "OK") {
            S.println("code 200, message OK , updating with current state");
            S.println();
            sendStateToArtik(true, 100);
        }
        else {
          S.println("ERROR: Looks like registration FAILED!");
        }
      }
      else if (type == "" && getInFlightRequestType() == LAST_SENT_MSG_TYPE_STATUS_UPDATE) {
        clearInFlightRequestType();
        S.println("Type is empty, expecting a response from a status update");
        String messageId = root["data"]["mid"];
        String cid = root["data"]["cid"]; // not really interested in this at the moment
        S.println("Message Id recieved:" + messageId);
        S.println();
      }
      else if (type == "") {
        S.println("!!Got an empty type, but dont know what it is in reply to! (expecting a response type of " + getInFlightRequestType());
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
        S.print("Action is " + action);
        if (action == "setOn") {
          S.println();
          actionSetOn(); // no parameters
        }
        else if(action == "setOff") {
          S.println();
          actionSetOff(); // no parameters
        }
        else if (action == "setLevel") {
          int pct = root["data"]["actions"][0]["parameters"]["level"];
          S.print(" Level:" + pct);
          S.println();
          actionSetLevel(pct);
        }

      }
 

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
    root["type"] = "message"; // 2017-08-05 fix to send type with state message
    root["cid"] = cid;
    root["data"] = jsonBuffer.createObject();
    root["data"]["state"] = RawJson(state ? "\"on\"" : "\"off\"");
    root["data"]["level"] = level;

    S.println("sending:>>");
    String output;
    root.printTo(output);
    S.println(output);
    setInFlightRequestType(LAST_SENT_MSG_TYPE_STATUS_UPDATE);
    webSocket.sendTXT(output);
    S.println("<<");
}

void loop() {
  // put your main code here, to run repeatedly:
  webSocket.loop();
}

void setInFlightRequestType(String s) {
  expectingResponseFor = s;
}
String getInFlightRequestType() {
  return expectingResponseFor;
}
void clearInFlightRequestType() {
  expectingResponseFor = LAST_SENT_MSG_TYPE_NONE;
  }

