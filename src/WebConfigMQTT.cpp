#include "WebConfigMQTT.h"

void WebConfigMQTT::setup(void){


  if (enable_certificates){

  #ifdef ESP32
    // Load certificate file:
    // But you must convert it to .der
    // openssl x509 -in ./certs/IoLed_controller/client.crt -out ./certs/IoLed_controller/cert.der -outform DER

    File cert = LittleFS.open(cert_file, "r"); //replace cert.crt with your uploaded file name
    if (!cert) Serial.println("Failed to open cert file ");
    else Serial.println("Success to open cert file");

    if (wifiClientSecure.loadCertificate(cert, cert.size())) Serial.println("cert loaded");
    else Serial.println("cert not loaded");
    cert.close();

    // Load private key:
    // But you must convert it to .der
    // openssl rsa -in ./certs/IoLed_controller/client.key -out ./certs/IoLed_controller/private.der -outform DER
    File private_key = LittleFS.open(key_file, "r");
    if (!private_key) Serial.println("Failed to open key file ");
    else Serial.println("Success to open key file");

    if (wifiClientSecure.loadPrivateKey(private_key, private_key.size())) Serial.println("key loaded");
    else Serial.println("key not loaded");
    private_key.close();

    // Load CA file:
    File ca = LittleFS.open(ca_file, "r");
    if (!ca) Serial.println("Failed to open CA file ");
    else Serial.println("Success to open CA file");

    if (wifiClientSecure.loadCACert(ca, ca.size())) Serial.println("CA loaded");
    else Serial.println("CA not loaded");
    ca.close();

  #elif defined(ESP8266)

    // Cert FIle
    File cert = LittleFS.open(cert_file, "r");
    if(!cert) Serial.println("Couldn't load cert");
    else {
      size_t certSize = cert.size();
      client_cert = (char *)malloc(certSize);
      if (certSize != cert.readBytes(client_cert, certSize)) {
        Serial.println("Loading client cert failed");
      } else {
        Serial.println("Loaded client cert");
        clientCert = new BearSSL::X509List(client_cert);
      }
      free(client_cert);
      cert.close();
    }

    // Key File
    File key = LittleFS.open(key_file, "r");
    if(!key) Serial.println("Couldn't load key");
    else {
      size_t keySize = key.size();
      client_key = (char *)malloc(keySize);
      if (keySize != key.readBytes(client_key, keySize)) {
        Serial.println("Loading key failed");
      } else {
        Serial.println("Loaded key");
        clientKey = new BearSSL::PrivateKey(client_key);
      }
      free(client_key);
      key.close();
    }

    wifiClientSecure.setClientRSACert(clientCert, clientKey);

    // CA File
    File ca = LittleFS.open(ca_file, "r");
    if(!ca) Serial.println("Couldn't load CA cert");
    else {
      size_t certSize = ca.size();
      ca_cert = (char *)malloc(certSize);
      if (certSize != ca.readBytes(ca_cert, certSize)) {
        Serial.println("Loading CA cert failed");
      } else {
        Serial.println("Loaded CA cert");
        rootCert = new BearSSL::X509List(ca_cert);
        wifiClientSecure.setTrustAnchors(rootCert);
      }
      free(ca_cert);
      ca.close();
  }

  #endif

  }

  if (enable_certificates){
    if(enable_websockets){
      wsClient = new WebSocketClient(wifiClientSecure, server.c_str(), port);
      wsStreamClient = new WebSocketStreamClient(*wsClient, websockets_path.c_str());
      mqttClient.setClient(*wsStreamClient);
      Serial.printf("Configuring MQTT using certificates and websockets path: %s\n", websockets_path.c_str());
    } else {
      mqttClient.setClient(wifiClientSecure);
      Serial.println("Configuring MQTT using certificates");
      mqttClient.setServer(server.c_str(), port);
    }
  } else {
    if(enable_websockets){
      wsClient = new WebSocketClient(wifiClient, server.c_str(), port);
      wsStreamClient = new WebSocketStreamClient(*wsClient, websockets_path.c_str());
      mqttClient.setClient(*wsStreamClient);
      Serial.println("Configuring MQTT using websockets");
    } else {
      mqttClient.setClient(wifiClient);
      Serial.println("Configuring MQTT using websockets without certificates");
      mqttClient.setServer(server.c_str(), port);
    }
  }

  mqttClient.setCallback(WebConfigMQTT::callbackMQTT);

};

void WebConfigMQTT::disconnect() {
  // Close old possible conections
  if (mqttClient.connected() ) mqttClient.disconnect();

  // Delete client pointers
  if (wsStreamClient){
    wsStreamClient->flush();
    wsStreamClient->stop();
    delete wsStreamClient;
    wsStreamClient = nullptr;
  }
  if (wsClient){
    wsClient->stop();
    delete wsClient;
    wsClient = nullptr;
  }

  //Renew the connection
  WebConfigMQTT::setup();

};

