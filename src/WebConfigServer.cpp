#include "WebConfigServer.h"


WebConfigServer::WebConfigServer(void){
  // Serial.println("WebConfigServer loaded");
  config_status = CONFIG_NOT_LOADED;
}

bool WebConfigServer::begin(void){

  #ifdef ESP32
    if (!SPIFFS.begin(true)) {
      Serial.println("SPIFFS Mount failed");
      config_status = CONFIG_NOT_LOADED;
      return false;
    } else {
      Serial.println("SPIFFS Mount succesfull");
      File root = SPIFFS.open("/");
      File file = root.openNextFile();
      while (file) {
        String fileName = file.name();
        size_t fileSize = file.size();
        Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
        file = root.openNextFile();
      }

      if (SPIFFS.exists(CONFIG_FILE)) {
        Serial.println(); Serial.print(CONFIG_FILE); Serial.println(" exists!");
        loadConfigurationFile(CONFIG_FILE);
      // printFile(CONFIG_FILE);
    } else {
      config_status = CONFIG_NOT_LOADED;
      return false;
      }
    }

  #elif defined(ESP8266)
    if (!SPIFFS.begin()) {
      Serial.println("SPIFFS Mount failed");
      config_status = CONFIG_NOT_LOADED;
      return false;
    } else {
      Serial.println("SPIFFS Mount succesfull");
      Dir dir = SPIFFS.openDir("/");
      while (dir.next()) {
        String fileName = dir.fileName();
        size_t fileSize = dir.fileSize();
        Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
      }

      if (SPIFFS.exists(CONFIG_FILE)) {
        Serial.println(); Serial.print(CONFIG_FILE); Serial.println(" exists!");
        loadConfigurationFile(CONFIG_FILE);
      // printFile(CONFIG_FILE);
    } else {
      config_status = CONFIG_NOT_LOADED;
      return false;
      }
    }
  #endif


  config_status = CONFIG_LOADED;
  return true;

}


// Format bytes:
String WebConfigServer::formatBytes(size_t bytes){
  if (bytes < 1024) {
    return String(bytes) + " B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + " KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + " MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
  }
}


// Saves the web configuration from a POST req to a file
bool WebConfigServer::saveWebConfigurationFile(const char *filename, const JsonDocument& doc){
  // Delete existing file, otherwise the configuration is appended to the file
  Serial.println(F("Saving webconfig file..."));
  SPIFFS.remove(filename);

  // Open file for writing
  // File file = SD.open(filename, FILE_WRITE);
  File file = SPIFFS.open(filename, "w");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return false;
  }

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
    return false;
  }

  // Close the file
  file.close();
  return true;
}


