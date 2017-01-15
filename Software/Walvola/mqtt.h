#ifndef MQTT_H
#define MQTT_H

#include "Walvola.h"


#define MQTT_UNAME       "YOUR MQTT BROKER ACCOUNT USER NAME"     //MQTT brokor user name - I use tis broker https://www.cloudmqtt.com/
#define MQTT_PASSW       "YOUR MQTT BROKER ACCOUNT PASSWORD" //MQTT broker passowrd
#define MQTT_BROKER_PORT 17780          //MQTT BROKER listening port

//MQTT command definition
#define CMD_GET_STATUS  "GET_STATUS" //request the status of device
#define CMD_SET_STATUS  "SET_STATUS" //set the status of device
#define CMD_GET_INFO    "GET_INFO"   //get device information including IP address
#define CMD_UPDATE      "UPDATE"     //set device in OTA mode

//MQTT command type
#define TYPE_REQUEST    "REQUEST"
#define TYPE_REPLY      "REPLY"


//forward declaration
extern const String mqtt_client_id;
int MQTT_init();
void mqtt_disconnect() ;
void mqtt_log(String mex);

#endif
