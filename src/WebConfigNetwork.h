#ifndef WebConfigNetwork_H
#define WebConfigNetwork_H

#include <Arduino.h>

#include "IWebConfig.h"

#ifdef ESP32
  #include <WiFi.h>
  #include "esp_wifi.h"
  #include <WiFiMulti.h>
  #include <ESPmDNS.h>

  #define PROTO_TCP 6
  #define PROTO_UDP 17

  #if !IP_NAPT
    #warning "IP_NAPT is not available with this configuration."
  #else
    #include "lwip/lwip_napt.h"
  #endif

#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WiFiMulti.h>
  #include <ESP8266mDNS.h>
#endif

class WebConfigNetwork: public IWebConfig{

private:

  String ap_name;
  String ap_password;
  int ap_channel;
  bool ap_ssid_hidden;
  int ap_max_connection;
  String ssid_name;
  String ssid_password;
  int connection_retries;
  String ip_address;
  String subnet;
  String dns_server;
  String hostname;
  bool enable_NAT;

#if ESP32 && IP_NAPT
  uint8_t AP_clients = 0;
  uint8_t AP_clients_last = AP_clients;
  unsigned long currentLoopMillis = 0;
  unsigned long previousHandleAPMillis = 0;


  esp_err_t enableNAT(void);
  void handleAPStations(void);
#endif


public:

  void restart(void);

  void loop(void);

  void parseWebConfig(JsonObjectConst configObject);

};

#endif