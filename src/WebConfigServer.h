#ifndef WebConfigServer_H
#define WebConfigServer_H

#include <Arduino.h>

// To enable asyc webserver use platformio build_flags = -D USE_ASYNC_WEBSERVER
// Or define here and in main.cpp:
// #define USE_ASYNC_WEBSERVER

// Allocate a temporary JsonDocument
// Don't forget to change the capacity to match your requirements.
// Use arduinojson.org/assistant to compute the capacity.
#ifndef CONFIG_FILE
  #define CONFIG_FILE "/config/config.json"
#endif

#ifndef CONFIG_JSON_SIZE
  #define CONFIG_JSON_SIZE 4096
#endif
#define DYNAMIC_JSON_DOCUMENT_SIZE CONFIG_JSON_SIZE   //  To redefine "AsyncJson.h" AsyncCallbackJsonWebHandler json doc max size

#ifndef MIN_HEAP_SIZE_FOR_SAVING_CONFIG
  #define MIN_HEAP_SIZE_FOR_SAVING_CONFIG 2000
#endif


#define JSON_MAX_SIZE_LIST 6

#define CONFIG_LOADED "loaded"
#define CONFIG_NOT_LOADED "not_loaded"

typedef int8_t WebConfigStatus;
#define CONFIG_NOT_LOADED            ((WebConfigStatus)   0)
#define CONFIG_LOADED                ((WebConfigStatus)   1)
#define SERVER_INITIALIZING          ((WebConfigStatus)   2)
#define SERVER_RUNNING               ((WebConfigStatus)   3)
#define SERVER_DOWN                  ((WebConfigStatus)   -1)

#define ARDUINOJSON_ENABLE_ALIGNMENT 1


#include <LinkedList.h>
// Using AsyncWebServer LinkedList lib can not be used because there is a class
// using the same name. For that reason, for now we use SimpleList until we fix
// this using namespace for example. Check PR: https://github.com/me-no-dev/ESPAsyncWebServer/pull/1029
// #include <SimpleList.h>
#include "IWebConfig.h"


// MQTT
#ifndef DISABLE_WEBCONFIG_MQTT
#include <PubSubClient.h>
#include "WebConfigMQTT.h"
#endif

// WebConfigOTA
#include "WebConfigOTA.h"

// FTP server
#include <FTPServer.h>

// NTP
#include <time.h>                         // time() ctime()
#include <sys/time.h>                     // struct timeval

#ifdef ESP32
  #define NTP_MIN_VALID_EPOCH 1620413406  // For example: May 7th, 2021
#elif defined(ESP8266)
  #include <coredecls.h>                  // settimeofday_cb()
#endif

extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

// WebScokets
#include <WebConfigWebSockets.h>


// Web server
#include <LittleFS.h>

#ifdef ESP32
  #ifdef USE_ASYNC_WEBSERVER
    #include <AsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #include "AsyncJson.h"          // To handle JSON post request using AsyncCallbackJsonWebHandler
    #pragma message ( "WebConfig running AsyncWebServer for ESP32" )
  #else
    #include <WebServer.h>
    #pragma message ( "WebConfig running NO Async WebServer for ESP32" )
  #endif

#elif defined(ESP8266)
  #ifdef USE_ASYNC_WEBSERVER
    #include <ESPAsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #include "AsyncJson.h"          // To handle JSON post request using AsyncCallbackJsonWebHandler
    #pragma message ( "WebConfig running AsyncWebServer for ESP8266" )
  #else
    #include <ESP8266WebServer.h>
    #pragma message ( "WebConfig running NO Async WebServer for ESP8266" )
  #endif
#else
  #error Only ESP32 or ESP8266 builds are supported
#endif

// Logs
#ifdef ESP8266
  #include <ESP8266Log.h>
#endif

// WiFi & Network
#include "WebConfigNetwork.h"
#include "WebConfigNetworkObserver.h"


#include <ArduinoJson.h>