void WebConfigServer::parseConfig(const JsonDocument& doc){

  // serializeJsonPretty(doc, Serial);

  // Parse char network[64]:
  // strlcpy(network.ssid_name, doc["network"]["ssid_name"] | "SSID_name", sizeof(network.ssid_name));

  // Network object:
  network.ap_name = doc["network"]["AP_SSID"] | "iotdevice";
  network.ap_password = doc["network"]["AP_password"] | "iotdevice";
  network.ap_channel = doc["network"]["AP_channel"] | 6;
  network.ap_ssid_hidden = doc["network"]["AP_SSID_hidden"] | false;
  network.ap_max_connection = doc["network"]["AP_max_connection"] | 4;
  network.ssid_name = doc["network"]["WiFi_SSID"] | "SSID_name";
  network.ssid_password = doc["network"]["WiFi_password"] | "SSID_password";
  network.ap_max_connection = doc["network"]["connection_retries"] | 0;
  network.ip_address = doc["network"]["ip_address"] | "192.168.1.2";
  network.subnet = doc["network"]["subnet"] | "255.255.255.0";
  network.dns_server = doc["network"]["dns_server"] | "192.168.1.1";
  network.hostname = doc["network"]["hostname"] | "iotdevice.local";
  network.enable_NAT = doc["network"]["enable_NAT"] | false;


  // MQTT object:
  mqtt.enabled = doc["mqtt"]["enabled"] | false;
  mqtt.server = doc["mqtt"]["server"] | "server_address";
  mqtt.port = doc["mqtt"]["port"] | 8888;
  mqtt.id_name = doc["mqtt"]["id_name"] | "iotdevice";
  mqtt.reconnect_mqtt = doc["mqtt"]["reconnect_mqtt"] | false;
  mqtt.enable_user_and_pass = doc["mqtt"]["enable_user_and_pass"] | false;
  mqtt.user_name = doc["mqtt"]["user_name"] | "user_name";
  mqtt.user_password = doc["mqtt"]["user_password"] | "user_password";
  mqtt.enable_certificates = doc["mqtt"]["enable_certificates"] | false;
  mqtt.ca_file = doc["mqtt"]["ca_file"] | "certs/ca.crt";
  mqtt.cert_file = doc["mqtt"]["cert_file"] | "certs/client.crt";
  mqtt.key_file = doc["mqtt"]["key_file"] | "certs/client.key";
  mqtt.ca_file = doc["mqtt"]["ca_file"] | "server_address";
  for (unsigned int i = 0; i < doc["mqtt"]["pub_topic"].size(); i++) { //Iterate through results
    // mqtt.pub_topic[i] = doc["mqtt"]["pub_topic"][i];  //Implicit cast
    mqtt.pub_topic[i] = doc["mqtt"]["pub_topic"][i].as<String>(); //Explicit cast
  }
  for (unsigned int i = 0; i < doc["mqtt"]["sub_topic"].size(); i++)
    mqtt.sub_topic[i] = doc["mqtt"]["sub_topic"][i].as<String>();


  // Services object:
  // OTA
  services.ota = doc["services"]["OTA"] | false;
  // FTP
  services.ftp.enabled = doc["services"]["FTP"]["enabled"] | false;
  services.ftp.user = doc["services"]["FTP"]["user"] | "admin";
  services.ftp.password = doc["services"]["FTP"]["password"] | "admin";
  // WebSockets
  services.webSockets.enabled = doc["services"]["WebSockets"]["enabled"] | false;
  services.webSockets.publish_time_ms = doc["services"]["WebSockets"]["publish_time_ms"];
  services.webSockets.port = doc["services"]["WebSockets"]["port"];
  // DeepSleep
  services.deep_sleep.enabled = doc["services"]["deep_sleep"]["enabled"] | false;
  services.deep_sleep.mode = doc["services"]["deep_sleep"]["mode"] | "WAKE_RF_DEFAULT";
  for (unsigned int i = 0; i < doc["services"]["deep_sleep"]["mode_options"].size(); i++) { //Iterate through results
    services.deep_sleep.mode_options[i] = doc["services"]["deep_sleep"]["mode_options"][i].as<String>(); //Explicit cast
  }
  services.deep_sleep.sleep_time = doc["services"]["deep_sleep"]["sleep_time"];
  services.deep_sleep.sleep_delay = doc["services"]["deep_sleep"]["sleep_delay"];
  // LightSleep
  services.light_sleep.enabled = doc["services"]["light_sleep"]["enabled"] | false;
  services.light_sleep.mode = doc["services"]["light_sleep"]["mode"] | "LIGHT_SLEEP_T";
  for (unsigned int i = 0; i < doc["services"]["light_sleep"]["mode_options"].size(); i++) { //Iterate through results
    services.light_sleep.mode_options[i] = doc["services"]["light_sleep"]["mode_options"][i].as<String>(); //Explicit cast
  }
  // NTP
  services.ntp.enabled = doc["services"]["ntp"]["enabled"] | false;
  services.ntp.ntpServer = doc["services"]["ntp"]["ntpServer"] | "2pool.ntp.org";
  services.ntp.gmtOffset_sec = doc["services"]["ntp"]["gmt_offset_sec"];
  services.ntp.daylightOffset_sec = doc["services"]["ntp"]["daylight_offset_sec"];


  // Device object:
  device.track_restart_counter = doc["device"]["track_restart_counter"] | true;
  device.loop_time_ms = doc["device"]["loop_time_ms"];
  device.publish_time_ms = doc["device"]["publish_time_ms"];


  // Info object:
  info.restart_counter = doc["info"]["ftp"]["enabled"] | false;
  info.fw_version = doc["info"]["fw_version"] | "-";
  info.repo = doc["info"]["repo"] | "github.com/paclema";


}


void WebConfigServer::addConfig(IWebConfig* config, String nameObject){
  config->nameConfigObject = nameObject;
  this->configs.add(config);
  Serial.print("IWebConfig Object added for: ");
  Serial.println(config->nameConfigObject);
};


