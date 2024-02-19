#ifndef __LIB_ESP32_TIME__
#define __LIB_ESP32_TIME__

#include <Arduino.h>
#include <time.h>
#include <ESP32Time.h>

#define DEFAULT_TIMESTAMP 	1577829600 //is set if time hasn't been syncronized 

void esp32TimeSet(time_t epochSeconds);
void esp32TimeSet(int sc, int mn, int hr, int dy, int mt, int yr, int ms = 0);
void esp32TimeSetTimeStruct(tm t);

tm   esp32TimeGetTimeStruct();
time_t   esp32TimeGetEpochSeconds(void);
void     esp32TimeGet(int &sc, int &mn, int &hr, int &dy, int &mt, int &yr);

void esp32TimePrint(void);


#endif //__LIB_ESP32_TIME__