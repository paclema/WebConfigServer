#ifndef WebConfigMQTT_H
#define WebConfigMQTT_H

#include <Arduino.h>

#include "IWebConfig.h"

#include <MQTTClient.h>


class WebConfigMQTT: public IWebConfig {
private:
  MQTTClient mqttClient;

public:

  MQTTClient * getMQTTClient(void) { return this->&mqttClient; };
  
  void setup(void) {
    Serial.printf("WebConfigMQTT::setup()\n");
    mqttClient.setup();
  }

  void loop(void) {
    Serial.printf("WebConfigMQTT::loop()\n");
    mqttClient.loop();
  }

  void parseWebConfig(JsonObjectConst configObject) {
    Serial.printf("WebConfigMQTT::parseWebConfig()\n");
    mqttClient.setConfig(configObject);
  }

  void setMQTTClientId(std::string client_id) { 
    Serial.printf("WebConfigMQTT::setMQTTClientId()\n");
    mqttClient.setMQTTClientId(client_id); 
  }

  std::string getBaseTopic(void) { 
    Serial.printf("WebConfigMQTT::getBaseTopic()\n");
    return mqttClient.getBaseTopic(); 
  }

};

#endif