void WebConfigServer::parseIWebConfig(const JsonDocument& doc){
  // Serial.print("List IWebConfig Objects size: ");
  // Serial.println(configs.size());

  IWebConfig *config ;
  for(int i = 0; i < configs.size(); i++){
    config = configs.get(i);
    config->parseWebConfig(doc[config->nameConfigObject]);
    // Serial.print("IWebConfig Object parsed for: ");
    // Serial.println(config->nameConfigObject);

  }
};


// Loads the configuration from a file
void WebConfigServer::loadConfigurationFile(const char *filename){
  // Open file for reading
  File file = SPIFFS.open(filename, "r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  DynamicJsonDocument doc(CONFIG_JSON_SIZE);

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Parse file to Config struct object:
  WebConfigServer::parseConfig(doc);

  // Parse file to IWebConfig objects:
  WebConfigServer::parseIWebConfig(doc);

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}


// Saves the Config struct configuration to a file
void WebConfigServer::saveConfigurationFile(const char *filename){
  // Delete existing file, otherwise the configuration is appended to the file
  SPIFFS.remove(filename);

  // Open file for writing
  // File file = SD.open(filename, FILE_WRITE);
  File file = SPIFFS.open(filename, "w");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  DynamicJsonDocument doc(CONFIG_JSON_SIZE);

  // Network object:
  doc["network"]["ssid_name"] = network.ssid_name;
  doc["network"]["ssid_password"] = network.ssid_password;

  // Network object:
  doc["mqtt"]["server"] = mqtt.server;
  doc["mqtt"]["port"] = mqtt.port;

  // Services object:
  doc["services"]["FTP"] = mqtt.server;
  doc["services"]["port"] = mqtt.port;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();
}


// Prints the content of a file to the Serial
void WebConfigServer::printFile(String filename){
  // Open file for reading
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}


// Restore the backup of a file:
bool WebConfigServer::restoreBackupFile(String filenamechar){

      String filename = filenamechar;
      if(!filename.startsWith("/")) filename = "/"+filename;
      String filename_bak =  "/.bak"+filename;
      Serial.print("Restoring backup for: "); Serial.println(filename);
      // Delete existing file, otherwise the configuration is appended to the file
      SPIFFS.remove(filename);

      File file = SPIFFS.open(filename, "w+");
      if (!file) {
        Serial.print(F("Failed to read file: "));Serial.println(filename);
        return false;
      }

      //Serial.print("Opened: "); Serial.println(filename);
      File file_bak = SPIFFS.open(filename_bak, "r");
      if (!file) {
        Serial.print(F("Failed to read backup file: "));Serial.println(filename_bak);
        return false;
      }

      //Serial.print("Opened: "); Serial.println(filename_bak);

      size_t n;
      uint8_t buf[64];
      while ((n = file_bak.read(buf, sizeof(buf))) > 0) {
        file.write(buf, n);
      }
      Serial.println("Backup restored");
      file_bak.close();
      file.close();
      return true;
}


#ifdef USE_ASYNC_WEBSERVER
void WebConfigServer::updateGpio(AsyncWebServerRequest *request){

  //List all parameters (Compatibility)
  // int args = request->args();
  // for(int i=0;i<args;i++){
  //   Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
  // }

  String gpio, val;

  if(request->hasArg("id")) gpio = request->arg("id");
  if(request->hasArg("val")) val = request->arg("val");

  String success = "1";

  #ifdef ESP32
  int pin = GPIO_ID_PIN(5);
  if ( gpio == "D5" ) {
    pin = GPIO_ID_PIN(5);
  } else if ( gpio == "D7" ) {
     pin = GPIO_ID_PIN(7);
  } else if ( gpio == "D8" ) {
     pin = GPIO_ID_PIN(8);
  } else if ( gpio == "LED_BUILTIN" ) {
     pin = GPIO_ID_PIN(LED_BUILTIN);
  } else {
     // Built-in nodemcu GPI16, pin 16 led D0
     // esp12 led is 2 or D4
     pin = LED_BUILTIN;
  }
  #elif defined(ESP8266)
    int pin = D5;
    if ( gpio == "D5" ) {
      pin = D5;
    } else if ( gpio == "D7" ) {
       pin = D7;
    } else if ( gpio == "D8" ) {
       pin = D8;
    } else if ( gpio == "LED_BUILTIN" ) {
       pin = LED_BUILTIN;
    } else {
       // Built-in nodemcu GPI16, pin 16 led D0
       // esp12 led is 2 or D4
       pin = LED_BUILTIN;
    }
  #endif


    // Reverse current LED status:
    pinMode(pin, OUTPUT);
    // digitalWrite(pin, LOW);
    digitalWrite(pin, !digitalRead(pin));

    // if ( val == "true" ) {
    //   digitalWrite(pin, HIGH);
    // } else if ( val == "false" ) {
    //   digitalWrite(pin, LOW);
    // } else {
    //   success = "true";
    //   Serial.println("Err parsing GPIO Value");
    // }

    // String response = "{\"gpio\":\"" + String(gpio) + "\",";
    // response += "\"val\":\"" + String(val) + "\",";
    // response += "\"success\":\"" + String(success) + "\"}";

    String response = "{\"message\":\"GPIO " + String(gpio);
    response += " changed to " + String(!digitalRead(pin)) + "\"}";

    request->send(200, "text/json", response);
    Serial.println("JSON POST /gpio : " + response);

}

void WebConfigServer::configureServer(AsyncWebServer *server){

  //list directory
  // server->serveStatic("/certs", SPIFFS, "/certs");
  // server->serveStatic("/img", SPIFFS, "/img");
  // server->serveStatic("/", SPIFFS, "/index.html");

  server->serveStatic("/config/config.json", SPIFFS, "/config/config.json");
  server->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setCacheControl("max-age=600");
  // server->serveStatic("/", SPIFFS, "/");
  server->serveStatic("/index.html", SPIFFS, "/index.html");
  server->serveStatic("/main.js", SPIFFS, "/main.js");
  server->serveStatic("/polyfills.js", SPIFFS, "/polyfills.js");
  server->serveStatic("/runtime.js", SPIFFS, "/runtime.js");
  server->serveStatic("/styles.css", SPIFFS, "/styles.css");
  server->serveStatic("/scripts.js", SPIFFS, "/scripts.js");
  server->serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
  server->serveStatic("/chiplogo.png", SPIFFS, "/chiplogo.png");
  // server->serveStatic("/3rdpartylicenses.txt", SPIFFS, "/3rdpartylicenses.txt");



  server->on("/gpio", HTTP_POST, [& ,this](AsyncWebServerRequest *request){
    updateGpio(request);
    Serial.print("getHeapFree(): "); Serial.print(ESP.getFreeHeap());
    Serial.println();

  });



  AsyncCallbackJsonWebHandler* handlerSaveConfig = new AsyncCallbackJsonWebHandler(
      "/save_config",
      [& ,server](AsyncWebServerRequest *request, JsonVariant &json) {
    DynamicJsonDocument doc(CONFIG_JSON_SIZE);
    doc = json;
    String response;
    if ( !doc.isNull()){
      size_t freeBytes =  ESP.getFreeHeap() - (CONFIG_JSON_SIZE);
      Serial.println("Free Heap bytes: " + String(freeBytes));
      if ( freeBytes >= MIN_HEAP_SIZE_FOR_SAVING_CONFIG){
        // Serial.print("\nJSON received: ");
        // serializeJsonPretty(doc, Serial);
        // Serial.println("");

        // Parse file to Config struct object:
        WebConfigServer::parseConfig(doc);
        // Parse file to IWebConfig objects:
        WebConfigServer::parseIWebConfig(doc);
        // Save the config file with new configuration:

        if (WebConfigServer::saveWebConfigurationFile(CONFIG_FILE,doc)){
          response = "{\"message\": \"Configurations saved\"}";
          request->send(200, "text/json", response);
          Serial.println("JSON POST /save_config: " + response);
        } else {
          response = "{\"message\": \"Error saving the configuration\"}";
          request->send(503, "text/json", response);
          Serial.println("JSON POST /save_config: " + response);
        }
      } else {
      response = "{\"message\": \"Error: Not enought memory for saving configurations. Free Heap bytes: " +String(freeBytes)+ "\"}";
      request->send(507, "text/json", response);
      Serial.println("JSON POST /save_config: " + response);
      }

    } else {
      response = "{\"message\": \"Error: JSON received is Null or chunked\"}";
      request->send(400, "text/json", response);
      Serial.println("JSON POST /save_config: " + response);
    }
  });
  server->addHandler(handlerSaveConfig);


  server->on("/restore_config", HTTP_POST, [& ,server](AsyncWebServerRequest *request){

    // if (request->args() > 0){
    //   for (int i = 0; i < request->args(); i++ ) {
    //     // Serial.print("POST Arguments: " ); Serial.println(server->args(i));
    //     Serial.print("Name: "); Serial.println(request->argName(i));
    //     Serial.print("Value: "); Serial.println(request->arg(i));
    //   }
    // }

    String response;
    if( ! request->hasArg("filename") || request->arg("filename") == NULL){
      response = "{\"message\": \"filename to restore not provided\"}";
      request->send(400, "text/json", response);
      Serial.println("JSON POST /restore_config: " + response);
    } else {
      Serial.print("File to restore: "); Serial.println(request->arg("filename"));
      if (restoreBackupFile(request->arg("filename"))){
        response = "{\"message\": \"Configuration restored\"}";
        request->send(200, "text/json", response);
        Serial.println("JSON POST /restore_config: " + response);
      }
      else{
        response = "{\"message\": \"Error while reading files\"}";
        request->send(400, "text/json", response);
        Serial.println("JSON POST /restore_config: " + response);
      }
    }
  });


  server->on("/restart", HTTP_POST, [& ,this](AsyncWebServerRequest *request){
    if( ! request->hasArg("restart") || request->arg("restart") == NULL){
      String response = "{\"message\": \"400: Invalid Request\"}";
      request->send(400, "text/json", response);
      Serial.println("JSON POST /restart: " + response);
    } else{
      if (request->arg("restart") == "true"){
        String response = "{\"message\": \"Restarting device...\"}";
        request->send(200, "text/json", response);
        Serial.println("JSON POST /restart: " + response);

        // This hould not be here, we should track the reboot autside of the
        // handler as here: https://github.com/me-no-dev/ESPAsyncWebServer#setting-up-the-server
        // If not, the 200 response won't be sent properly.
        // delay(150);    // Delays will not take place under Asyc WebServer
        WiFi.disconnect();
        SPIFFS.end();
        ESP.restart();
      }
    }
  });


  // Handle files also gzipped if requested other file not configured using serveStatic():
  server->onNotFound([&, this](AsyncWebServerRequest *request) {
    // If the client requests any URI
    // String path = server->uri();
    if (!handleFileRead(request, request->url()))
      // send it if it exists
      // otherwise, respond with a 404 (Not Found) error:
      request->send(404, "text/plain", "404: Not Found");
      request->client()->close();
  });

  server->begin();
  Serial.println ( "HTTP server started" );
}


bool WebConfigServer::handleFileRead(AsyncWebServerRequest *request, String path) {
  // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  // If a folder is requested, send the index file
  if(path.endsWith("/")) path += "index.html";
  // Get the MIME type
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  // If the file exists, either as a compressed archive, or normal
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    // If there's a compressed version available use it
    bool gzipContent = false;
    if(SPIFFS.exists(pathWithGz)){
      path += ".gz";
      gzipContent = true;
    }
    // Open the file and send it to the client
    File file = SPIFFS.open(path, "r");
    // size_t sent_size = server->streamFile(file, contentType);
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType);
    if(gzipContent) response->addHeader("Content-Encoding", "gzip");
    request->send(response);
    file.close();
    // Serial.println(String("\tSent file: ") + path + String(" - ") + formatBytes(sent_size));
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;
}

