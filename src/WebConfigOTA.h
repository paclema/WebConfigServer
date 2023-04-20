#ifndef WebConfigOTA_H
#define WebConfigOTA_H

#include <Arduino.h>

// OTA Includes
#include <ArduinoOTA.h>
#include <LittleFS.h>

#include "IWebConfig.h"

class WebConfigOTA: public IWebConfig{

private:
  bool enabled = false;
  String hostnameOTA = "";


public:

  void init(void);
  void handle(void) { ArduinoOTA.handle(); }

  void setHostname(String hostname){ this->hostnameOTA = hostname;}

  bool isEnabled(void) { return this->enabled; }

  void parseWebConfig(JsonObjectConst configObject);


};

#endif
