#ifndef WebConfigServer_H
#define WebConfigServer_H

#include <Arduino.h>

// To enable asyc webserver use platformio build_flags = -D USE_ASYNC_WEBSERVER
// Or define here and in main.cpp:
// #define USE_ASYNC_WEBSERVER

// Allocate a temporary JsonDocument
// Don't forget to change the capacity to match your requirements.
// Use arduinojson.org/assistant to compute the capacity.
#define CONFIG_FILE "/config/config.json"

#ifndef CONFIG_JSON_SIZE
  #define CONFIG_JSON_SIZE 4096
#endif
#define DYNAMIC_JSON_DOCUMENT_SIZE CONFIG_JSON_SIZE   //  To redefine "AsyncJson.h" AsyncCallbackJsonWebHandler json doc max size
#define MIN_HEAP_SIZE_FOR_SAVING_CONFIG 3000

#define MQTT_TOPIC_MAX_SIZE_LIST 10
#define JSON_MAX_SIZE_LIST 6

#define CONFIG_LOADED "loaded"
#define CONFIG_NOT_LOADED "not_loaded"

#define ARDUINOJSON_ENABLE_ALIGNMENT 1


// #include <LinkedList.h>
// Using AsyncWebServer LinkedList lib can not be used because there is a class
// using the same name. For that reason, for now we use SimpleList until we fix
// this using namespace for example.
#include <SimpleList.h>
#include "IWebConfig.h"


// WebConfigOTA
#include "WebConfigOTA.h"

// FTP server
#include <ESP8266FtpServer.h>
#include <FS.h>

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


#ifdef ESP32
  #include <SPIFFS.h>
  #ifdef USE_ASYNC_WEBSERVER
    #include <AsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #include "AsyncJson.h"          // To handle JSON post request using AsyncCallbackJsonWebHandler
  #else
    #include <WebServer.h>
  #endif

#elif defined(ESP8266)
  #include <FS.h>
  #ifdef USE_ASYNC_WEBSERVER
    #include <ESPAsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #include "AsyncJson.h"          // To handle JSON post request using AsyncCallbackJsonWebHandler
  #else
    #include <ESP8266WebServer.h>
  #endif
#endif

#include <ArduinoJson.h>



class WebConfigServer {

public:

  // const char* status = new char(32);
  String config_status;

  struct Network {
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
  } network;

  struct Mqtt {
    bool enabled;
    String server;
    int port;
    String id_name;
    bool reconnect_mqtt;
    bool enable_user_and_pass;
    String user_name;
    String user_password;
    bool enable_certificates;
    String ca_file;
    String cert_file;
    String key_file;
    String pub_topic[MQTT_TOPIC_MAX_SIZE_LIST];
    String sub_topic[MQTT_TOPIC_MAX_SIZE_LIST];
  } mqtt;

  struct FTP {
    bool enabled;
    String user;
    String password;
  };
  FtpServer ftpSrv;


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
  #if defined(ESP8266)
    timeval cbtime;			// time set in callback
    bool cbtime_set = false;

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

  #ifdef USE_ASYNC_WEBSERVER
    void configureServer(AsyncWebServer *server);
  #else
    #ifdef ESP32
      void configureServer(WebServer *server);
    #elif defined(ESP8266)
      void configureServer(ESP8266WebServer *server);
    #endif
  #endif

  void handle(void);
  bool begin(void);
  void loop(void);

  String status(void) { return config_status;};

  void addConfig(IWebConfig* config, String nameObject);
  void addDashboardObject(String key, String (*valueFunction)()) { services.webSockets.addObjectToPublish(key, valueFunction);}



private:

  // LinkedList<IWebConfig*> configs = LinkedList<IWebConfig*>();
  SimpleList<IWebConfig*> configs = SimpleList<IWebConfig*>();


  void addConfigService(IWebConfig* config, String nameObject);
  void parseIWebConfig(const JsonDocument& doc);
  void parseIWebConfigService(const JsonDocument& doc);

  void enableServices(void);
  
  String formatBytes(size_t bytes);

  bool saveWebConfigurationFile(const char *filename, const JsonDocument& doc);
  void parseConfig(const JsonDocument& doc);

  void loadConfigurationFile(const char *filename);
  void saveConfigurationFile(const char *filename);
  void printFile(String filename);
  bool restoreBackupFile(String filenamechar);

  #ifdef USE_ASYNC_WEBSERVER
    void updateGpio(AsyncWebServerRequest *request);
    bool handleFileRead(AsyncWebServerRequest *request, String path);
  #else
    #ifdef ESP32
      void updateGpio(WebServer *server);
      bool handleFileRead(WebServer *server, String path);
    #elif defined(ESP8266)
      void updateGpio(ESP8266WebServer *server);
      bool handleFileRead(ESP8266WebServer *server, String path);
    #endif
  #endif

  String getContentType(String filename);


};
#endif
