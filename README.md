# ESP8266-ArtikIntegration

Sample integration of an ESP8266 with Artik Cloud with voice control from Google Home

.

**WARNING: The artik cloud fingerprint is NOT Validated yet, and really MUST be unless you want your house to be vulnerable!**

At the time of developing Artik Clouds Google Home integration only seems to support turn on/off and setLevel from Google Home.



Artik Cloud is capable of much more, but not through Google Home. 

A quick video of this working is at https://www.youtube.com/watch?v=dM3_Hb11DmU&t=1s

NOTE: once you have all of the information you need please edit 
WiFi.cpp to change  
  #define WiFi_SID YOUR_WIFI_SID  
  #define WiFi_pwd YOUR_WIFI_PWD  

ESP8266-ArtikIntegration.ino to change  
  #define DEVICE_ID YOUR_DEVICE_ID  
  #define DEVICE_TOKEN YOUR_DEVICE_TOKEN  


You will also need 
*	the ArduinoJson library for the Json parsing  
*	the WebSockets library  