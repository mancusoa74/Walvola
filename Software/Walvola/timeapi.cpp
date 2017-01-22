#include "timeapi.h"

String months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//the www.timeapi.org servce is not availabvle anymore on the Internet
//for this reason I have replaced it with querying google home page
//and getting the date from the response header


//helper function to convert month from literal format to month number
int month2index (String month) 
{
        for (int i = 0; i < 12; i++) {
                if (months[i] == month)
                        return i + 1;  
        }
        return 0;
}

//helper function which pad number with 0 if necessary
String zero_padding(byte num)
{
        String padding;
        
        if (num < 10)
                padding = String(0) + String(num);
        else
                padding = String(num);
        
        return padding;
}
               
String get_web_time()
{
        int retry = 0;
        String html;
        WiFiClient http_client;

        //try TIMEAPI_RETRY times to connect to service
        while (!http_client.connected() && (retry < TIMEAPI_RETRY)) {
                if (http_client.connect("www.google.it", 80) ) {
                        log("Connected to web time api");
                } else {
                        log("H");
                }

                retry++;
                delay(500);
        }

        //try TIMEAPI_RETRY to send time request
        while (!http_client.available() && (retry < TIMEAPI_RETRY)) {
                http_client.println("GET");
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

        //parse and format reply from www.google.it
        //the google reply to parse is this one
        //HTTP/1.0 302 Found
        //Cache-Control: private
        //Content-Type: text/html; charset=UTF-8
        //Location: http://www.google.it/?gfe_rd=cr&ei=MGWDWP6sKKzCXqLErtAN
        //Content-Length: 256
        //Date: Sat, 21 Jan 2017 13:42:08 GMT
        //
        //<HTML><HEAD><meta http-equiv="content-type" content="text/html;charset=utf-8">
        //<TITLE>302 Moved</TITLE></HEAD><BODY>
        //<H1>302 Moved</H1>
        //The document has moved
        //<A HREF="http://www.google.it/?gfe_rd=cr&amp;ei=MGWDWP6sKKzCXqLErtAN">here</A>.
        //</BODY></HTML>

        int date_start = html.indexOf("Date"); //identify the Date in the http reponse
        int date_end = html.indexOf("GMT");
        
        String raw_timestamp = html.substring(date_start + 11, date_end - 4); //get the timestamp as provided (i.e Sat, 21 Jan 2017 13:42:08)
        String year = raw_timestamp.substring(7,11);  //get the year         
        String month = zero_padding(month2index(raw_timestamp.substring(3,6)));  //get month string and convert to month number with padding
        String day = raw_timestamp.substring(0,2);    //get the day
        //for hh we might have issues with daylight saving
        String hh = zero_padding(raw_timestamp.substring(12,14).toInt() + 1); // get the hour, convert to int, add one as Italy is GMT+1, convert back o string
        String mm = raw_timestamp.substring(15,17); //get the minutes
        
        String timestamp = year + "-" + month + "-" + day + "--" + hh + "-" + mm + "-00"; //build timestamp in expected format
        
        log("TIMESTAMP:");
        log(raw_timestamp);
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

