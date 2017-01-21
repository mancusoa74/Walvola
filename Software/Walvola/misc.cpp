#include "misc.h"
#include <EEPROM.h>

String walvola_status = WALVOLA_ON; //initial Walvola status
String relay_status   = RELAY_OFF; //initial IRB status


void eeprom_init()
{
        EEPROM.begin(1);
}

//read from EEPROM walvola status
String get_eeprom_walvola_status()
{
        byte status = EEPROM.read(EEPROM_WALVOLA_STATUS_ADDRESS);

        log("eeprom read");
        if (status > 1) {
               log("eeprom not set");
                return "EEPROM_NOT_SET";
        }
        log(String(status));

        return (status==1?WALVOLA_ON:WALVOLA_OFF);
}

//store in EEPROM walvola status
void set_eeprom_walvola_status(String status)
{
        log("eeprom write");
        log(status);

        byte value = (status==WALVOLA_ON?1:0);
        log(String(value));

        EEPROM.write(EEPROM_WALVOLA_STATUS_ADDRESS, value);
        EEPROM.commit();
}

//get ESP voltage level
String get_voltage()
{
        String voltageStr = String (readvdd33(), DEC);
        log("reading voltage:");
        log(voltageStr);

        return voltageStr;
}

//read voltage from ESP ans post it to https://keen.io
void keen_voltage()
{
        String voltage = get_voltage();
        keen_post(keen_voltage_collection,  "{\"voltage\":" + voltage + "}");
}

//classical not blocking delay
void millis_delay(unsigned long interval)
{
        unsigned long start_time = millis();

        while (millis() < (start_time + interval)) {delay(0);}
}


void GPIO_init()
{ 
        pinMode(GPIO2, OUTPUT);
        pinMode(GPIO0, OUTPUT);

        //the below two are key as motor must be off during initalization (WALVOLA_ROLE)
        //the below two are key as relay must be off during initalization (IRB_ROLE)
        digitalWrite(GPIO2, LOW); 
        digitalWrite(GPIO0, LOW);

        log("GPIOs initialization SUCCESS.........");
        delay(200);
}

//valid combination to drive H-bridge circuit are
//LOW-LOW : motor off
//LOW-HIGH: motor spin clockwise
//HIGH-LOW: motor spin counter clock wise
//HIGH-HIGH: not allowed

//spin the motor clockwise
void motor_right()
{
#ifdef WALVOLA_ROLE 
        digitalWrite(GPIO_MOTOR_LEFT, LOW);
        digitalWrite(GPIO_MOTOR_RIGHT, HIGH);   
        log("Motor RIGHT.........");
        delay(200);
#endif
}

//sping the motor counter clockwise
void motor_left()
{
#ifdef WALVOLA_ROLE 
        digitalWrite(GPIO_MOTOR_LEFT, HIGH);
        digitalWrite(GPIO_MOTOR_RIGHT, LOW);      
        log("Motor LEFT.........");
        delay(200);
#endif
}

//stop the motor spinning
void motor_off()
{
#ifdef WALVOLA_ROLE 
        digitalWrite(GPIO_MOTOR_LEFT, LOW);
        digitalWrite(GPIO_MOTOR_RIGHT, LOW);      
        log("Motor OFF.........");
        delay(200);
#endif
}

//turn IRB ready led
void irb_ready()
{
#ifdef IRB_ROLE
        digitalWrite(GPIO_READY, HIGH);
        log("IRB ready........");  
#endif
}

//turn relay ON
void irb_relay_on()
{
#ifdef IRB_ROLE
        digitalWrite(GPIO_RELAY, HIGH);
        log("IRB Relay ON.....");
#endif
}

//turn relay OFF
void irb_relay_off()
{
#ifdef IRB_ROLE
        digitalWrite(GPIO_RELAY, LOW);
        log("IRB Relay OFF.....");
#endif
}

