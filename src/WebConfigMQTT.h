#ifndef WebConfigMQTT_H
#define WebConfigMQTT_H

#include <Arduino.h>

#include "IWebConfig.h"

#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#define MQTT_TOPIC_MAX_SIZE_LIST 10

#ifdef ESP32
  #include <SPIFFS.h>
#elif defined(ESP8266)
  #include <FS.h>
#endif


class WebConfigMQTT: public IWebConfig{

private:

  WiFiClientSecure wifiClientSecure;    // To use with mqtt and certificates
  WiFiClient wifiClient;                // To use with mqtt without certificates
  PubSubClient mqttClient;
  unsigned long previousMqttReconnectionMillis = millis();
  unsigned int mqttReconnectionTime = 10000;
  int mqttRetries = 0;
  int mqttMaxRetries = 10;

  unsigned long currentLoopMillis = 0;
  unsigned long previousMQTTPublishMillis = 0;
  unsigned long connectionTime = millis();


  bool enabled;
  String server;
  int port;
  String id_name;
  String base_topic_pub;
  bool reconnect_mqtt;
  bool enable_user_and_pass;
  String user_name;
  String user_password;
  bool enable_certificates;
  String ca_file;
  String cert_file;
  String key_file;
  String pub_topic[MQTT_TOPIC_MAX_SIZE_LIST];
  String sub_topic[MQTT_TOPIC_MAX_SIZE_LIST];

  int publish_time_ms = 2000;

public:

  void setup(void);
  void reconnect(void);
  void loop(void);

  bool isEnabled(void) { return enabled; }
  bool isConnected(void) { return mqttClient.connected(); }
  bool getReconnect(void) { return reconnect_mqtt; }
  int getPublishTime(void) { return publish_time_ms; }
  String getBaseTopic(void) { return base_topic_pub; }

  void setPublishTime(int ms) { publish_time_ms = ms; }
  PubSubClient *getMQTTClient(void) { return &mqttClient; }

  void parseWebConfig(JsonObjectConst configObject);


  
  static void callbackMQTT(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    char buff[length + 1];
    for (unsigned int i = 0; i < length; i++) {
      //Serial.print((char)payload[i]);
      buff[i] = (char)payload[i];
    }


    buff[length] = '\0';
    String message(buff);

    Serial.print(message);
    Serial.println();

  /*
    if (strcmp(topic, "/lamp") == 0) {
      //Lamp color request:
      if (message.equals("red")){
        Serial.println("Turning lamp to red");
        //colorWipe(strip.Color(255, 0, 0), 10);
      }
      else if (strcmp(buff, "blue") == 0){
          Serial.println("Turning lamp to blue");
          //colorWipe(strip.Color(0, 0, 255), 10);
      } else if (message.equals("green")){
          Serial.println("Turning lamp to green");
          //colorWipe(strip.Color(0, 255, 0), 10);
      }
      //client.publish((char*)"/lamp",(char*)"color changed");
    }
  */

    Serial.print("Heap: "); Serial.println(ESP.getFreeHeap()); //Low heap can cause problems
    ///

  }


};

#endif
