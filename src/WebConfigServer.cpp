#include "WebConfigServer.h"


WebConfigServer::WebConfigServer(void): 
  ftpSrv(SPIFFS) {
  config_status = CONFIG_NOT_LOADED;

  #ifdef USE_ASYNC_WEBSERVER
    server = new AsyncWebServer(80);
  #elif defined(ESP32)
    server = new WebServer(80);
  #elif defined(ESP8266)
    server = new ESP8266WebServer(80);
  #endif

  // Setup internal configs:
  WebConfigServer::addConfig(mqtt, "mqtt");
  WebConfigServer::addConfigService(services.ota, "OTA");
  WebConfigServer::addConfigService(services.webSockets, "WebSockets");
}


bool WebConfigServer::initWebConfigs(void){

  #ifdef ESP32
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount failed");
    config_status = CONFIG_NOT_LOADED;
    return false;
  } else {
    Serial.println("SPIFFS Mount succesfull");
    WebConfigServer::updateSizeSPIFFS(true);
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
      config_status = CONFIG_LOADED;
      return true;
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
      WebConfigServer::updateSizeSPIFFS(true);
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
        config_status = CONFIG_LOADED;
        return true;
      } else {
        config_status = CONFIG_NOT_LOADED;
        return false;
      }
    }
  #endif

}


