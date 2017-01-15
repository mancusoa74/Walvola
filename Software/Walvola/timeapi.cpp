#include "timeapi.h"


//query www.timeapi.org servce for the current time using this query
//http://www.timeapi.org/cet/now?format=%25Y-%25m-%25d--%25H-%25M-%25S
//service provide time information in this format: 2016-12-01--21-44-47
String get_web_time()
{
        int retry = 0;
        String html;
        WiFiClient http_client;

        //try TIMEAPI_RETRY times to connect to service
        while (!http_client.connected() && (retry < TIMEAPI_RETRY)) {
                if (http_client.connect("www.timeapi.org", 80) ) {
                        log("Connected to web time api");
                } else {
                        log("H");
                }

                retry++;
                delay(500);
        }

        //try TIMEAPI_RETRY to send time request
        while (!http_client.available() && (retry < TIMEAPI_RETRY)) {
                http_client.println("GET /cet/now?format=%25Y-%25m-%25d--%25H-%25M-%25S HTTP/1.1");
                http_client.println("Host:www.timeapi.org");
                http_client.println("User-Agent:Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/54.0.2840.71 Safari/537.36");
                http_client.println("Connection: close");
                http_client.println();

                log("getting timeapi time");
                delay(500);
        }

        while(http_client.available()) {
                char c = http_client.read();
                html += c;
        }

        log("TIMEAPI ANSWER:");
        log(html);

        //parse and format reply from www.timeapi.org
        String timestamp = html.substring(247);

        if (timestamp.length() != 20)
                timestamp = "2099-01-01--08-08-08";

        log("TIMESTAMP:");
        log(timestamp);

        http_client.stop();

        return timestamp;
}

//helper to get hours, minutes or seconds
int get_web_time_hours(String timestamp)
{
        //2016-12-01--22-09-02
        return timestamp.substring(12, 14).toInt();
}

int get_web_time_minutes(String timestamp)
{
        //2016-12-01--22-09-02
        return timestamp.substring(15, 17).toInt();
}

int get_web_time_seconds(String timestamp)
{
        //2016-12-01--22-09-02
        return timestamp.substring(18, 20).toInt();
}

