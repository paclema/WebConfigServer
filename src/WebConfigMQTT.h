#ifndef WebConfigMQTT_H
#define WebConfigMQTT_H

#include <Arduino.h>

#include "IWebConfig.h"

#include <LittleFS.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "WebSocketStreamClient.h"

#define MQTT_TOPIC_MAX_SIZE_LIST 10

#ifdef ESP32
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif


class WebConfigMQTT: public IWebConfig{

private:
#ifdef ESP32
  WiFiClientSecure wifiClientSecure;
#elif defined(ESP8266)
  BearSSL::WiFiClientSecure wifiClientSecure;
  
  char *client_cert = nullptr;
  char *client_key = nullptr;
  char *ca_cert = nullptr;

  BearSSL::X509List *rootCert;
  BearSSL::X509List *clientCert;
  BearSSL::PrivateKey *clientKey;
#endif

  WiFiClient wifiClient;
  WebSocketClient  * wsClient = nullptr;
  WebSocketStreamClient * wsStreamClient = nullptr;
  PubSubClient mqttClient;

  unsigned long previousMqttReconnectionMillis = millis();
  int mqttRetries = 0;

  unsigned long currentLoopMillis = 0;
  unsigned long previousMQTTPublishMillis = 0;
  unsigned long connectionTime = millis();


  bool enabled;
  String server;
  int port;
  String id_name;
  String base_topic_pub;
  bool reconnect_mqtt;
  int mqttMaxRetries;
  unsigned int mqttReconnectionTime;
  bool enable_user_and_pass;
  String user_name;
  String user_password;
  bool enable_certificates;
  String ca_file;
  String cert_file;
  String key_file;
  bool enable_websockets;
  String websockets_path;
  String pub_topic[MQTT_TOPIC_MAX_SIZE_LIST];
  String sub_topic[MQTT_TOPIC_MAX_SIZE_LIST];

public:

  void setup(void);
  void disconnect(void);
  void reconnect(void);
  void loop(void);

  bool isEnabled(void) { return enabled; }
  bool isConnected(void) { return mqttClient.connected(); }
  bool useWebsockets(void) { return enable_websockets; }
  bool getReconnect(void) { return reconnect_mqtt; }
  String getBaseTopic(void) { return base_topic_pub; }

  PubSubClient *getMQTTClient(void) { return &mqttClient; }
  void setMQTTClientId(String client_id) { 
    id_name = client_id;
    StaticJsonDocument<192> docSave;
    docSave["id_name"] = this->id_name;
    WebConfigMQTT::saveWebConfig(docSave.as<JsonObject>());
  }


  void parseWebConfig(JsonObjectConst configObject);

  
  static void callbackMQTT(char* topic, byte* payload, unsigned int length) {
    // Serial.print("Message arrived [");
    // Serial.print(topic);
    // Serial.print("] ");

    // char buff[length + 1];
    // for (unsigned int i = 0; i < length; i++) {
    //   //Serial.print((char)payload[i]);
    //   buff[i] = (char)payload[i];
    // }
    // buff[length] = '\0';

    // String message(buff);
    // Serial.print(message);
    // Serial.println();
  
    Serial.printf("[%lu] +++ MQTT received %s %.*s\n", millis(), topic, length, payload);

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

    Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());

  }


};

#endif
