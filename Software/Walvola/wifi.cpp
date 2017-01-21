#include "wifi.h"

const char* wifi_ssid   = "YOUR WIFI SSID";                     //your Wifi SSID
const char* wifi_passwd = "YOUE WIFI PASSWORD"; //your wifi password
WiFiClient wifi_client;

//initialize OTA mode
#ifdef OTA_MODE
        ESP8266WebServer httpServer(OTA_PORT);
        ESP8266HTTPUpdateServer httpUpdater;
#endif

//connect to wifi
int WiFi_init()
{
        int retries = 0;

        log("Connecting to WiFi AP..........");

        WiFi.mode(WIFI_STA);
        WiFi.begin(wifi_ssid, wifi_passwd); //start connecting to WiFi AP

        //check the status of WiFi connection to be WL_CONNECTED
        while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY)) {
               retries++;
                delay(WIFI_RETRY_DELAY);
                log("#");
        }

        return WiFi.status(); //return the WiFi connection status
}

//wake up wifi from modem sleep mode http://www.espressif.com/sites/default/files/9b-esp8266-low_power_solutions_en_0.pdf
void wifi_wakeup()
{
        Serial.println("WiFi wakeup");

        //the following to bring ESP to station mode from modem sleep mode
        wifi_fpm_do_wakeup();
        wifi_fpm_close();
        wifi_set_opmode(STATION_MODE);
        wifi_station_connect();
        delay(100);
}

//set ESP to modem sleep mode
void wifi_disconnect()
{
        Serial.println("WiFi disconnect");

        //the following to put ESP in modem sleep mode
        wifi_station_disconnect();
        wifi_set_opmode(NULL_MODE);
        wifi_set_sleep_type(MODEM_SLEEP_T);
        wifi_fpm_open();
        wifi_fpm_do_sleep(0xFFFFFFF);
        delay(100);
}

//initialize OTA mode
#ifdef OTA_MODE
void OTA_init()
{
        httpUpdater.setup(&httpServer);
        httpServer.begin();
        MDNS.addService("http", "tcp", OTA_PORT);
}
#endif

