#ifndef JSON_H
#define JSON_H

#include "Walvola.h"

#define KV_SIZE            32
#define JSON_KVS           16
#define JSON_MEX_MAX_SIZE  256

//forward declaration
int mqtt2JSONkvs(String mex);
String getJSONvalue(char * key);

#endif