#else
#ifdef ESP32
void WebConfigServer::updateGpio(WebServer *server){
  String gpio = server->arg("id");
  String val = server->arg("val");
  String success = "1";

  int pin = GPIO_ID_PIN(5);
  if ( gpio == "D5" ) {
    pin = GPIO_ID_PIN(5);
  } else if ( gpio == "D7" ) {
     pin = GPIO_ID_PIN(7);
   } else if ( gpio == "D8" ) {
     pin = GPIO_ID_PIN(8);
   } else if ( gpio == "LED_BUILTIN" ) {
     pin = LED_BUILTIN;
   } else {
     // Built-in nodemcu GPI16, pin 16 led D0
     // esp12 led is 2 or D4
     pin = LED_BUILTIN;

    }

  // Reverse current LED status:
  pinMode(pin, OUTPUT);
  // digitalWrite(pin, LOW);
  Serial.println(pin);
  Serial.print("Current status:");
  Serial.println(digitalRead(pin));
  digitalWrite(pin, !digitalRead(pin));


  // if ( val == "true" ) {
  //   digitalWrite(pin, HIGH);
  // } else if ( val == "false" ) {
  //   digitalWrite(pin, LOW);
  // } else {
  //   success = "true";
  //   Serial.println("Err parsing GPIO Value");
  // }

  // String json = "{\"gpio\":\"" + String(gpio) + "\",";
  // json += "\"val\":\"" + String(val) + "\",";
  // json += "\"success\":\"" + String(success) + "\"}";

  String json = "{\"message\":\"GPIO " + String(gpio);
  json += " changed to " + String(!digitalRead(pin)) + "\"}";

  server->send(200, "application/json", json);
  Serial.println("GPIO updated!");

}


