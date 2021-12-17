#ifndef IWebConfig_H
#define IWebConfig_H

#include <Arduino.h>
#include <ArduinoJson.h>
#ifdef ESP32
  #include <SPIFFS.h>
#elif defined(ESP8266)
  #include <FS.h>
#endif

#ifndef CONFIG_FILE
  #define CONFIG_FILE "/config/config.json"
#endif

class IWebConfig {

public:
  String nameConfigObject;

  virtual void parseWebConfig(JsonObjectConst configObject) = 0;

  // virtual void parseWebConfig(const JsonDocument& doc){
    //   Serial.println("parsing on BaseObject...");
    // };

  bool saveWebConfig(JsonObject newConfigObject) {
    // Serial.println("Received WebConfig to save: ");
    // serializeJsonPretty(newConfigObject, Serial);
    // Serial.println();

    File fileIn = SPIFFS.open(CONFIG_FILE, "r");
    DynamicJsonDocument doc(CONFIG_JSON_SIZE);
    DeserializationError error = deserializeJson(doc, fileIn);
    if (error) {
      Serial.printf("Failed to read %s", CONFIG_FILE);
      return false;
    }
    fileIn.close();

    // Iterate throught received JsonObject to save
    for (JsonObject::iterator it=newConfigObject.begin(); it!=newConfigObject.end(); ++it) {
      // Serial.print(it->key().c_str());
      // Serial.print(": ");

      if (it->value().is<int>()) {
          int value = it->value().as<int>();
          // Serial.print(value);
          // Serial.print(" --> (int) from ");
          // Serial.print((int)doc[nameConfigObject][it->key().c_str()]);
          doc[nameConfigObject][it->key().c_str()] = value;
          // Serial.print(" --> now ");
          // Serial.print((int)doc[nameConfigObject][it->key().c_str()]);
      } else if (it->value().is<float>()) {
          float value = it->value().as<float>();
          doc[nameConfigObject][it->key().c_str()] = value;
      } else if (it->value().is<bool>()) {
          bool value = it->value().as<bool>();
          doc[nameConfigObject][it->key().c_str()] = value;
      } else if (it->value().is<const char*>()) {
          const char* value = it->value().as<const char*>();
          doc[nameConfigObject][it->key().c_str()] = value;
      } else if (it->value().is<String>()) {
          String value = it->value().as<String>();
          doc[nameConfigObject][it->key().c_str()] = value;
      } else {
          Serial.printf("%s contains WebConfig value type not suported to be save\n", it->key().c_str());
          return false;
      }

      // Serial.println();
    }
    
    SPIFFS.remove(CONFIG_FILE);
    File fileOut = SPIFFS.open(CONFIG_FILE, "w");
    if (!fileOut) {
      Serial.printf("Failed to open %s", CONFIG_FILE);
      return false;
    }
    if (serializeJson(doc, fileOut) == 0) {
      Serial.printf("Failed to write %s", CONFIG_FILE);
      return false;
    }

    fileOut.close();
    return true;
  };

};
#endif
