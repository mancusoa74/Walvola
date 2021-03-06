 /*
#Author:   Antonio Mancuso
#Purpose:  Walvola: code to run on any SMHA esp8266-01/12 module. This code can impersonate different roles

#Version   2.0.0 - summer 2017 - adding WAC support
#Version:  1.5.2 - 11/02/2017 - adding reset after deep sleep
#Version:  1.5.1 - 05/02/2017 - fixing telegram notification (28% 295513 and 58% 48168 are needed for telegram to work SSL certificate)
#Version:  1.5   - 04/02/2017 - control eeprom use trhough define - wifi disable during motor action to save energy
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
//const PROGMEM String keen_voltage_collection = "YOUR KEEN.IO VOLTAGE COLLECTION" + String(WALVOLA_LABEL); //I post voltage value to a https://keen.io collection for statistics and chart
const String keen_voltage_collection = "YOUR KEEN.IO VOLTAGE COLLECTION" + String(WALVOLA_LABEL); //I post voltage value to a https://keen.io collection for statistics and chart

//unsigned long walvola_sleep_period = WALVOLA_DEFAULT_SLEEP_PERIOD;
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
                log(" ");
                log("Walvola Software " + String(VERSION) + " starting ..........");
                log(mqtt_client_id);
        }

//        enum rst_reason {
//                REASON_DEFAULT_RST              = 0,    /* normal startup by power on */
//                REASON_WDT_RST                  = 1,    /* hardware watch dog reset */
//                REASON_EXCEPTION_RST    = 2,    /* exception reset, GPIO status wonâ€™t change */
//                REASON_SOFT_WDT_RST     = 3,    /* software watch dog reset, GPIO status wonâ€™t change */
//                REASON_SOFT_RESTART     = 4,    /* software restart ,system_restart , GPIO status wonâ€™t change */
//                REASON_DEEP_SLEEP_AWAKE = 5,    /* wake up from deep-sleep */
//                REASON_EXT_SYS_RST      = 6             /* external system reset */
//        };

//        #if defined(WALVOLA_ROLE) || defined(WAC_ROLE)
//            rst_info *rinfo = ESP.getResetInfoPtr();
//            log(String("ResetInfo.reason = ") + (*rinfo).reason); 
//            if ((*rinfo).reason == REASON_DEEP_SLEEP_AWAKE) {
//                    log("boot from deep sleep --> reset") ;
//                    ESP.restart() ;
//            }
//        #endif

        // walvola should maintain the status (ON/OFF) as it a battery powered device. in case battery discharges the status of walvola is not changing as the motor is not spinning
        // irb is driving a relay,so in case power is removed the relay switch back to normal position and status reset so we don't need to keep status for it
        #if defined(WALVOLA_ROLE) || defined(WAC_ROLE)
        #if EEPROM_ENABLE == 1
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
                        log("start_inet_connectivity(true)");  
                        start_inet_connectivity(true);  
                }

                // get voltage from power source and post it to https://keen.io service
                log("KEEN VOLTAGE");
                keen_voltage();

                //tgram_sendmex("telegram sent"); 
                
                //if devie is NOT in OTA_MODE goes to deep_sleep for WALVOLA_DEFAULT_DEEP_SLEEP_PERIOD seconds
                if (!update_mode) {
//                        stop_inet_connectivity();
//                        delay(5000);
                        log("going to DEEP SLEEP");
                        ESP.deepSleep(WALVOLA_DEFAULT_DEEP_SLEEP_PERIOD * 1000000);
                        delay(500); //candidate to go into 1.4.2
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
                if (millis() > (last_schedule  + WALVOLA_DEFAULT_SLEEP_PERIOD)) {
                        if (WiFi.status() != WL_CONNECTED) {
                                log("WiFi connecting...");
                                log("start_inet_connectivity(true)");  
                                start_inet_connectivity(true);
                        }

                        keen_voltage();
                        last_schedule = millis();

                        //to increase the duration of batteries during night time the sleep time is increased to 4 hrs
                        //walvola_sleep_period = dynamic_sleep_period();

                        //log("Calculated dynamic walvola sleep time is:");
                        //log(walvola_sleep_period);
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

