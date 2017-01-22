/*
#Author:   Antonio Mancuso
#Purpose:  Walvola: code to run on any SMHA esp8266-01/12 module. This code can impersonate different roles

#Version:  1.4.1 - 22/01/2017 - fixing time calculation
#Version:  1.4   - 21/01/2017 - replacing  www.timeapi.org as this service is not available anymore   
#Version:  1.3   - 11/01/2017 - clean up for publishing on github - Teleram disbale due to some issue with SSL https://github.com/espressif/ESP8266_MESH_DEMO/issues/19 - https://github.com/esp8266/Arduino/issues/1375
#Version:  1.2   - 24/12/2016 - adding OTA support
#Version:  1.1   - 08/12/2016 - intrducing deep_sleep mode
#Version:  1.0   - 01/12/2016 - adding dynamic WALVOLA_SLEEP_PERIOD and WALVOLA_SLEEP_DELAY based on time of the day
#Version:  0.9   - 27/11/2016 - switching to mutiple queues model
#Version:  0.8   - 24/11/2016 - code refactoring
#Version:  0.7   - 20/11/2016 - Adding WiFi OFF period
#Version:  0.6   - 13/11/2016 - Switching to CloudMQTT as PubNub looks like is not working anymore and adding voltage post to Keen.io
#Version:  0.5   - 06/11/2016 - Adding Telegram Bot support
#Version:  0.4   - 04/11/2016 - Adding support for IRB
#Version:  0.3   - 18/09/2016 - Removal of ArduinoJson library and manual JSON parsing
#Version:  0.2   - 18/09/2016 - clean up and MQTT message processing logic
*/

#include "Walvola.h"

/*
 * =========== Global variables to configure behavior ===========
*/
const String keen_voltage_collection = "YOUR KEEN.IO VOLTAGE COLLECTION" + String(WALVOLA_LABEL); //I post voltage value to a https://keen.io collection for statistics and chart
unsigned long walvola_sleep_period   = WALVOLA_DEFAULT_SLEEP_PERIOD;
String walvola_time                  = "1970-01-01--00-00-00"; //default time in honor of Unix
boolean inet_connected               = false; //track whether connected to Internet or not
boolean update_mode                  = false; //track whether the walvola is in OTA update mode
unsigned long last_schedule          = millis();

void setup()
{
        // Setup Serial Console only in DEBUG mode
        if (DEBUG){ 
                Serial.begin(SERIAL_SPEED);
                delay(SERIAL_INIT_DELAY);
                log("");
                log("Walvola Software " + String(VERSION) + " starting ..........");
                log(mqtt_client_id);
        }

        // walvola should maintain the status (ON/OFF) as it a battery powered device. in case battery discharges the status of walvola is not changing as the motor is not spinning
        // irb is driving a relay,so in case power is removed the relay switch back to normal position and status reset so we don't need to keep status for it
        #ifdef WALVOLA_ROLE
                eeprom_init();

                //get the status of walvola in eeprom and if not set, set it to WALVOLA_ON (default status of walvola)
                if ((walvola_status = get_eeprom_walvola_status()) == "EEPROM_NOT_SET") {
                        log("eeprom walvola status not set.... initializing it");
                        walvola_status = WALVOLA_ON;
                        set_eeprom_walvola_status(walvola_status);
                }

                log("Inital Walvola Status = ");
                log(walvola_status);
        #endif

        //basic initialization of the two GPIO pins used by Walvola and IRB
        GPIO_init();

        //this is the main code executed for device in DEEP_SLEEP_MODE as loop will be empty
        //in my setup walvola is ESP8266-12 so I use deep sleep
        #ifdef WALVOLA_DEEP_SLEEP_MODE
                log("DEEP SLEEP MODE");

                //the following chuck of code performs the following tasks
                //1- connect to wifi if not connected
                //2- retrieve web time (no needs for RTC when you have Internet :) )
                //3- initialize connection to MQTT broker
                //4- check if MQTT message is available and process it
                if (WiFi.status() != WL_CONNECTED) {
                        log("WiFi connecting...");
                        start_inet_connectivity();  
                }

                // get voltage from power source and post it to https://keen.io service
                log("KEEN VOLTAGE");
                keen_voltage();

                //disconnet from Wifi - this might be skipped
                if (WiFi.status() == WL_CONNECTED) {
                        log("WiFi disconnecting...");
                        stop_inet_connectivity();
                }

                //if devie is NOT in OTA_MODE goes to deep_sleep for WALVOLA_DEFAULT_DEEP_SLEEP_PERIOD seconds
                if (!update_mode) {
                        log("going to DEEP SLEEP");
                        ESP.deepSleep(WALVOLA_DEFAULT_DEEP_SLEEP_PERIOD * 1000000);
                } 
                
                log("Entering OTA mode...");
        #endif
}

void loop()
{
        //if device is in OTA_MODE let's handle client request
        #ifdef OTA_MODE
                httpServer.handleClient();
        #endif

        //this is the main code executed for device in MODEM_SLEEP_MODE
        //in my setup walvola is ESP8266-01 so I use modem sleep
        //most of the logic is the same as for deep sleep
        #ifdef WALVOLA_MODEM_SLEEP_MODE
                if (millis() > (last_schedule  + walvola_sleep_period)) {
                        if (WiFi.status() != WL_CONNECTED) {
                                log("WiFi connecting...");
                                start_inet_connectivity();
                        }

                        keen_voltage();
                        last_schedule = millis();

                        //to increase the duration of batteries during night time the sleep time is increased to 4 hrs
                        walvola_sleep_period = dynamic_sleep_period();

                        log("Calculated dynamic walvola sleep time is:");
                        log(walvola_sleep_period);
                } else {
                        if (WiFi.status() == WL_CONNECTED) {
                                log("WiFi disconnecting...");
                                stop_inet_connectivity(); 
                        }
                        log("sleeping...");
                        delay(WALVOLA_SLEEP_DELAY);
                        //delay(walvola_sleep_period);
                }
        #endif
}