//wrapper to drive IRB status
void set_irb(String state)
{
        if(state == RELAY_ON) {
                log("Opening Relay....");
                relay_status = RELAY_ON;
                irb_relay_on();
                log("Relay open SUCCESS....");

                //now we send a telegram notification to the heating system bot to notify user of status change
                tgram_sendmex("Walvola[" + String(WALVOLA_LABEL) + "] APERTA"); 
        } 

        if(state == RELAY_OFF) {
                log("Closing Relay....");
                relay_status = RELAY_OFF;
                irb_relay_off();
                log("Relay close SUCCESS....");
                tgram_sendmex("Walvola[" + String(WALVOLA_LABEL) + "] CHIUSA");
        }
}

//wrapper to drive Walvola status
void set_walvola(String state)
{
//  if (state == WALVOLA_ON &&  walvola_status == WALVOLA_OFF) { // as I do not implement encoder controlled motor, I should implement this logic. for the time being disabled
        if (state == WALVOLA_ON) {
                log("Opening walvola.........");

                //start the motor in the right direction
                //wait for the proper time for walvola to open/close
                //stop the motor
                walvola_status = WALVOLA_TRANSITION;    
                motor_off();
                motor_right();
                delay(WALVOLA_DELAY_ON);
                motor_off();
                walvola_status = WALVOLA_ON;

                //store status in EEPROM
                set_eeprom_walvola_status(walvola_status);

                //telegram notification to bot
                log("walvola open SUCCESS");
                tgram_sendmex("Walvola[" + String(WALVOLA_LABEL) + "] APERTA");
        }
  //if (state == WALVOLA_OFF &&  walvola_status == WALVOLA_ON) {
        if (state == WALVOLA_OFF) {
                log("Closing walvola.........");

                walvola_status = WALVOLA_TRANSITION;    
                motor_off();
                motor_left();
                delay(WALVOLA_DELAY_OFF);
                motor_off();
                walvola_status = WALVOLA_OFF;

                set_eeprom_walvola_status(walvola_status);

                log("walvola close SUCCESS");
                tgram_sendmex("Walvola[" + String(WALVOLA_LABEL) + "] CHIUSA");
        }       
}

//stop communication to external world
void stop_inet_connectivity()
{
        mqtt_disconnect();

        //is modem sleep mode we need to peroperly set ESP8266
        #ifdef WALVOLA_MODEM_SLEEP_MODE
               wifi_disconnect();
        #endif

        inet_connected = false;
}

//start communication to external world
void start_inet_connectivity()
{
        //in modem sleep need to explicitly wake up the EPS8266
        #ifdef WALVOLA_MODEM_SLEEP_MODE
               wifi_wakeup();
        #endif

        //connect to wifi
        if (WiFi_init() != WL_CONNECTED) {
                log("Cannot connect to WiFi AP..........");
                log("Aborting program!");
        } else {
                inet_connected = true;
                delay(500);
                log("Connection to WiFi succesfull!");

                //get web time from service
                walvola_time = get_web_time();

                ///init MQTT connectiont o broker and MQTT call back and process MQTT message
                if (MQTT_init()) {
                        //in IRB mode let's switch ON the ready led
                        #ifdef IRB_ROLE
                                irb_ready();
                        #endif

                        log("Initializing MQTT communication SUCCESS.........");
                } else {
                        log("MQTT_INIT error......cannot Initialize MQTT ERROR");
                }
        }
}

//this function change the modem sleep period based on the current time
//from midnight to 5AM let's skeep. this is done to increase battery duration
//however I am deprecating this as walvola is ALWAYS in deep sleep mode
unsigned long dynamic_sleep_period()
{
        unsigned long walvola_delay = 0L;

        log("Calculating dynamic sleep period for walvola...");

        String current_time = get_web_time();
        log("current time:");
        log(current_time);

        int current_hour = get_web_time_hours(current_time);
        log("current hour:");
        log(current_hour);

        if ((current_hour >= 0) && (current_hour <= 4)) {
                log("night time");
                walvola_delay = WALVOLA_NIGHT_SLEEP_PERIOD;
        } else {
                log("day time");
                walvola_delay = WALVOLA_DEFAULT_SLEEP_PERIOD;
        }
        return walvola_delay;
}