void WebConfigServer::configureServer(WebServer *server){

  // Create configuration file
  //saveConfigurationFile(CONFIG_FILE);

  //printFile(CONFIG_FILE);

  //SERVER INIT
  //list directory
  // server->serveStatic("/config/config.json", SPIFFS, "/config/config.json");
  // server->serveStatic("/certs", SPIFFS, "/certs.gz");
  // server->serveStatic("/img", SPIFFS, "/img.gz");
  // server->serveStatic("/", SPIFFS, "/index.html.gz");
  // server->serveStatic("/main.js", SPIFFS, "/main.js.gz");
  // server->serveStatic("/polyfills.js", SPIFFS, "/polyfills.js.gz");
  // server->serveStatic("/runtime.js", SPIFFS, "/runtime.js.gz");
  // server->serveStatic("/styles.css", SPIFFS, "/styles.css.gz");
  // server->serveStatic("/scripts.js", SPIFFS, "/scripts.js.gz");
  // server->serveStatic("/3rdpartylicenses.txt", SPIFFS, "/3rdpartylicenses.txt.gz");
  // server->serveStatic("/favicon.ico", SPIFFS, "/favicon.ico.gz");
  // server->serveStatic("/", SPIFFS, "/");


  server->on("/gpio", HTTP_POST, [& ,server](){
    updateGpio(server);
  });

  server->on("/save_config", HTTP_POST, [& ,server](){

    DynamicJsonDocument doc(CONFIG_JSON_SIZE);
    deserializeJson(doc, server->arg("plain"));

    // JsonObject network = doc["network"];

    // Serial.print("JSON POST: ");
    // serializeJsonPretty(doc, Serial);
    // Serial.println("");

    // Parse file to Config struct object:
    WebConfigServer::parseConfig(doc);

    // Parse file to IWebConfig objects:
    WebConfigServer::parseIWebConfig(doc);

    // Save the config file with new configuration:
    WebConfigServer::saveWebConfigurationFile(CONFIG_FILE,doc);

    String response = "{\"message\": \"Configurations saved\"}";
    server->send ( 200, "text/json", response );
    Serial.println("JSON POST: " + response);

  });


  server->on("/restore_config", HTTP_POST, [& ,server](){
    String response;
    if( ! server->hasArg("filename") || server->arg("filename") == NULL){
      response = "{\"message\": \"filename to restore not provided\"}";
      server->send(400, "text/json", response);
      Serial.println("JSON POST /restore_config: " + response);
    } else {
      Serial.print("File to restore: "); Serial.println(server->arg("filename"));
      if (restoreBackupFile(server->arg("filename"))){
        response = "{\"message\": \"Configuration restored\"}";
        server->send(200, "text/json", response);
        Serial.println("JSON POST /restore_config: " + response);
      }
      else{
        response = "{\"message\": \"Error while reading files\"}";
        server->send(400, "text/json", response);
        Serial.println("JSON POST /restore_config: " + response);
      }
    }
  });


  server->on("/restart", HTTP_POST, [& ,server](){
    if( ! server->hasArg("restart") || server->arg("restart") == NULL){
      server->send(400, "text/plain", "400: Invalid Request");
    } else{
      Serial.print("File to restore: "); Serial.println(server->arg("restart"));
      if (server->arg("restart") == "true"){
        server->send ( 200, "text/json", "{\"message\": \"Restarting device...\"}" );

        delay(150);
        server->close();
        server->stop();
        WiFi.disconnect();
        SPIFFS.end();
        ESP.restart();
      }
      server->send ( 200, "text/json", "{\"message\": \"Device restarted\"}" );
    }
  });

  // Handle files also gziped:
  server->onNotFound([& ,server]() {
    // If the client requests any URI
    String path = server->uri();
    if (!handleFileRead(server, server->uri()))
      // send it if it exists
      // otherwise, respond with a 404 (Not Found) error:
      server->send(404, "text/plain", "404: Not Found");
  });

  server->begin();
  Serial.println ( "HTTP server started" );

}


