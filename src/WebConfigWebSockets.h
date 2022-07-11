#ifndef WebConfigWebSockets_H
#define WebConfigWebSockets_H

#include <Arduino.h>

// WebSockets Includes
#include <WebSocketsServer.h>

#ifndef MAX_WS_FUNCTIONS
#define MAX_WS_FUNCTIONS 10
#endif


#include "IWebConfig.h"


static WebSocketsServer* webSocket ;


class WebConfigWebSockets: public IWebConfig{


// List of objects keys:
String listObjets[MAX_WS_FUNCTIONS];

// List of Object values for the listObjects. It represents the value for the key
// and it will be the String result of the function:
String (*listObjetFunctions[MAX_WS_FUNCTIONS])();
int listObjetsIndex = 0;

int port = 81;
bool enabled = false;
int publish_time_ms;

unsigned long currentWSMillis = 0;
unsigned long previousWSMillis = 0;

public:

  void setPort(int port) {this->port = port; };
  void init(void);
  static void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
  void publishClients(void);
  void handle(void);
  bool addObjectToPublish(String key, String (*valueFunction)());

  bool isEnabled(void) { return this->enabled; }
  int getPublishTime(void) { return this->publish_time_ms; }
  void parseWebConfig(JsonObjectConst configObject);

};

#endif
