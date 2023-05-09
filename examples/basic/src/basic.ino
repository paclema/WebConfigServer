#include <Arduino.h>


// Device configurations
unsigned long currentLoopMillis = 0;
unsigned long previousPublishMillis = 0;
unsigned long previousMainLoopMillis = 0;


// WebConfigServer Configuration
#include "WebConfigServer.h"
WebConfigServer config;   // <- global configuration object


// Websocket functions to publish:
String getLoopTime(){ return String(currentLoopMillis - previousMainLoopMillis);}
String getRSSI(){ return String(WiFi.RSSI());}
String getHeapFree(){ return String((float)GET_FREE_HEAP/1000);}

#include <MQTTClient.h>
MQTTClient *mqttClient;


void callBeforeDeviceOff(){
  Serial.println("Going to sleep!");
};


void setup() {
  Serial.begin(115200);

  config.begin();
 
  config.addDashboardObject("heap_free", getHeapFree);
  config.addDashboardObject("loop", getLoopTime);
  config.addDashboardObject("RSSI", getRSSI);

  config.setPreSleepRoutine(callBeforeDeviceOff);

  mqttClient = config.getMQTTClient();

  Serial.println("###  Looping time\n");

}

void loop() {

  currentLoopMillis = millis();

  config.loop();

  // Main Loop:
  if( mqttClient->connected() && (currentLoopMillis - previousPublishMillis > 2000)) {
    previousPublishMillis = currentLoopMillis;

    String topic = config.getDeviceTopic() + "/test";
    mqttClient->publish(topic.c_str(),"Message from main loop");
    
    Serial.println("Message published");
  }

previousMainLoopMillis = currentLoopMillis;
}