bool WebConfigServer::handleFileRead(WebServer *server, String path) {
  // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  // If a folder is requested, send the index file
  if(path.endsWith("/")) path += "index.html";
  // Get the MIME type
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  // If the file exists, either as a compressed archive, or normal
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    // If there's a compressed version available use it
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    // Open the file and send it to the client
    File file = SPIFFS.open(path, "r");
    size_t sent_size = server->streamFile(file, contentType);
    file.close();
    Serial.println(String("\tSent file: ") + path + String(" - ") + formatBytes(sent_size));
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;
}

#elif defined(ESP8266)

void WebConfigServer::updateGpio(ESP8266WebServer *server){
  String gpio = server->arg("id");
  String val = server->arg("val");
  String success = "1";

  int pin = D5;
  if ( gpio == "D5" ) {
    pin = D5;
  } else if ( gpio == "D7" ) {
     pin = D7;
   } else if ( gpio == "D8" ) {
     pin = D8;
   } else if ( gpio == "LED_BUILTIN" ) {
     pin = LED_BUILTIN;
   } else {
     // Built-in nodemcu GPI16, pin 16 led D0
     // esp12 led is 2 or D4
     pin = LED_BUILTIN;

    }

  // Reverse current LED status:
  pinMode(pin, OUTPUT);
  // digitalWrite(pin, LOW);
  Serial.println(pin);
  Serial.print("Current status:");
  Serial.println(digitalRead(pin));
  digitalWrite(pin, !digitalRead(pin));


  // if ( val == "true" ) {
  //   digitalWrite(pin, HIGH);
  // } else if ( val == "false" ) {
  //   digitalWrite(pin, LOW);
  // } else {
  //   success = "true";
  //   Serial.println("Err parsing GPIO Value");
  // }

  // String json = "{\"gpio\":\"" + String(gpio) + "\",";
  // json += "\"val\":\"" + String(val) + "\",";
  // json += "\"success\":\"" + String(success) + "\"}";

  String json = "{\"message\":\"GPIO " + String(gpio);
  json += " changed to " + String(!digitalRead(pin)) + "\"}";

  server->send(200, "application/json", json);
  Serial.println("GPIO updated!");

}

