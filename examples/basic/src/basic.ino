// basic
// -----
//
// This example shows how to use the MQTTClient to:
// * Create a WebConfigServer config object
// * Publish a message to a MQTT topic using the WebConfigServer MQTTClient client
// * Use WebConfigServer to get device MQTT base topic for publishing
//

#include <Arduino.h>

// WebConfigServer Configuration
#include "WebConfigServer.h"
WebConfigServer config;   // <- global configuration object

#include <MQTTClient.h>
MQTTClient *mqttClient;


// Device configurations
unsigned long currentLoopMillis = 0;
unsigned long previousPublishMillis = 0;
unsigned long previousMainLoopMillis = 0;
unsigned long publishTime = 2000; // milliseconds for publishing to MQTT broker from main loop


// Websocket functions to publish:
String getLoopTime(){ return String(currentLoopMillis - previousMainLoopMillis);}
String getRSSI(){ return String(WiFi.RSSI());}
String getHeapFree(){ return String((float)GET_FREE_HEAP/1000);}

// Pre Sleep Routine:
void callBeforeDeviceOff(){
  Serial.println("Going to sleep!");
};



void setup() {
  Serial.begin(115200);

  config.begin();

  // Add dashboard plots in WebConfigServer:
  config.addDashboardObject("heap_free", getHeapFree);
  config.addDashboardObject("loop", getLoopTime);
  config.addDashboardObject("RSSI", getRSSI);

  // Configure the pre sleep routine:
  config.setPreSleepRoutine(callBeforeDeviceOff);

  mqttClient = config.getMQTTClient();

  Serial.println("###  Looping time\n");
}

void loop() {

  currentLoopMillis = millis();

  // WebConfigServer Loop:
  config.loop();

  // Main Loop:
  if( mqttClient->connected() && (currentLoopMillis - previousPublishMillis > publishTime)) {
    previousPublishMillis = currentLoopMillis;

    String topic = config.getDeviceTopic() + "/test";
    mqttClient->publish(topic.c_str(),"Message from main loop");
    Serial.printf("Message from main loop published to topic %s\n", topic.c_str());
    
  }

  previousMainLoopMillis = currentLoopMillis;
}
