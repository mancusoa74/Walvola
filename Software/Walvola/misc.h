#ifndef MISC_H
#define MISC_H

#include "Walvola.h"

#ifdef WALVOLA_ROLE //ESP8266-12
  #define GPIO2              4
  #define GPIO0              5
  #define GPIO_MOTOR_LEFT    5
  #define GPIO_MOTOR_RIGHT   4
#endif

#ifdef IRB_ROLE //ESP8266-01
  #define GPIO2              2
  #define GPIO0              0
  #define GPIO_READY         0
  #define GPIO_RELAY         2
#endif

//device status definition
#define WALVOLA_ON         "ON"
#define WALVOLA_OFF        "OFF"
#define WALVOLA_TRANSITION "TRANSITION"
#define WALVOLA_OTA        "OTA"
#define RELAY_ON           "ON"
#define RELAY_OFF          "OFF"

#define EEPROM_WALVOLA_STATUS_ADDRESS 0  //EEPROM's address to store the device status

//forward declaration
extern String walvola_status;
extern String relay_status;
extern const String walvola_label;
extern const String keen_voltage_collection;
extern "C" uint16_t readvdd33(void);

String get_voltage();
void keen_voltage();
void millis_delay(unsigned long interval);
void GPIO_init();
void motor_right();
void motor_left();
void motor_off();
void irb_ready();
void irb_relay_on();
void irb_relay_off();
void set_irb(String state);
void set_walvola(String state);
void logzio_debug(String mex);
void stop_inet_connectivity();
void start_inet_connectivity();
void set_walvola_status(String state);
void eeprom_init();
String get_eeprom_walvola_status();
void set_eeprom_walvola_status(String status);
unsigned long dynamic_sleep_period();

#endif
