#include "json.h"

//key value struct
struct kv {
        char k[KV_SIZE];
        char v[KV_SIZE];
};

struct kv JSONkvs[JSON_KVS];
//char JSON_STRING[JSON_MEX_MAX_SIZE];

//parse an incoming json message from MQTT to json object
int mqtt2JSONkvs(String mex)
{
        char JSON_STRING[JSON_MEX_MAX_SIZE];
        jsmn_parser p;
        jsmntok_t t[JSON_KVS * 2];

        mex.replace("u'", "'"); //remove the u (unicode) prefix if present
        mex.replace("'", ""); //remove the ' if present
        mex.toCharArray(JSON_STRING, JSON_MEX_MAX_SIZE); //mqtt mex to JSON_STRING

        //clear the JSONkvs structure
        for (int k = 0; k < JSON_KVS; k++) {
                memset(JSONkvs[k].k, '\0', KV_SIZE);
                memset(JSONkvs[k].v, '\0', KV_SIZE);
        }

        //initialize the json library
        jsmn_init(&p);

        //parse the JSON_STRING(MQTT mex) into a json object
        int r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));
        if (r < 0) {
                log("Failed to parse JSON");
                return 1;
        }


        //fill in the key value struct from json object
        int kvidx = 0;
        for (int i = 1; i < r; i+=2) {
                if ((t[i].type == JSMN_STRING && t[i+1].type == JSMN_STRING) || (t[i].type == JSMN_PRIMITIVE && t[i+1].type == JSMN_PRIMITIVE)) {
                        memcpy(JSONkvs[kvidx].k, JSON_STRING + t[i].start, t[i].end-t[i].start);
                        memcpy(JSONkvs[kvidx].v, JSON_STRING + t[i+1].start, t[i+1].end-t[i+1].start);
                        kvidx++;
                }
        }
        return 1;
}

//retrive the value associated to the provided key; if not present return empty string
String getJSONvalue(char * key)
{
        int i;
        for (i = 0; i < JSON_KVS; i++) {
                if (strcmp(JSONkvs[i].k, key) == 0)
                        return JSONkvs[i].v;
        }
        return "";
}

