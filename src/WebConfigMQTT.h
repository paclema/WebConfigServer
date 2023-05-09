#ifndef WebConfigMQTT_H
#define WebConfigMQTT_H

#include <Arduino.h>

#include "IWebConfig.h"

#include <MQTTClient.h>


class WebConfigMQTT: public IWebConfig {
private:
  MQTTClient mqttClient;

public:
  
  void setup(void) { mqttClient.setup(); }

  #ifdef ESP8266
    void loop(void) { mqttClient.loop(); }
  #endif

  void parseWebConfig(JsonObjectConst configObject) { mqttClient.setConfig(configObject); }

  MQTTClient * getMQTTClient(void) { return &mqttClient; };
  void setMQTTClientId(std::string client_id) { mqttClient.setMQTTClientId(client_id); }
  std::string getBaseTopic(void) { return mqttClient.getBaseTopic(); }
};

#endif // WebConfigMQTT_H
