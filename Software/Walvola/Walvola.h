/*
#Author:   Antonio Mancuso
#Purpose:  Walvola: code to run on any SMHA esp8266-01 module. This code can impersonate different roles
*/

#ifndef WALVOLA_H
#define WALVOLA_H

extern "C"
{
        #include "user_interface.h"
}


#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESPTelegramBot.h>
#include <MQTT.h>
#include <PubSubClient.h>
#include <jsmn.h> //https://github.com/zserge/jsmn
#include <Arduino.h>
#include "keen.h"
#include "misc.h"
#include "telegram.h"
#include "json.h"
#include "wifi.h"
#include "mqtt.h"
#include "timeapi.h"

/*
 * =========== Global defines to configure behavior ===========
*/

//#define WALVOLA_DEEP_SLEEP_MODE  //uncomment to enable deep sleep mode
#define WALVOLA_MODEM_SLEEP_MODE //uncomment to enable modem sleep mode
#define WALVOLA_ROLE             //code behaves as a Walvola http://mancusoa74.blogspot.it/2017/01/come-aggiungere-il-controllo-wireless.html
//#define IRB_ROLE                 //code behaves as a IRB http://mancusoa74.blogspot.it/2015/11/internet-relay-board-v2.html
#define OTA_MODE                 //uncomment to enable OTA software upgrade http://esp8266.github.io/Arduino/versions/2.0.0/doc/ota_updates/ota_updates.html#web-browser

#define DEBUG              1           //1=enable DEBUG mode; 0=disble debug mode
#define MQTT_DEBUG         1           //1=enable MQTT DEBUG mode; 0=disble MQTT debug mode; post debug message to a /debug topic
#define VERSION            "v1.3(00021)" 
#define SERIAL_SPEED       115200
#define SERIAL_INIT_DELAY  10
#define WALVOLA_DELAY_ON   35000       //time (ms) for which the walvola motor is activated to open the walvola
#define WALVOLA_DELAY_OFF  35000       //time (ms) for which the walvola motor is activated to close the walvola
#define WALVOLA_LABEL      "YOUR UNIQUE VALVOLA NAME"   //walvola name which is shown on mobile APP
#define WALVOLA_ID         "YOUR UNIQUE VALVOLA ID" //unique id of the walvola. it is used to define MQTT topic for communications with mobile app

#if DEBUG == 0
  #define WALVOLA_DEFAULT_DEEP_SLEEP_PERIOD 600 //deep sleep period in seconds (10 minutes)
  #define WALVOLA_DEFAULT_SLEEP_PERIOD 600000L  //modem sleep perion in ms (10 minutes) 
  #define WALVOLA_SLEEP_DELAY 300000            //loop delay in ms
#else
  #define WALVOLA_DEFAULT_DEEP_SLEEP_PERIOD 15  //debug
  #define WALVOLA_DEFAULT_SLEEP_PERIOD 10000L   //debug
  #define WALVOLA_SLEEP_DELAY 5000 //debug
#endif

#define WALVOLA_NIGHT_SLEEP_PERIOD 18000000L   //night modem sleep period (5 hrs)

extern String walvola_time;
extern boolean inet_connected;
extern boolean update_mode;

//---------- SUPPORT MACRO AND DEFINITION ----------
//#define log(mex) if (DEBUG) {Serial.println(walvola_time + "::" + mex);Serial.flush();}
#define log(mex) if (DEBUG) {Serial.println(walvola_time + "::" + mex);Serial.flush(); if(MQTT_DEBUG) {mqtt_log(walvola_time + "::" + mex);}}

#endif
