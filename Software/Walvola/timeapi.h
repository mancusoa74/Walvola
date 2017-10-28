#ifndef TIMEAPI_H
#define TIMEAPI_H

#include "Walvola.h"

#define TIMEAPI_RETRY 3 //I am using this service http://www.timeapi.or however is now very reliable so using a retry mechanism in crease realiability

//forward declaration
String get_web_time();
int get_web_time_hours(String timestamp);
int get_web_time_minutes(String timestamp);
int get_web_time_seconds(String timestamp);

#endif