void WebConfigMQTT::reconnect() {
  // Loop until we're reconnected
  if (currentLoopMillis - previousMqttReconnectionMillis > mqttReconnectionTime){

    
    if (!mqttClient.connected() && ( mqttMaxRetries <= 0 || (mqttRetries <= mqttMaxRetries)) ) {
      WebConfigMQTT::disconnect();
      
      bool mqttConnected = false;
      Serial.print("Attempting MQTT connection... ");
      
      String mqttWillTopic = base_topic_pub + "connected";
      uint8_t mqttWillQoS = 2;
      boolean mqttWillRetain = true;
      String mqttWillMessage = "false";

      if (enable_user_and_pass)
        mqttConnected = mqttClient.connect(id_name.c_str(),
                                            user_name.c_str(),
                                            user_password.c_str(),
                                            mqttWillTopic.c_str(),
                                            mqttWillQoS,
                                            mqttWillRetain,
                                            mqttWillMessage.c_str());
      else
        mqttConnected = mqttClient.connect(id_name.c_str(),
                                            mqttWillTopic.c_str(),
                                            mqttWillQoS,
                                            mqttWillRetain,
                                            mqttWillMessage.c_str());

      if (mqttConnected) {
        Serial.println("connected");
        // Once connected, publish an announcement...
        String topic_connected_pub = base_topic_pub + "connected";
        String msg_connected ="true";
        mqttClient.publish(topic_connected_pub.c_str(), msg_connected.c_str(), true);
        // ... and resubscribe
        String base_topic_sub = base_topic_pub + "#";
        mqttClient.subscribe(base_topic_sub.c_str());

        u_int8_t sub_topicSize = MQTT_TOPIC_MAX_SIZE_LIST;
        for (unsigned int i = 0; i < sub_topicSize ; i++){
          if (sub_topic[i] == "") break;
          Serial.printf("MQTT subscribing %d of %d topic %s: %s\n",
              i, sub_topicSize, sub_topic[i].c_str(),
              mqttClient.subscribe(sub_topic[i].c_str()) ? "ok" : "failed");
        }

        mqttRetries = 0;
        Serial.printf("Time to connect MQTT client: %.2fs\n",(float)(millis() - connectionTime)/1000);

      } else {
        Serial.printf("failed, rc=%d try again in %ds: %d/%s\n", 
                        mqttClient.state(), mqttReconnectionTime/1000, mqttRetries, 
                        mqttMaxRetries <= 0 ? "-" : String(mqttMaxRetries));
      }
      previousMqttReconnectionMillis = millis();
      mqttRetries++;
    }
  }
};


void WebConfigMQTT::loop(void){
  currentLoopMillis = millis();
  
  if ( this->enabled) {
    if ( !this->mqttClient.connected() &&
          reconnect_mqtt && 
          WiFi.status() == WL_CONNECTED ) {
            connectionTime = currentLoopMillis;
            WebConfigMQTT::reconnect();
      }

    if (mqttClient.connected()) mqttClient.loop();
  }

};


void WebConfigMQTT::parseWebConfig(JsonObjectConst configObject){
  this->enabled = configObject["enabled"] | false;
  this->server = configObject["server"] | "server_address";
  this->port = configObject["port"] | 8888;
  this->id_name = configObject["id_name"] | "iotdevice";
  this->reconnect_mqtt = configObject["reconnect_mqtt"] | false;
  this->mqttMaxRetries = configObject["reconnect_retries"] | 10;
  this->mqttReconnectionTime = configObject["reconnect_time_ms"] | 10000;
  this->enable_user_and_pass = configObject["enable_user_and_pass"] | false;
  this->user_name = configObject["user_name"] | "user_name";
  this->user_password = configObject["user_password"] | "user_password";
  this->enable_certificates = configObject["enable_certificates"] | false;
  this->ca_file = configObject["ca_file"] | "certs/ca.crt";
  this->cert_file = configObject["cert_file"] | "certs/client.crt";
  this->key_file = configObject["key_file"] | "certs/client.key";
  this->enable_websockets = configObject["enable_websockets"] | false;
  this->websockets_path = configObject["websockets_path"] | "/";

  if (configObject["pub_topic"].size() > 0)
    for (unsigned int i = 0; i < configObject["pub_topic"].size(); i++)
      this->sub_topic[i] = configObject["pub_topic"][i].as<String>();
  else
    this->sub_topic[0] = configObject["sub_topic"].as<String>();

  if (configObject["sub_topic"].size() > 0)
    for (unsigned int i = 0; i < configObject["sub_topic"].size(); i++)
      this->sub_topic[i] = configObject["sub_topic"][i].as<String>();
  else
    this->sub_topic[0] = configObject["sub_topic"].as<String>();


  uint32_t chipId = 0;
  #ifdef ESP32
  for(int i=0; i<17; i=i+8) {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
  // Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  // Serial.printf("This chip has %d cores\n", ESP.getChipCores());
  // Serial.print("Chip ID: "); Serial.println(chipId);
  #else degined(ESP8266)
    chipId = ESP.getChipId();
  #endif

  this->base_topic_pub = "/" + id_name + "/" + chipId + "/";
};