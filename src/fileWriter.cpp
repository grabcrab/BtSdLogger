#include "fileWriter.h"
#include "mySD.h"
#include "libESP32Time.h"

#define MAX_PORTS 5

String fileNames[MAX_PORTS] = {""};
String prnStrings[MAX_PORTS]  = {""};
//////////////
bool initSD(void)
{
    Serial.println(">>>>>[BT-LOG] SD card init");
    pinMode(SD_CS,OUTPUT);
    digitalWrite(SD_CS, HIGH); 
    delay(20);
    digitalWrite(SD_CS, LOW); 
    SD.end();
    if (!SD.begin( SD_CS, SD_MOSI, SD_MISO, SD_CLK )) 
    {
        Serial.println(">>>>>[BT-LOG] SD card initialization failed!");
        return false;
    } 
    else 
    {
        Serial.println(">>>>>[BT-LOG] initialization done.");
        for (auto n = 0; n < MAX_PORTS; n++) prnStrings[n] = sdGetPrefix();            
    }    
    return true;
}
//////////////
bool sdAppendFile(const String fName, String txt)
{    
    ext::File f;
    f = SD.open(fName.c_str(), FILE_WRITE);
    if (!f) 
    {
        Serial.printf(">>>>>[BT-LOG] sdAppendFile ERROR: can't open <%s>\r\n", fName.c_str());
        return false;
    }    
    size_t bW = f.print(txt.c_str());
    f.flush();
    f.close();   
    
    if (bW) return true;
    return false;
}
//////////////
static String sdFileName = "";
String sdGetFileName(int num)
{
    if (sdFileName != "") return sdFileName;
    char buf[128];
    int year, month, day, hour, minute, second;
    esp32TimeGet(second, minute, hour, day, month, year);
    sprintf(buf, "%02d%02d%02d%02d.txt", month, day, hour, minute);
    sdFileName = buf;
    return sdFileName;
}
//////////////
String sdGetPrefix(void)
{
    char buf[128];
    int year, month, day, hour, minute, second;
    esp32TimeGet(second, minute, hour, day, month, year);
    sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d >", year, month, day, hour, minute, second); 
    return buf; 
}
//////////////
bool sdAppendFile(String txt, int num)
{
    if (fileNames[num] == "")
      fileNames[num] = sdGetFileName(num);     
    return sdAppendFile(fileNames[num], txt);  
}
//////////////
bool sdAddChar(char c, int num)
{
    if (c == '\r') return true;
    if (!c) return true;   
    prnStrings[num] += String(c);       
    if (c != '\n')      
        return true;
    bool res = sdAppendFile(prnStrings[num], num);
    prnStrings[num] = sdGetPrefix();
    return res;
}
//////////////
void sdWriteHeader(void)
{
    char buf[128];
    int year, month, day, hour, minute, second;
    esp32TimeGet(second, minute, hour, day, month, year);  
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d\r\n", year, month, day, hour, minute, second);
    sdAppendFile("<<<LOG file started>>>\n", 0);
    sdAppendFile(buf, 0);
    sdAppendFile("==============", 0);
}
//////////////
void sdFlush(void)
{
    sdAppendFile(prnStrings[0], 0);
}