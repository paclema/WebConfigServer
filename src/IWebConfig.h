#ifndef IWebConfig_H
#define IWebConfig_H

#include <Arduino.h>

#include <ArduinoJson.h>


class IWebConfig {

public:
  String nameConfigObject;

  virtual void parseWebConfig(JsonObjectConst configObject) = 0;

  // virtual void parseWebConfig(const JsonDocument& doc){
    //   Serial.println("parsing on BaseObject...");
    // };

};
#endif
