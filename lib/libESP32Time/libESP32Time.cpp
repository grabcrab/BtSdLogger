#include "libESP32Time.h"
#include "time.h"
#include <sys/time.h>


ESP32Time esp32Time_;
ESP32Time *esp32Time = &esp32Time_;

struct tm esp32Timeinfo;
////////////////////////////////
void esp32TimeInit(void)
{

}
////////////////////////////////
void esp32TimeSet(time_t epochSeconds)
{
    esp32TimeInit();
    esp32Time->setTime(epochSeconds);
}
////////////////////////////////
void esp32TimeSet(int sc, int mn, int hr, int dy, int mt, int yr, int ms)
{
    esp32TimeInit();    
    esp32Time->setTime(sc, mn, hr, dy, mt, yr, ms);
}
////////////////////////////////
void esp32TimeSetTimeStruct(tm t)
{
    esp32TimeInit();
    esp32Time->setTimeStruct(t);
}
////////////////////////////////
tm  esp32TimeGetTimeStruct()
{
    esp32TimeInit();
    esp32Timeinfo = esp32Time->getTimeStruct();
    return esp32Timeinfo;
}
////////////////////////////////
time_t  esp32TimeGetEpochSeconds(void)
{           
    esp32TimeInit();      
    time_t res = esp32Time->getEpoch();    

    if (res < 10000)
        {
            esp32TimeSet(DEFAULT_TIMESTAMP);     
        }     
    res = esp32Time->getEpoch();    
    return  res;
}
////////////////////////////////
void esp32TimeGet(int &sc, int &mn, int &hr, int &dy, int &mt, int &yr)
{
    esp32TimeInit();    
    sc = esp32Time->getSecond();
    mn = esp32Time->getMinute();
    hr = esp32Time->getHour(true);
    dy = esp32Time->getDay();
    mt = esp32Time->getMonth()+1; 
    yr = esp32Time->getYear();       
}

////////////////////////////////
String etFNum(int iN, byte idLen = 2)
{
  String sN = String(iN);
  while(sN.length()<idLen)
    sN = String("0") + sN;

  return sN;
}
////////////////////////////////
void esp32TimePrint(void)
{
  int sc; int mn; int hr; int dy; int mt; int yr;
  
  esp32TimeGet(sc, mn, hr, dy, mt, yr);

  Serial.print(">>>>>[BT-LOG] ESP32 date/time: "); Serial.print(yr); Serial.print(".");  Serial.print(etFNum(mt)); Serial.print("."); Serial.print(etFNum(dy));

  Serial.print("   ");

  Serial.print(etFNum(hr)); Serial.print(":");  Serial.print(etFNum(mn)); Serial.print(":"); Serial.println(etFNum(sc));
}
//////////////////////////////
