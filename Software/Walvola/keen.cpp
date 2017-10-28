#include "keen.h"

//post tghe provided data to a given keen.io collection through HTTP POST
void keen_post(String collection, String data)
{
        HTTPClient http;

        log("Keen POST begin...\n");
        http.setTimeout(2000);
        String url = "http://api.keen.io/3.0/projects/" + String(KEEN_PROJECT_ID) + "/events/" + collection;
        log(url);

        log("HTTP BEGIN");
        http.begin(url);
        log("HTTP BEGIN");
        http.addHeader("Authorization", KEEN_MASTER_KEY, false, false); //this is important to get authorization to POST data to collection
        
        http.addHeader("Content-Type", "application/json", false, false);

        if(http.POST(data) == HTTP_CODE_CREATED) {
                log("Keen POST SUCCESS!!");
        } else {
                log("Keen POST ERROR!!");
        }

        http.end();
}

