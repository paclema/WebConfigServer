#ifndef WebConfigNetwork_H
#define WebConfigNetwork_H

#include <Arduino.h>

#include "IWebConfig.h"

#ifdef ESP32
  #include <WiFi.h>
  #include "esp_wifi.h"
  #include <esp_event.h>
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

#include "WebConfigNetworkObserver.h"


class WebConfigNetwork: public IWebConfig {
protected:
  WebConfigNetworkObserver* networkObserver;
private:
  String ap_name;
  String ap_password;
  int ap_channel;
  bool ap_ssid_hidden;
  int ap_max_connection;
  String ssid_name;
  String ssid_password;
  String ip_address;
  String subnet;
  String dns_server;
  String hostname;
  bool enable_NAT;

// WiFiMulti
#ifdef ESP32
    WiFiMulti wifiMulti;
#elif defined(ESP8266)
    ESP8266WiFiMulti wifiMulti;
#endif


#ifdef ESP32
  #ifdef IP_NAPT
  uint8_t AP_clients = 0;
  uint8_t AP_clients_last = AP_clients;
  unsigned long currentLoopMillis = 0;
  unsigned long previousHandleAPMillis = 0;

  esp_err_t enableNAT(void);
  void handleAPStations(void);
  #endif
  
  static void WiFiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);


#elif defined(ESP8266)
  WiFiEventHandler staConnectedHandler;
  WiFiEventHandler staDisconnectedHandler;
  WiFiEventHandler staGotIPHandler;

  void onStationModeConnectedHandler(const WiFiEventStationModeConnected& event) {
    char str_bssid[20];
    sprintf(str_bssid, "%02x:%02x:%02x:%02x:%02x:%02x", event.bssid[0], event.bssid[1], event.bssid[2], event.bssid[3], event.bssid[4], event.bssid[5]);
    Serial.printf("Conected to SSID: %s BSSID: %s\n", event.ssid.c_str(), str_bssid);
  }

  void onStationModeGotIPHandler(const WiFiEventStationModeGotIP& event) {
    Serial.printf("Got IP: %s\n", event.ip.toString().c_str());
    if (networkObserver) {
      networkObserver->onNetworkConnected();
    }
  }

  void onStationModeDisconnectedHandler(const WiFiEventStationModeDisconnected& event) {
    char str_bssid[20];
    sprintf(str_bssid, "%02x:%02x:%02x:%02x:%02x:%02x", event.bssid[0], event.bssid[1], event.bssid[2], event.bssid[3], event.bssid[4], event.bssid[5]);
    Serial.printf("Disconnected from SSID: %s BSSID: %s\n", event.ssid.c_str(), str_bssid);
    Serial.printf("Disconnected Reason: %d\n",event.reason);
    if (this->networkObserver) {
      this->networkObserver->onNetworkDisconnected();
    }
  }

#endif


public:
  WebConfigNetwork(WebConfigNetworkObserver* observer);

  void restart(void);
  void loop(void);

  void parseWebConfig(JsonObjectConst configObject);

};

#endif