void WebConfigServer::configureServer(ESP8266WebServer *server){

  // Create configuration file
  //saveConfigurationFile(CONFIG_FILE);

  //printFile(CONFIG_FILE);

  //SERVER INIT
  //list directory
  // server->serveStatic("/config/config.json", SPIFFS, "/config/config.json");
  // server->serveStatic("/certs", SPIFFS, "/certs.gz");
  // server->serveStatic("/img", SPIFFS, "/img.gz");
  // server->serveStatic("/", SPIFFS, "/index.html.gz");
  // server->serveStatic("/main.js", SPIFFS, "/main.js.gz");
  // server->serveStatic("/polyfills.js", SPIFFS, "/polyfills.js.gz");
  // server->serveStatic("/runtime.js", SPIFFS, "/runtime.js.gz");
  // server->serveStatic("/styles.css", SPIFFS, "/styles.css.gz");
  // server->serveStatic("/scripts.js", SPIFFS, "/scripts.js.gz");
  // server->serveStatic("/3rdpartylicenses.txt", SPIFFS, "/3rdpartylicenses.txt.gz");
  // server->serveStatic("/favicon.ico", SPIFFS, "/favicon.ico.gz");
  // server->serveStatic("/", SPIFFS, "/");


  server->on("/gpio", HTTP_POST, [& ,server](){
    updateGpio(server);
  });

  server->on("/save_config", HTTP_POST, [& ,server](){

    DynamicJsonDocument doc(CONFIG_JSON_SIZE);
    deserializeJson(doc, server->arg("plain"));

    // JsonObject network = doc["network"];

    // Serial.print("JSON POST: ");
    // serializeJsonPretty(doc, Serial);
    // Serial.println("");

    // Parse file to Config struct object:
    WebConfigServer::parseConfig(doc);

    // Parse file to IWebConfig objects:
    WebConfigServer::parseIWebConfig(doc);

    // Save the config file with new configuration:
    WebConfigServer::saveWebConfigurationFile(CONFIG_FILE,doc);

    String response = "{\"message\": \"Configurations saved\"}";
    server->send ( 200, "text/json", response );
    Serial.println("JSON POST: " + response);

  });


  server->on("/restore_config", HTTP_POST, [& ,server](){
    String response;
    if( ! server->hasArg("filename") || server->arg("filename") == NULL){
      response = "{\"message\": \"filename to restore not provided\"}";
      server->send(400, "text/json", response);
      Serial.println("JSON POST /restore_config: " + response);
    } else {
      Serial.print("File to restore: "); Serial.println(server->arg("filename"));
      if (restoreBackupFile(server->arg("filename"))){
        response = "{\"message\": \"Configuration restored\"}";
        server->send(200, "text/json", response);
        Serial.println("JSON POST /restore_config: " + response);
      }
      else{
        response = "{\"message\": \"Error while reading files\"}";
        server->send(400, "text/json", response);
        Serial.println("JSON POST /restore_config: " + response);
      }
    }
  });


  server->on("/restart", HTTP_POST, [& ,server](){
    if( ! server->hasArg("restart") || server->arg("restart") == NULL){
      server->send(400, "text/plain", "400: Invalid Request");
    } else{
      Serial.print("File to restore: "); Serial.println(server->arg("restart"));
      if (server->arg("restart") == "true"){
        server->send ( 200, "text/json", "{\"message\": \"Restarting device...\"}" );

        delay(150);
        server->close();
        server->stop();
        WiFi.disconnect();
        SPIFFS.end();
        ESP.restart();
      }
      server->send ( 200, "text/json", "{\"message\": \"Device restarted\"}" );
    }

  });

  // Handle files also gziped:
  server->onNotFound([& ,server]() {
    // If the client requests any URI
    String path = server->uri();
    if (!handleFileRead(server, server->uri()))
      // send it if it exists
      // otherwise, respond with a 404 (Not Found) error:
      server->send(404, "text/plain", "404: Not Found");
  });

  server->begin();
  Serial.println ( "HTTP server started" );

}

