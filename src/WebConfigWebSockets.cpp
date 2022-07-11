#include "WebConfigWebSockets.h"

// String getHeapFree(void ){
//   return String(GET_FREE_HEAP);
// };

void WebConfigWebSockets::init(void) {
  // Serial.println("Starting Websocket server...");

  webSocket = new WebSocketsServer(this->port);

  // start webSocket server
  webSocket->begin();
  webSocket->onEvent(webSocketEvent);

  // Exaple to add a websocket function:
  // listObjets[0] = "heap_free";
  // listObjetFunctions[0] = getHeapFree;
  // listObjetsIndex++;
  // this->addObjectToPublish("heap_free", getHeapFree);

};


void WebConfigWebSockets::webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip =  webSocket->remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            webSocket->sendTXT(num, "{\"Connected\": true}");
        }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] Text received: %s\n", num, payload);

            if(payload[0] == '#') {
                // we get RGB data

                // decode rgb data
                // uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);

            //     analogWrite(LED_RED, ((rgb >> 16) & 0xFF));
            //     analogWrite(LED_GREEN, ((rgb >> 8) & 0xFF));
            //     analogWrite(LED_BLUE, ((rgb >> 0) & 0xFF));
            // }

            break;
    }
  }

};


void WebConfigWebSockets::publishClients(void) {

  // int i = webSocket->connectedClients(ping);
  // Serial.printf("%d Connected websocket clients ping: %d\n", i, ping);

  // To send msg to all connected clients:
  // webSocket->broadcastTXT("message here");
  // To send msg to specific client id:
  // webSocket->sendTXT(i-1, msg_ws.c_str());

  String msg_ws = "{";

  for (int i = 0; i <= this->listObjetsIndex-1; i++) {
    msg_ws +="\""+ this->listObjets[i] + "\": " + (*this->listObjetFunctions[i])();
    if (this->listObjetsIndex-1 != i) msg_ws += " , ";
    else msg_ws += "}";

  };

  // Serial.println(msg_ws.c_str());
  // webSocket->sendTXT(i-1, msg_ws.c_str());
  // webSocket->sendTXT(0, msg_ws.c_str());
  webSocket->broadcastTXT(msg_ws.c_str());

};


void WebConfigWebSockets::handle(void) {
  currentWSMillis = millis();
  webSocket->loop();
  if(currentWSMillis - previousWSMillis > (unsigned)this->publish_time_ms) {
    WebConfigWebSockets::publishClients();
    previousWSMillis = currentWSMillis;
  }
};


bool WebConfigWebSockets::addObjectToPublish(String key, String (*valueFunction)()) {

  if (this->listObjetsIndex+1 <= MAX_WS_FUNCTIONS){
    this->listObjets[listObjetsIndex] = key;
    this->listObjetFunctions[listObjetsIndex] = valueFunction;
    this->listObjetsIndex++;
    // Serial.print("Added WebSocket msg object[");
    // Serial.print(this->listObjetsIndex);
    // Serial.println("]: " + key);

    return true;
  } else {
    // Serial.print("NOT added WebSocket msg object[");
    // Serial.print(this->listObjetsIndex);
    // Serial.println("]: " + key);

    return false;
  }
};


void WebConfigWebSockets::parseWebConfig(JsonObjectConst configObject){
  this->enabled = configObject["enabled"] | false;
  this->publish_time_ms = configObject["publish_time_ms"];
  this->port = configObject["port"];
};