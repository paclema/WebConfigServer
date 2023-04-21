#include "WebConfigNetwork.h"

WebConfigNetwork::WebConfigNetwork(WebConfigNetworkObserver* observer): 
  networkObserver(observer) {

}

#ifdef ESP32
void WebConfigNetwork::WiFiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  WebConfigNetwork* self = (WebConfigNetwork*) arg;
  log_i("[WebConfigNetwork-event] event_id: %d", event_id);

  switch (event_id) {
    case WIFI_EVENT_STA_CONNECTED: {
      log_i("WIFI CONNECTED");
      log_i("Connecting to %s...\n",ssid_name.c_str());

      if (self->networkObserver) {
          self->networkObserver->onNetworkConnected();
        }
      break;
      }
    case IP_EVENT_STA_GOT_IP:{
      Serial.print("Obtained IP address: ");
      ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
      log_i("[ipaddr][%s]", IPAddress(event->ip_info.ip.addr).toString());
      log_i("[ipaddr][%s]", WiFi.localIP().toString());
      break;
      }
    case WIFI_EVENT_STA_DISCONNECTED:{
      log_i("WIFI DISCONNECTED ");
      ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
      log_i("[ipaddr][%s]", IPAddress(event->ip_info.ip.addr).toString());
      log_i("[ipaddr][%s]", WiFi.localIP().toString());
      break;
      }
    default:
      break;
  }
}
#endif


void WebConfigNetwork::restart(void){

  // WiFi setup:
  // WiFi.disconnect(true);        // close old connections
  // #ifndef DISABLE_WEBCONFIG_MQTT
  // if (mqtt.isConnected() ) mqtt.disconnect();
  // #endif
  
  #ifdef ESP32
    WiFi.setHostname(hostname.c_str());
    WiFi.mode(WIFI_MODE_APSTA);
  #elif defined(ESP8266)
    WiFi.hostname(hostname);
    // WiFi.mode(WIFI_STA);
    WiFi.mode(WIFI_AP_STA);
  #endif



  // Client Wifi config:
  if (ssid_name!=NULL && ssid_password!=NULL){

    Serial.printf("Connecting to %s...\n",ssid_name.c_str());

    // wifiMulti.addAP(ssid_name.c_str(),ssid_password.c_str());    // Using wifiMulti
    WiFi.begin(ssid_name.c_str(),ssid_password.c_str());      // Connecting just to one ap


    #ifdef ESP32
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WebConfigNetwork::WiFiEventHandler, this);
    esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &WebConfigNetwork::WiFiEventHandler, this);
    #elif defined(ESP8266)
    int retries = 0;
    // while ((wifiMulti.run() != WL_CONNECTED)) {   // Using wifiMulti
    while (WiFi.status() != WL_CONNECTED) {    // Connecting just to one ap
      if (connection_retries == 0) Serial.print('.');
      else Serial.printf(" %d of %d \n", retries, connection_retries);
      delay(500);
      retries++;
      if (connection_retries != 0 && (retries > connection_retries)) break;
    }
    if ((connection_retries != 0 && (retries <= connection_retries)) || connection_retries == 0) {
      Serial.print("\n\nConnected to ");Serial.print(WiFi.SSID());
      Serial.print("\nIP address:\t");Serial.println(WiFi.localIP());
      
      // TODO: change this call to be called into wifi event handler
      networkObserver->onNetworkConnected();

    } else {Serial.print("\n\nNot Connected to ");Serial.print(ssid_name);Serial.println(" max retries reached.");}
    #endif

  }


  // Config access point:
  bool APEnabled = false;
  uint8_t channelSTA;
  #ifdef ESP32
    wifi_ap_record_t staConfig;
    esp_wifi_sta_get_ap_info(&staConfig);
    channelSTA = staConfig.primary;
  #elif defined(ESP8266)
    channelSTA = wifi_get_channel();
  #endif


  if (ap_name!=NULL && ap_password!=NULL){
      Serial.print("Setting soft-AP... ");
      
      if (enable_NAT){
        APEnabled = WiFi.softAP(ap_name.c_str(),
          ap_password.c_str());
      } else {
        APEnabled = WiFi.softAP(ap_name.c_str(),
          ap_password.c_str(),
          // If STA connected, use the same channel instead configured one:
          (WiFi.isConnected() && channelSTA) ? channelSTA : ap_channel,  
          ap_ssid_hidden,
          ap_max_connection);
      }
      Serial.println(APEnabled ? "Ready" : "Failed!");
      IPAddress myIP = WiFi.softAPIP();
      Serial.print(ap_name);Serial.print(" AP IP address: ");
      Serial.println(myIP);

  }

  // Enable NAPT:
  #if ESP32 && IP_NAPT
    if (enable_NAT){
      if (WiFi.isConnected() && APEnabled) {
        esp_err_t err = WebConfigNetwork::enableNAT();
        if (err == ESP_OK) Serial.println("NAT configured and enabled");
        else Serial.printf("Error configuring NAT: %s\n", esp_err_to_name(err));
      } else Serial.printf("Error configuring NAT: STA or AP not working\n");
    }
  #endif

  // Configure device hostname:
  if (hostname){
    if (MDNS.begin(hostname.c_str())) Serial.println("mDNS responder started");
    else Serial.println("Error setting up MDNS responder!");
  }

};