class WebConfigServer:
    public WebConfigNetworkObserver {

public:

  WebConfigStatus config_status = CONFIG_NOT_LOADED;

  WebConfigNetwork network;
#ifndef DISABLE_WEBCONFIG_MQTT
  WebConfigMQTT mqtt;
#endif

  struct FTP {
    bool enabled;
    String user;
    String password;
  };
  FTPServer ftpSrv;
  size_t totalBytes;
  size_t usedBytes;
  size_t freeBytes;


  struct DeepSleep {
    bool enabled;
    String mode;
    String mode_options[JSON_MAX_SIZE_LIST];
    float sleep_time;
    float sleep_delay;
  };

  struct LightSleep {
    bool enabled;
    String mode;
    String mode_options[JSON_MAX_SIZE_LIST];
  };

  struct NTP {
    bool enabled;
    // const char* ntpServer = new char(30);   // Max 30 char
    String ntpServer;   // Max 30 char
    long gmtOffset_sec;
    int daylightOffset_sec;
  };
  bool cbtime_set = false;
  #if defined(ESP8266)
    timeval cbtime;			// time set in callback

    // Callback function to know when the time is synch with ntp server:
    void time_is_set(void) {
      gettimeofday(&cbtime, NULL);
      cbtime_set = true;
      Serial.println("------------------ settimeofday() was called ------------------");
    }
  #endif
  struct Services {
    WebConfigOTA ota;
    FTP ftp;
    WebConfigWebSockets webSockets;
    DeepSleep deep_sleep;
    LightSleep light_sleep;
    NTP ntp;
  } services;

  struct Device {
    bool track_restart_counter;
    int loop_time_ms;
    int publish_time_ms;

  } device;

  struct Info {
    int restart_counter;
    String fw_version;
    String repo;

  } info;



  WebConfigServer(void);
  
  bool initWebConfigs(void);
  bool begin(void);
  void loop(void);

  WebConfigStatus status(void) { return config_status;};

  void addConfig(IWebConfig& config, String nameObject);
  void addDashboardObject(String key, String (*valueFunction)()) { services.webSockets.addObjectToPublish(key, valueFunction);}

#ifndef DISABLE_WEBCONFIG_MQTT
  PubSubClient *getMQTTClient(void) { return mqtt.getMQTTClient(); }
  void setMQTTClientId(String client_id) { mqtt.setMQTTClientId(client_id); }
  String getDeviceTopic(void) { return mqtt.getBaseTopic(); }
#endif

  unsigned long getDeviceSetupTime(void) {return deviceSetupTime; }
  bool getTimeSet(void) {return cbtime_set; }

  String formatBytes(size_t bytes);
  void updateSizeLittleFS(bool print = false){

    #ifdef ESP32
    totalBytes = LittleFS.totalBytes();
    usedBytes = LittleFS.usedBytes();
    #elif defined(ESP8266)
    FSInfo fs_info;
    LittleFS.info(fs_info);
    totalBytes = fs_info.totalBytes;
    usedBytes = fs_info.usedBytes;
    #endif

    freeBytes  = totalBytes - usedBytes ;
    if(print){
      Serial.println("File system memory size: ");
      Serial.print("Total: ");
      Serial.println(formatBytes(totalBytes));
      Serial.print("Used: ");
      Serial.println(formatBytes(usedBytes));
      Serial.print("Available: ");
      Serial.println(formatBytes(freeBytes));
    }
  };
  String getTotalBytes(){return formatBytes(totalBytes);}
  String getUsedBytess(){return formatBytes(usedBytes);}
  String getFreeBytes(){return formatBytes(freeBytes);}

  void setPreSleepRoutine(void (*routine)()) { 
    this->preSleep_routine = routine; 
    preSleep_routine_configured = true;
  };

private:

  LinkedList<IWebConfig*> configs = LinkedList<IWebConfig*>();
  LinkedList<IWebConfig*> configsServices = LinkedList<IWebConfig*>();
  // SimpleList<IWebConfig*> configs = SimpleList<IWebConfig*>();
  // SimpleList<IWebConfig*> configsServices = SimpleList<IWebConfig*>();

  #ifdef USE_ASYNC_WEBSERVER
    AsyncWebServer * server;
  #elif defined(ESP32)
    WebServer * server;
  #elif defined(ESP8266)
    ESP8266WebServer * server;
  #endif

  unsigned long currentLoopMillis = 0;

  // Device Sleep
  unsigned long deviceSetupTime = 0;
  void (*preSleep_routine)();
  bool preSleep_routine_configured = false;
  


  void configureServer(void);

  void addConfigService(IWebConfig& config, String nameObject);
  void parseIWebConfig(const JsonDocument& doc);
  void parseIWebConfigService(const JsonDocument& doc);

  void enableServices(void);
  

  bool saveWebConfigurationFile(const char *filename, const JsonDocument& doc);
  void parseConfig(const JsonDocument& doc);

  void loadConfigurationFile(const char *filename);
  void saveConfigurationFile(const char *filename);
  void printFile(String filename);
  bool restoreBackupFile(String filenamechar);

  #ifdef USE_ASYNC_WEBSERVER
    void updateGpio(AsyncWebServerRequest *request);
    bool handleFileRead(AsyncWebServerRequest *request, String path);
    static void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
  #elif defined(ESP32)
    void updateGpio(WebServer *server);
    bool handleFileRead(WebServer *server, String path);
  #elif defined(ESP8266)
    void updateGpio(ESP8266WebServer *server);
    bool handleFileRead(ESP8266WebServer *server, String path);
  #endif

  #if ESP32 && IP_NAPT
    esp_err_t enableNAT(void);
    void handleAPStations(void);
  #endif
  void networkRestart(void);

  void deepSleepHandler(void);


  String getContentType(String filename);

  // WebConfigNetworkObserver
  void onNetworkConnected() {
    log_w("WebConfigServer receives Network connected!");

    config_status = SERVER_INITIALIZING;

    // Configure and start the server:
    WebConfigServer::configureServer();

    // Reenable services:
    WebConfigServer::enableServices();

    log_w("WebConfigServer onNetworkConnected done");
    log_d("Heap size: %u\n", ESP.getFreeHeap());

    config_status = SERVER_RUNNING;

  }
  void onNetworkDisconnected() {
    log_e("WebConfigServer receives Network diconnected!");
    // TODO: stop old WebConfigServer client connections here
    config_status = SERVER_DOWN;

  }

};
#endif
