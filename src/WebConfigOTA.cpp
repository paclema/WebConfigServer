#include "WebConfigOTA.h"


void WebConfigOTA::init(void) {
  // Port defaults to 8266
  #ifdef ESP32
  ArduinoOTA.setPort(3232);
  #elif defined(ESP8266)
  ArduinoOTA.setPort(8266);
  #endif

  // Hostname defaults to esp8266-[ChipID]
  if (hostnameOTA != "") {
    char hostname[sizeof(this->hostnameOTA)];
    hostnameOTA.toCharArray(hostname, sizeof(hostname));
    ArduinoOTA.setHostname(hostname);
  }

  ArduinoOTA.begin();
  ArduinoOTA.onStart([]() {
    LittleFS.end();
    Serial.println("\n OTA update started... ");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // Log.info("OTA Progress: %i%%", (progress / (total / 100)));
    Serial.printf("OTA Progress: %i%%\n", (progress / (total / 100)));
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("OTA update finished!");
  });

};

void WebConfigOTA::parseWebConfig(JsonObjectConst configObject){
  this->enabled = configObject["enabled"] | false;
};