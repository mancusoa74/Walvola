#ifndef WIFI_H
#define WIFI_H

#include "Walvola.h"

#define WIFI_RETRY_DELAY     500 //retry delay to connect to wifi
#define MAX_WIFI_INIT_RETRY  50  //max number of retry to connect to wifi
#define OTA_PORT             9999 //the port to connect with your browser to perform OTA update

//forward declaration
int WiFi_init();
extern WiFiClient wifi_client;
void wifi_wakeup();
void wifi_disconnect();
void OTA_init();
extern ESP8266WebServer httpServer;


#endif
