#include "CurrentTime.h"
#include <time.h>

int HH,MM,SS;

void getCurrentTime(void){
  time_t rawtime;
  time ( &rawtime );

  struct tm * timeinfo;
  timeinfo = localtime ( &rawtime );

  HH = timeinfo ->tm_hour;
  MM = timeinfo ->tm_min;
  SS = timeinfo ->tm_sec;
}

int getHours(void){
    getCurrentTime();
    return HH;
}

int getMins(void){
    return MM;
}

int getSecs(void){
    return SS;
}