bool WebConfigServer::handleFileRead(ESP8266WebServer *server, String path) {
  // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  // If a folder is requested, send the index file
  if(path.endsWith("/")) path += "index.html";
  // Get the MIME type
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  // If the file exists, either as a compressed archive, or normal
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    // If there's a compressed version available use it
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    // Open the file and send it to the client
    File file = SPIFFS.open(path, "r");
    size_t sent_size = server->streamFile(file, contentType);
    file.close();
    Serial.println(String("\tSent file: ") + path + String(" - ") + formatBytes(sent_size));
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;
}

#endif
#endif

String WebConfigServer::getContentType(String filename) {
  // convert the file extension to the MIME type
  if(filename.endsWith(F(".html"))) 	return F("text/html");
  else if(filename.endsWith(F(".gz"))) 		return F("application/x-gzip");
  else if(filename.endsWith(F(".css"))) 	return F("text/css");
  else if(filename.endsWith(F(".js"))) 		return F("application/javascript");
  else if(filename.endsWith(F(".json"))) 	return F("application/json");
  else if(filename.endsWith(F(".ico"))) 	return F("image/x-icon");
  else if(filename.endsWith(F(".png"))) 	return F("image/png");
  else if(filename.endsWith(F(".htm"))) 	return F("text/html");
  else if(filename.endsWith(F(".gif"))) 	return F("image/gif");
  else if(filename.endsWith(F(".jpg"))) 	return F("image/jpeg");
  else if(filename.endsWith(F(".jpeg"))) 	return F("image/jpeg");
  else if(filename.endsWith(F(".xml"))) 	return F("text/xml");
  else if(filename.endsWith(F(".pdf"))) 	return F("application/x-pdf");
  else if(filename.endsWith(F(".zip"))) 	return F("application/x-zip");
  return "text/plain";
}

void WebConfigServer::handle(void){

}