bool WebConfigServer::begin(void){

  if (config_status != CONFIG_LOADED) initWebConfigs();

  // Configure NTP if enabled before wifi restart:
  if (services.ntp.enabled){
    #ifdef ESP8266
      // settimeofday_cb(time_is_set);
      // settimeofday_cb(std::bind(&WebConfigServer::time_is_set, this));
      settimeofday_cb([&]() {
        WebConfigServer::time_is_set();
      });
    #endif
    configTime(services.ntp.gmtOffset_sec, services.ntp.daylightOffset_sec, services.ntp.ntpServer.c_str());
  }


  // Restart the newtwork:
  WebConfigServer::networkRestart();

  // Configure and start the server:
  WebConfigServer::configureServer();

  // TODO: move this to reconnect future method:
  WebConfigServer::enableServices();

  // Setup MQTT:
  if (mqtt.isEnabled()) {
    mqtt.setup();
    if (mqtt.getReconnect()) mqtt.reconnect();
  }

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
  Serial.print(F("Saving WebConfig file... "));
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
  Serial.println(F("Saved!"));
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
  network.connection_retries = doc["network"]["connection_retries"] | 0;
  network.ip_address = doc["network"]["ip_address"] | "192.168.1.2";
  network.subnet = doc["network"]["subnet"] | "255.255.255.0";
  network.dns_server = doc["network"]["dns_server"] | "192.168.1.1";
  network.hostname = doc["network"]["hostname"] | "iotdevice.local";
  network.enable_NAT = doc["network"]["enable_NAT"] | false;


  // MQTT object:
  mqtt.setPublishTime(doc["device"]["publish_time_ms"]);


  // Services object:
  // FTP
  services.ftp.enabled = doc["services"]["FTP"]["enabled"] | false;
  services.ftp.user = doc["services"]["FTP"]["user"] | "admin";
  services.ftp.password = doc["services"]["FTP"]["password"] | "admin";
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


void WebConfigServer::addConfig(IWebConfig& config, String nameObject){
  config.nameConfigObject = nameObject;
  configs.add(&config);
  Serial.print("IWebConfig Object added for: ");
  Serial.println(config.nameConfigObject);
};


void WebConfigServer::addConfigService(IWebConfig& config, String nameObject){
  config.nameConfigObject = nameObject;
  configsServices.add(&config);
  Serial.print("IWebConfig Object added for: ");
  Serial.println(config.nameConfigObject);
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


void WebConfigServer::parseIWebConfigService(const JsonDocument& doc){
  // Serial.print("List IWebConfig Objects size: ");
  // Serial.println(configsServices.size());

  IWebConfig *config ;
  for(int i = 0; i < configsServices.size(); i++){
    config = configsServices.get(i);
    config->parseWebConfig(doc["services"][config->nameConfigObject]);
    // Serial.print("IWebConfig Object parsed for: ");
    // Serial.println(config->nameConfigObject);

  }
};


void WebConfigServer::enableServices(void){
  Serial.println("\n--- Services: ");

  if (services.ota.isEnabled()){
    services.ota.init();
    Serial.println("   - OTA -> enabled");
  } else Serial.println("   - OTA -> disabled");


  if (services.ftp.enabled && services.ftp.user !=NULL && services.ftp.password !=NULL){
    // ftpSrv.begin(services.ftp.user,services.ftp.password);
    ftpSrv.begin(services.ftp.user.c_str(),services.ftp.password.c_str());
    Serial.println("   - FTP -> enabled");
  } else Serial.println("   - FTP -> disabled");

  if (services.webSockets.isEnabled()){
    services.webSockets.init();
    Serial.println("   - WebSockets -> enabled");
  } else Serial.println("   - WebSockets -> disabled");

  if (services.deep_sleep.enabled){
    // We will enable it on the loop function
    // Serial.println("   - Deep sleep -> configured");
    Serial.print("   - Deep sleep -> enabled for ");
    Serial.print(services.deep_sleep.sleep_time);
    Serial.print("s after waitting ");
    Serial.print(services.deep_sleep.sleep_delay);
    Serial.println("s. Choose sleep_time: 0 for infinite sleeping");
    Serial.println("     Do not forget to connect D0 to RST pin to auto-wake up! Or I will sleep forever");
  } else Serial.println("   - Deep sleep -> disabled");


  #ifdef ESP32
    // TODO: enable sleep modes for ESP32 here

  #elif defined(ESP8266)
    if (services.light_sleep.enabled){
      if (services.light_sleep.mode == "LIGHT_SLEEP_T")
        wifi_set_sleep_type(LIGHT_SLEEP_T);
      else if (services.light_sleep.mode == "NONE_SLEEP_T")
        wifi_set_sleep_type(NONE_SLEEP_T);
      else {
        Serial.println("   - Light sleep -> mode not available");
        return;
      }
      Serial.println("   - Light sleep -> enabled");
    } else Serial.println("   - Light sleep -> disabled");
  #endif

  if (services.ntp.enabled && WiFi.status() == WL_CONNECTED){
    configTime(services.ntp.gmtOffset_sec, services.ntp.daylightOffset_sec, services.ntp.ntpServer.c_str());

    #ifdef ESP8266
      // settimeofday_cb(time_is_set);
      settimeofday_cb([&]() {
        WebConfigServer::time_is_set();
      });
    #elif defined(ESP32)
      Serial.print ("\t\tSynching Time over NTP ");
      while((time(nullptr)) < NTP_MIN_VALID_EPOCH) {
        // warning : no time out. May loop here forever
        delay(20);
        Serial.print(".");
      }
      cbtime_set = true;
      Serial.println("");
    #endif
    
    Serial.print("   - NTP -> enabled\n");
    Serial.printf("          - Server: %s\n", services.ntp.ntpServer.c_str());
    Serial.printf("          - GMT offset: %d\n", services.ntp.gmtOffset_sec);
    Serial.printf("          - Day light offset: %d\n", services.ntp.daylightOffset_sec);
  } else Serial.println("   - NTP -> disabled");



  Serial.println("");

}


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

  // Parse file to WebConfig IWebConfig service objects:
  WebConfigServer::parseIWebConfigService(doc);

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

  // MQTT object:
  // doc["mqtt"]["server"] = mqtt.server;
  // doc["mqtt"]["port"] = mqtt.port;

  // Services object:
  // doc["services"]["FTP"] = mqtt.server;
  // doc["services"]["port"] = mqtt.port;

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

void WebConfigServer::configureServer(){

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
      [& ,this](AsyncWebServerRequest *request, JsonVariant &json) {
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
        // Parse file to WebConfig IWebConfig service objects:
        WebConfigServer::parseIWebConfigService(doc);
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


  server->on("/restore_config", HTTP_POST, [& ,this](AsyncWebServerRequest *request){

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


	server->on("/uploadFile", HTTP_POST, [&, this](AsyncWebServerRequest *request) {
		// AsyncWebServerResponse *response = request->beginResponse(200, "text/html", "hello world");

		// AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{ \"status\": true, \"message\": \"File is uploaded\"}");

		// response->addHeader("Connection", "close");
		// request->send(response);.
    
    // List all parameters
    int params = request->params();
    Serial.printf(" \n/uploadFile Request parameters: %d\n", params);
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){ //p->isPost() is also true
        Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }


	}, handleUpload);

  server->onFileUpload(handleUpload);

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

void WebConfigServer::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){

  File file;

  // The file will come chunked if it is to big
	if (!index) {
    // For the first chunk we open the file to override it
		// Serial.printf("UploadStart: %s\n", filename.c_str());
    file = SPIFFS.open(filename, "w");
	} else{
    // The rest of the chunks we apend them into the file
    file = SPIFFS.open(filename, "a");
  }

  size_t writeSize = file.write(data,len);
  // Serial.printf("File index: %u B- size: %u B - wrote: %u B\n", index, len, writeSize);
  // Serial.printf("SPIFFS totalBytes: %u B- usedBytes: %u B\n",  SPIFFS.totalBytes(), SPIFFS.usedBytes());

  if( writeSize != len){
    Serial.printf("Error writing the next file chunk. Index: %u B - size: %u B - wrote: %u B\n", index, len, writeSize);
    AsyncWebServerResponse *response = request->beginResponse(507, "application/json", "{ \"status\": false, \"message\": \"File not fully saved\", \"data\": "\
                                            "{\"name\": \"" + filename + "\", \"mimetype\": \"application/octet-stream\", \"size\": " + (unsigned int)(index + len) + "}}");
		response->addHeader("Connection", "close");
		request->send(response);
  }

  // To print the file using the serial monitor:
	// for (size_t i = 0; i < len; i++) {
	// 	Serial.write(data[i]);
	// }

	if (final) {
		// Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);    
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{ \"status\": true, \"message\": \"File is uploaded\", \"data\": "\
                                               "{\"name\": \"" + filename + "\", \"mimetype\": \"application/octet-stream\", \"size\": " + (unsigned int)(index + len) + "}}");
		response->addHeader("Connection", "close");
		request->send(response);
	}

  file.close();
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


void WebConfigServer::configureServer(){

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


  server->on("/gpio", HTTP_POST, [&, this](){
    updateGpio(server);
  });

  server->on("/save_config", HTTP_POST, [&, this](){

    DynamicJsonDocument doc(CONFIG_JSON_SIZE);
    deserializeJson(doc, server->arg("plain"));

    // JsonObject network = doc["network"];

    // Serial.print("JSON POST: ");
    // serializeJsonPretty(doc, Serial);
    // Serial.println("");

    // Parse file to Config struct object:
    WebConfigServer::parseConfig(doc);

    // Parse file to WebConfig IWebConfig service objects:
    WebConfigServer::parseIWebConfigService(doc);

    // Parse file to IWebConfig objects:
    WebConfigServer::parseIWebConfig(doc);

    // Save the config file with new configuration:
    WebConfigServer::saveWebConfigurationFile(CONFIG_FILE,doc);

    String response = "{\"message\": \"Configurations saved\"}";
    server->send ( 200, "text/json", response );
    Serial.println("JSON POST: " + response);

  });


  server->on("/restore_config", HTTP_POST, [&, this](){
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


  server->on("/restart", HTTP_POST, [&, this](){
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


  server->on("/uploadFile", HTTP_POST, [&, this](){

    // List all parameters
    int args = server->args();
    Serial.printf(" \n/uploadFile Request args: %d\n", args);
    for (uint8_t i = 0; i < args; i++) {
      Serial.print("Param " + server->argName(i) + ": " + server->arg(i) + "\n");
    }

    // List all headers
    int headers = server->headers();
    Serial.printf(" \n/uploadFile Request headers: %d\n", args);
    for (uint8_t i = 0; i < headers; i++) {
      Serial.print("Header " + server->headerName(i) + ": " + server->header(i) + "\n");
    }


  },[&, this](){
    
    HTTPUpload& upload = server->upload();
    File file;
    if(upload.status == UPLOAD_FILE_START){
        Serial.printf("UploadStart: %s\n", upload.filename.c_str());

        // For the first chunk we open the file to override it
        file = SPIFFS.open(upload.filename, "w");
      if(!file){
        Serial.println("- failed to open file for writing");
        return;
      }
    }
    
    if(upload.status == UPLOAD_FILE_WRITE){

      // The rest of the chunks we apend them into the file
      file = SPIFFS.open(upload.filename, "a");
      size_t writeSize = file.write(upload.buf,upload.currentSize);
      if( writeSize != upload.currentSize){
        // Serial.printf("Error writing the next file chunk. Index: %u B - size: %u B - wrote: %u B\n", index, len, writeSize);
        Serial.printf("Error writing the next file chunk. Total size: %u B - currentSize: %u B- wrote: %u B\n", upload.totalSize, upload.currentSize, writeSize);
        server->sendHeader("Connection", "close");
        server->send(507, "application/json", "{ \"status\": false, \"message\": \"File not fully saved\", \"data\": "\
                                                "{\"name\": \"" + upload.filename + "\", \"mimetype\": \"application/octet-stream\", \"size\": " + (unsigned int)upload.totalSize + "}}");
        file.close();
        return;
      } else {
          // Serial.printf("Total size: %u B - currentSize: %u B- wrote: %u B\n", upload.totalSize, upload.currentSize, writeSize);
          // For ESP32:
          // Serial.printf("SPIFFS totalBytes: %u B- usedBytes: %u B\n",  SPIFFS.totalBytes(), SPIFFS.usedBytes());
          // For ESP8266
          // FSInfo fs_info;
          // SPIFFS.info(fs_info);
          // Serial.printf("SPIFFS totalBytes: %u B- usedBytes: %u B\n",  fs_info.totalBytes, fs_info.usedBytes);
      }

    } else if(upload.status == UPLOAD_FILE_END){
        Serial.printf("UploadEnd: %s, %u B\n", upload.filename.c_str(), upload.totalSize);
        server->sendHeader("Connection", "close");
        server->send(200, "application/json", "{ \"status\": true, \"message\": \"File is uploaded\", \"data\": "\
                                                "{\"name\": \"" + upload.filename + "\", \"mimetype\": \"application/octet-stream\", \"size\": " + (unsigned int)upload.totalSize + "}}");
    }

    file.close();

  });

  // Handle files also gziped:
  server->onNotFound([&, this]() {
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

void WebConfigServer::configureServer(){

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


  server->on("/gpio", HTTP_POST, [& ,this](){
    updateGpio(server);
  });

  server->on("/save_config", HTTP_POST, [& ,this](){

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


  server->on("/restore_config", HTTP_POST, [& ,this](){
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


  server->on("/restart", HTTP_POST, [& ,this](){
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


  server->on("/uploadFile", HTTP_POST, [&, this](){

    // List all parameters
    int args = server->args();
    Serial.printf(" \n/uploadFile Request args: %d\n", args);
    for (uint8_t i = 0; i < args; i++) {
      Serial.print("Param " + server->argName(i) + ": " + server->arg(i) + "\n");
    }

    // List all headers
    int headers = server->headers();
    Serial.printf(" \n/uploadFile Request headers: %d\n", args);
    for (uint8_t i = 0; i < headers; i++) {
      Serial.print("Header " + server->headerName(i) + ": " + server->header(i) + "\n");
    }


  },[&, this](){
    
    HTTPUpload& upload = server->upload();
    File file;
    if(upload.status == UPLOAD_FILE_START){
        Serial.printf("UploadStart: %s\n", upload.filename.c_str());

        // For the first chunk we open the file to override it
        file = SPIFFS.open(upload.filename, "w");
      if(!file){
        Serial.println("- failed to open file for writing");
        return;
      }
    }
    
    if(upload.status == UPLOAD_FILE_WRITE){

      // The rest of the chunks we apend them into the file
      file = SPIFFS.open(upload.filename, "a");
      size_t writeSize = file.write(upload.buf,upload.currentSize);
      if( writeSize != upload.currentSize){
        // Serial.printf("Error writing the next file chunk. Index: %u B - size: %u B - wrote: %u B\n", index, len, writeSize);
        Serial.printf("Error writing the next file chunk. Total size: %u B - currentSize: %u B- wrote: %u B\n", upload.totalSize, upload.currentSize, writeSize);
        server->sendHeader("Connection", "close");
        server->send(507, "application/json", "{ \"status\": false, \"message\": \"File not fully saved\", \"data\": "\
                                                "{\"name\": \"" + upload.filename + "\", \"mimetype\": \"application/octet-stream\", \"size\": " + (unsigned int)upload.totalSize + "}}");
        file.close();
        return;
      } else {
          // Serial.printf("Total size: %u B - currentSize: %u B- wrote: %u B\n", upload.totalSize, upload.currentSize, writeSize);
          // FSInfo fs_info;
          // SPIFFS.info(fs_info);
          // Serial.printf("SPIFFS totalBytes: %u B- usedBytes: %u B\n",  fs_info.totalBytes, fs_info.usedBytes);
      }

    } else if(upload.status == UPLOAD_FILE_END){
        Serial.printf("UploadEnd: %s, %u B\n", upload.filename.c_str(), upload.totalSize);
        server->sendHeader("Connection", "close");
        server->send(200, "application/json", "{ \"status\": true, \"message\": \"File is uploaded\", \"data\": "\
                                                "{\"name\": \"" + upload.filename + "\", \"mimetype\": \"application/octet-stream\", \"size\": " + (unsigned int)upload.totalSize + "}}");
    }

    file.close();

  });

  // Handle files also gziped:
  server->onNotFound([& ,this]() {
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

void WebConfigServer::loop(void){

  // Update internal variables:
  currentLoopMillis = millis();
  WebConfigServer::updateSizeSPIFFS(false);

  if (!deviceSetupDone) {
    deviceSetupTime = currentLoopMillis;
    deviceSetupDone = true;
  }
  
  // Handle stations connection to the AP:
  #if ESP32 && IP_NAPT
    if(currentLoopMillis - previousHandleAPMillis > (unsigned)3000) {
      handleAPStations();
      previousHandleAPMillis = currentLoopMillis;
    }
  #endif

  // Handle WebConfigServer not asyc web server:
  #ifndef USE_ASYNC_WEBSERVER
    server->handleClient();
  #endif

  // Handle mqtt reconnection:
  if (mqtt.isEnabled()) {
    if (mqtt.getReconnect() && !mqtt.isConnected()) mqtt.reconnect();
    mqtt.loop();
  }

  // Services loop:
  if (services.ota.isEnabled()) services.ota.handle();
  if (services.ftp.enabled) ftpSrv.handleFTP();
  if (services.webSockets.isEnabled()) services.webSockets.handle();
  if (services.deep_sleep.enabled) deepSleepHandler();


  // NTP time example:
  // time_t now = time(nullptr);
  // timeval tv;
  // gettimeofday(&tv, nullptr);
  // timespec tp;
  // clock_gettime(0, &tp);

  // // time from boot
  // Serial.print("Time from boot: ");
  // Serial.print((uint32_t)tp.tv_sec);
  // Serial.print("s / ");
  // Serial.print((uint32_t)tp.tv_nsec);
  // Serial.println("ns");
  // // EPOCH+tz+dst
  // Serial.print("gtod: ");
  // Serial.print((uint32_t)tv.tv_sec);
  // Serial.print("s / ");
  // Serial.print((uint32_t)tv.tv_usec);
  // Serial.println("us");

  // Serial.print("timestamp:");
  // Serial.print((uint32_t)now);
  // Serial.print(" - ");
  // Serial.println(ctime(&now));

}


void WebConfigServer::networkRestart(void){
  if(config_status == CONFIG_LOADED){

    // WiFi setup:
    // WiFi.disconnect(true);        // close old connections
    #ifdef ESP32
      WiFi.setHostname(network.hostname.c_str());
      WiFi.mode(WIFI_MODE_APSTA);
    #elif defined(ESP8266)
      WiFi.hostname(network.hostname);
      // WiFi.mode(WIFI_STA);
      WiFi.mode(WIFI_AP_STA);
    #endif



    // Client Wifi config:
    if (network.ssid_name!=NULL && network.ssid_password!=NULL){

      // wifiMulti.addAP(network.ssid_name.c_str(),network.ssid_password.c_str());    // Using wifiMulti
      WiFi.begin(network.ssid_name.c_str(),network.ssid_password.c_str());      // Connecting just to one ap

      Serial.printf("Connecting to %s...\n",network.ssid_name.c_str());
      int retries = 0;
      // while ((wifiMulti.run() != WL_CONNECTED)) {   // Using wifiMulti
      while (WiFi.status() != WL_CONNECTED) {    // Connecting just to one ap
        if (network.connection_retries == 0) Serial.print('.');
        else Serial.printf(" %d of %d \n", retries, network.connection_retries);
        delay(500);
        retries++;
        if (network.connection_retries != 0 && (retries > network.connection_retries)) break;
      }
      if ((network.connection_retries != 0 && (retries <= network.connection_retries)) || network.connection_retries == 0) {
        Serial.print("\n\nConnected to ");Serial.print(WiFi.SSID());
        Serial.print("\nIP address:\t");Serial.println(WiFi.localIP());
      } else {Serial.print("\n\nNot Connected to ");Serial.print(network.ssid_name);Serial.println(" max retries reached.");}

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


    if (network.ap_name!=NULL && network.ap_password!=NULL){
        Serial.print("Setting soft-AP... ");
        
        if (network.enable_NAT){
          APEnabled = WiFi.softAP(network.ap_name.c_str(),
            network.ap_password.c_str());
        } else {
          APEnabled = WiFi.softAP(network.ap_name.c_str(),
            network.ap_password.c_str(),
            // If STA connected, use the same channel instead configured one:
            (WiFi.isConnected() && channelSTA) ? channelSTA : network.ap_channel,  
            network.ap_ssid_hidden,
            network.ap_max_connection);
        }
        Serial.println(APEnabled ? "Ready" : "Failed!");
        IPAddress myIP = WiFi.softAPIP();
        Serial.print(network.ap_name);Serial.print(" AP IP address: ");
        Serial.println(myIP);

    }

    // Enable NAPT:
    #if ESP32 && IP_NAPT
      if (network.enable_NAT){
        if (WiFi.isConnected() && APEnabled) {
          esp_err_t err = WebConfigServer::enableNAT();
          if (err == ESP_OK) Serial.println("NAT configured and enabled");
          else Serial.printf("Error configuring NAT: %s\n", esp_err_to_name(err));
        } else Serial.printf("Error configuring NAT: STA or AP not working\n");
      }
    #endif

    // Configure device hostname:
    if (network.hostname){
      if (MDNS.begin(network.hostname.c_str())) Serial.println("mDNS responder started");
      else Serial.println("Error setting up MDNS responder!");
    }

  }
}


#if ESP32 && IP_NAPT
esp_err_t WebConfigServer::enableNAT(void){

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

void WebConfigServer::handleAPStations(void){
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


void WebConfigServer::deepSleepHandler() {


  #ifdef ESP32
    #pragma message ( "WebConfig deepSleep not implemented yet for ESP32" )
    if (currentLoopMillis > deviceSetupTime + (services.deep_sleep.sleep_delay*1000)){

      if (preSleep_routine_configured) (*this->preSleep_routine)();

      Serial.println("   - Deep sleep -> mode not available. Not implemented yet for ESP32");
      delay(100);

    }

  #elif defined(ESP8266)
    if (currentLoopMillis > deviceSetupTime + (services.deep_sleep.sleep_delay*1000)){

      if (preSleep_routine_configured) (*this->preSleep_routine)();
      delay(100);

      if(services.deep_sleep.sleep_time == 0) Serial.printf("Deep sleep activated forever or until RST keeps HIGH...\n");
      else Serial.printf("Deep sleep activated for %f seconds...\n", services.deep_sleep.sleep_time);
      if (services.deep_sleep.mode == "WAKE_RF_DEFAULT")
        // sleep_time is in secs, but the function gets microsecs
        ESP.deepSleep(services.deep_sleep.sleep_time * 1000000, WAKE_RF_DEFAULT);
      else if (services.deep_sleep.mode == "WAKE_RF_DISABLED")
        ESP.deepSleep(services.deep_sleep.sleep_time * 1000000, WAKE_RF_DISABLED);
      else if (services.deep_sleep.mode == "WAKE_RFCAL")
        ESP.deepSleep(services.deep_sleep.sleep_time * 1000000, WAKE_RFCAL);
      else if (services.deep_sleep.mode == "WAKE_NO_RFCAL")
        ESP.deepSleep(services.deep_sleep.sleep_time * 1000000, WAKE_NO_RFCAL);
      else {
        Serial.println("   - Deep sleep -> mode not available");
        delay(100);
        return;
      }
    }
  #endif

}