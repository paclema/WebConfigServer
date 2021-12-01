#include "WebConfigMQTT.h"


void WebConfigMQTT::setup(void){

  if (enable_certificates){
    mqttClient.setClient(wifiClientSecure);
    Serial.println("Configuring MQTT using certificates");
  } else {
    mqttClient.setClient(wifiClient);
    Serial.println("Configuring MQTT without certificates");
  }

  mqttClient.setServer(server.c_str(), port);
  mqttClient.setCallback(WebConfigMQTT::callbackMQTT);

  if (enable_certificates){
    // Load certificate file:
    // But you must convert it to .der
    // openssl x509 -in ./certs/IoLed_controller/client.crt -out ./certs/IoLed_controller/cert.der -outform DER
    File cert = SPIFFS.open(cert_file, "r"); //replace cert.crt with your uploaded file name
    if (!cert) Serial.println("Failed to open cert file ");
    else Serial.println("Success to open cert file");

    // if (wifiClientSecure.loadCertificate(cert, cert.size())) Serial.println("cert loaded");
    // else Serial.println("cert not loaded");
    cert.close();

    // Load private key:
    // But you must convert it to .der
    // openssl rsa -in ./certs/IoLed_controller/client.key -out ./certs/IoLed_controller/private.der -outform DER
    File private_key = SPIFFS.open(key_file, "r");
    if (!private_key) Serial.println("Failed to open key file ");
    else Serial.println("Success to open key file");

    // if (wifiClientSecure.loadPrivateKey(private_key, private_key.size())) Serial.println("key loaded");
    // else Serial.println("key not loaded");
    private_key.close();

    // Load CA file:
    File ca = SPIFFS.open(ca_file, "r");
    if (!ca) Serial.println("Failed to open CA file ");
    else Serial.println("Success to open CA file");

    // if (wifiClientSecure.loadCACert(ca, ca.size())) Serial.println("CA loaded");
    // else Serial.println("CA not loaded");
    ca.close();
  }

};


void WebConfigMQTT::reconnect() {
  // Loop until we're reconnected
  if (currentLoopMillis - previousMqttReconnectionMillis > mqttReconnectionTime){
    if (!mqttClient.connected() && (mqttRetries <= mqttMaxRetries) ) {
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

        long time_now = millis() - connectionTime;
        mqttRetries = 0;
        Serial.print("Time to connect MQTT client: ");
        Serial.print((float)time_now/1000);
        Serial.println("s");

      } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.print(" try again in ");
        Serial.print(mqttReconnectionTime/1000);
        Serial.print("s: ");
        Serial.print(mqttRetries);
        Serial.print("/");
        Serial.println(mqttMaxRetries);

      }
      previousMqttReconnectionMillis = millis();
      mqttRetries++;
    }
  }
};


void WebConfigMQTT::loop(void){
  currentLoopMillis = millis();
  
  mqttClient.loop();

  /*
  if(mqttClient.connected() && (publish_time_ms != 0) &&
      (currentLoopMillis - previousMQTTPublishMillis > (unsigned)publish_time_ms)) {
    previousMQTTPublishMillis = currentLoopMillis;
    // Here starts the MQTT publish loop configured:

    String topic_pub = base_topic_pub + "status";
    String msg_pub ="{\"connected\":true}";
    mqttClient.publish(topic_pub.c_str(), msg_pub.c_str());
  }
  */

};


void WebConfigMQTT::parseWebConfig(JsonObjectConst configObject){
  this->enabled = configObject["enabled"] | false;
  this->server = configObject["server"] | "server_address";
  this->port = configObject["port"] | 8888;
  this->id_name = configObject["id_name"] | "iotdevice";
  this->reconnect_mqtt = configObject["reconnect_mqtt"] | false;
  this->enable_user_and_pass = configObject["enable_user_and_pass"] | false;
  this->user_name = configObject["user_name"] | "user_name";
  this->user_password = configObject["user_password"] | "user_password";
  this->enable_certificates = configObject["enable_certificates"] | false;
  this->ca_file = configObject["ca_file"] | "certs/ca.crt";
  this->cert_file = configObject["cert_file"] | "certs/client.crt";
  this->key_file = configObject["key_file"] | "certs/client.key";
  for (unsigned int i = 0; i < configObject["pub_topic"].size(); i++) { //Iterate through results
    // this->pub_topic[i] = configObject["pub_topic"][i];  //Implicit cast
    this->pub_topic[i] = configObject["pub_topic"][i].as<String>(); //Explicit cast
  }
  for (unsigned int i = 0; i < configObject["sub_topic"].size(); i++)
    this->sub_topic[i] = configObject["sub_topic"][i].as<String>();

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