#if ESP32 && IP_NAPT
esp_err_t WebConfigNetwork::enableNAT(void){

  // Give DNS servers to AP side:
  esp_err_t err;
  tcpip_adapter_dns_info_t ip_dns_main;
  tcpip_adapter_dns_info_t ip_dns_backup;


  err = tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP); if (err != ESP_OK) return err;

  err = tcpip_adapter_get_dns_info(TCPIP_ADAPTER_IF_STA, ESP_NETIF_DNS_MAIN, &ip_dns_main); if (err != ESP_OK) return err;
  err = tcpip_adapter_get_dns_info(TCPIP_ADAPTER_IF_STA, ESP_NETIF_DNS_BACKUP, &ip_dns_backup); if (err != ESP_OK) return err;

  err = tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_AP, ESP_NETIF_DNS_MAIN, &ip_dns_main); if (err != ESP_OK) return err;
  // Serial.printf("\ntcpip_adapter_set_dns_info ESP_NETIF_DNS_MAIN: err %s . ip_dns:" IPSTR, esp_err_to_name(err), IP2STR(&ip_dns_main.ip.u_addr.ip4)) ;

  dhcps_offer_t opt_val = OFFER_DNS; // supply a dns server via dhcps
  tcpip_adapter_dhcps_option(ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &opt_val, 1);

  err = tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP); if (err != ESP_OK) return err;


  // Enable NAT:
  ip_napt_enable(WiFi.softAPIP(), 1);

  // Example port mapping to stations:
  // Mapping Webserver: (of an sta connected to this ap)
  IPAddress ap_ip = WiFi.localIP();
  ip_portmap_add(PROTO_TCP,ap_ip, 8080,IPAddress(192, 168, 4, 2), 80 );
  ip_portmap_add(PROTO_UDP,ap_ip, 8080,IPAddress(192, 168, 4, 2), 80 );
  // Mapping WebSockets:
  ip_portmap_add(PROTO_TCP,ap_ip, 94,IPAddress(192, 168, 4, 2), 94 );
  ip_portmap_add(PROTO_UDP,ap_ip, 94,IPAddress(192, 168, 4, 2), 94 );

  return err;
}

void WebConfigNetwork::handleAPStations(void){
  AP_clients = WiFi.softAPgetStationNum();

  if (AP_clients_last != AP_clients){
    Serial.printf("Stations connected to AP: %d\n", AP_clients);
    AP_clients_last = AP_clients;

    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;
  
    memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
    memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));

    // delay(500);   // To give time to AP to provide IP to the new station
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);
  
    for (int i = 0; i < adapter_sta_list.num; i++) {
      tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
      Serial.printf("\t - Station %d MAC: ", i);
      for(int i = 0; i< 6; i++){
        Serial.printf("%02X", station.mac[i]);  
        if(i<5)Serial.print(":");
      }
      Serial.printf("  IP: " IPSTR, IP2STR(&station.ip));
      Serial.println();
    }
  }
}
#endif


void WebConfigNetwork::loop(void){

  // Handle stations connection to the AP:
  #if ESP32 && IP_NAPT
  currentLoopMillis = millis();
    if(currentLoopMillis - previousHandleAPMillis > (unsigned)3000) {
      handleAPStations();
      previousHandleAPMillis = currentLoopMillis;
    }
  #endif

};


void WebConfigNetwork::parseWebConfig(JsonObjectConst configObject){
  // Network object:
  this->ap_name = configObject["AP_SSID"] | "iotdevice";
  this->ap_password = configObject["AP_password"] | "iotdevice";
  this->ap_channel = configObject["AP_channel"] | 6;
  this->ap_ssid_hidden = configObject["AP_SSID_hidden"] | false;
  this->ap_max_connection = configObject["AP_max_connection"] | 4;
  this->ssid_name = configObject["WiFi_SSID"] | "SSID_name";
  this->ssid_password = configObject["WiFi_password"] | "SSID_password";
  this->connection_retries = configObject["connection_retries"] | 0;
  this->ip_address = configObject["ip_address"] | "192.168.1.2";
  this->subnet = configObject["subnet"] | "255.255.255.0";
  this->dns_server = configObject["dns_server"] | "192.168.1.1";
  this->hostname = configObject["hostname"] | "iotdevice.local";
  this->enable_NAT = configObject["enable_NAT"] | false;

  // serializeJsonPretty(configObject, Serial);
};