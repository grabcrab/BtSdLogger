#include "libUtils.h"

#include <SPIFFS.h>
#include <FS.h>
#include <Wire.h>

///////////////////////////////////////
#define base16char(i) ("0123456789ABCDEF"[i])
///////////////////////////////////////
String int64String(uint64_t value, uint8_t base, bool prefix, bool sign) 
{ 
    if (base < 2)
        base = 2;
    else if (base > 16)
        base = 16;

  
  uint8_t i = 64;
  
  char buffer[66] = {0};

  if (value == 0)
    buffer[i--] = '0';
  else {
    uint8_t base_multiplied = 3;
    uint16_t multiplier = base * base * base;

    if (base < 16) {
      multiplier *= base;
      base_multiplied++;
    }
    if (base < 8) {
      multiplier *= base;
      base_multiplied++;
    }


    while (value > multiplier) {
      uint64_t q = value / multiplier;
      // get remainder without doing another division with %
      uint16_t r = value - q * multiplier;

      for (uint8_t j = 0; j < base_multiplied; j++) {
        uint16_t rq = r / base;
        buffer[i--] = base16char(r - rq * base);
        r = rq;
      }

      value = q;
    }

    uint16_t remaining = value;
    while (remaining > 0) {
      uint16_t q = remaining / base;
      buffer[i--] = base16char(remaining - q * base);
      remaining = q;
    }
  }
 
  if (base == DEC && sign)
    buffer[i--] = '-';
  else if (prefix) {
    if (base == HEX) {
      // 0x prefix
      buffer[i--] = 'x';
      buffer[i--] = '0';
    }
    else if (base == OCT)
      buffer[i--] = '0';
    else if (base == BIN)
      buffer[i--] = 'B';
  }

  return String(&buffer[i + 1]);
}

///////////////////////////////////////////////////////////////////
String int64String(int64_t value, uint8_t base, bool prefix) 
{
  // Signed numbers only make sense for decimal numbers
  bool sign = base == DEC && value < 0;
  
  uint64_t uvalue = sign ? -value : value;

  // call the unsigned function to format the number
  return int64String(uvalue, base, prefix, sign);
}
/////////////////////////////////////////////////
String utilsUint64ToString(uint64_t input) 
{
    return  int64String(input, DEC, false);  
}
///////////////////////////////////////
String utilsUint64ToHexString(uint64_t input)
{ 
  return  int64String(input, HEX, false);  
}
///////////////////////////////////////
uint64_t utilsGetDeviceID64(void)
{  
  return ESP.getEfuseMac();
}
///////////////////////////////////////
String utilsGetDeviceID64Hex(void)
{
  return utilsUint64ToHexString(utilsGetDeviceID64());
}
///////////////////////////////////////
String utilsParseParamValue(const String str, String paramName)
{
    paramName.toLowerCase();
    
    int startIndex = str.indexOf(paramName);
    if (startIndex == -1) return "";

    String res = str.substring(startIndex);
    startIndex = res.indexOf(":");
    if (startIndex == -1) return "";
    startIndex++;

    res = res.substring(startIndex);
    res.replace("\n","");
    res.replace("\r","");
    res.replace("\"","");
    res.replace(" ","");

    int endIndex = res.indexOf(",");
    if (endIndex == -1)
        endIndex = res.indexOf("}");
    if (endIndex == -1)  
        return "";
    return res.substring(0, endIndex);
}
///////////////////////////////////////
void utilsPrintFile(String fName)
{
    File file = SPIFFS.open(fName, "r");
    size_t bRead = 0;  
    if (!file) 
    {
        Serial.println("utilsPrintFile: file open error");
        return;
    }

    char c;
    Serial.println("");
    Serial.printf("-----------------------< %s >--------------------------\n", fName.c_str());
    
    while(file.read((byte*)&c, 1))
    {
        bRead++;
        Serial.print(c);
    }
          
    Serial.println("");
    Serial.printf("----------------------< %s >< %d bytes> --------------------------\n", fName.c_str(),bRead );
    file.close();
}
///////////////////////////////////////
void utilsPrintBuf(byte *buf, uint16_t size, bool dec)
{
    Serial.println("");
    Serial.println("--------------------------------------------------");
    if (!dec) for (int i = 0; i < size; i++) Serial.print((char)buf[i]);

    if (dec) for (int i = 0; i < size; i++) {Serial.print(String(buf[i])); Serial.print(" ");}
    Serial.println("");
    Serial.println("--------------------------------------------------");
}
///////////////////////////////////////
bool utilsAppendFile(String dstFileName, String srcFileName)
{
    const uint16_t fileBlockSize = 1000;
    byte buf[fileBlockSize];
    uint16_t       bytesRead = 0;

    File srcFile = SPIFFS.open(srcFileName, "r");
    if (!srcFile) 
    {
        Serial.println("utilsAppendFile: source file open error");
        return false;
    }

    File dstFile = SPIFFS.open(dstFileName, "a");
    if (!dstFile) 
    {
        Serial.println("utilsAppendFile: destination file open error");
        srcFile.close();
        return false;
    }

    do
    {
        bytesRead = srcFile.read(buf, fileBlockSize);
        dstFile.write(buf, bytesRead);
    } 
    while (bytesRead);
  
    srcFile.close();
    dstFile.flush(); 
    dstFile.close();

    SPIFFS.remove(srcFileName);

    return true;
}
///////////////////////////////////////
bool utilsAppendFileBuf(String dstFileName, byte *buf, uint32_t bufSize)
{  
    File dstFile = SPIFFS.open(dstFileName, "a");
    if (!dstFile) 
    {
        Serial.println("utilsAppendFileBuf: destination file open error: " + dstFileName);
        return false;
    }
  
    size_t blockSize = 1000;
    size_t totalBytes = 0;   
    size_t bytesLeft = bufSize;

    unsigned long startedMs = millis();

    while(bytesLeft)  
    {    
        if (bytesLeft < blockSize) blockSize = bytesLeft;  
        size_t bytesWritten = dstFile.write(buf, blockSize);  

        if (bytesWritten != blockSize)
        {
            if (millis() - startedMs > 15000) 
            {
            Serial.printf("\nutilsAppendFileBuf: error writing [%s] file [1], %d of %d bytes written\n", dstFile.name(), totalBytes, bufSize);       
            dstFile.close();             
            return false;
            }
        } 
        else startedMs = millis();

        bytesLeft -= bytesWritten;
        buf += bytesWritten;
        totalBytes += bytesWritten;
        delay(10);
   }
  
    dstFile.flush(); 
    dstFile.close();  

    if (totalBytes != bufSize)
    {     
        Serial.printf("\nutilsAppendFileBuf: error writing [%s] file [2], %d of %d bytes written\n", dstFile.name(), totalBytes, bufSize);       
        return false;
    }

    return true;
}
/////////////////////
void utilsSPIFFSDir(void)
{
    if (!SPIFFS.begin(true)) 
    {
        Serial.println(" utilsSPIFFSDir: An Error has occurred while mounting SPIFFS");
        return;
    }
    Serial.println();
    Serial.println("-------------------------");
    delay(2);
    File root = SPIFFS.open("/");
 
    File file = root.openNextFile();
    uint16_t fileNum = 0;
    while(file)
    {
        Serial.print("FILE: ");
        Serial.print(file.name());
        Serial.print("\t");
        Serial.println(file.size());
        file = root.openNextFile();
        if (fileNum++>32000) break;
    }
    Serial.println("-------------------------");
}
//////////////////////////////////////  
bool utilsAppendFileBuf_old(String dstFileName, byte *buf, uint32_t bufSize)
{ 
    File dstFile = SPIFFS.open(dstFileName, "a");
    if (!dstFile) 
    {
        Serial.println("utilsAppendFileBuf: destination file open error: " + dstFileName);
        return false;
    }
    uint32_t bytesWritten = 0;
    unsigned long ms = millis();
    do
    {
        size_t s = dstFile.write(buf+bytesWritten, bufSize-bytesWritten);
        bytesWritten += s;
        if (s) ms = millis();
        if (millis()-ms>5000) break;
        delay(100);
    }
    while (bytesWritten<bufSize);

    dstFile.close();  

    if (bytesWritten != bufSize)
    {
        Serial.println("utilsAppendFileBuf: file write error, " + String(bytesWritten) + " from " + String(bufSize));
        return false;
    }

  return true;
}
///////////////////////////////////////

void utilsGetFsStat(size_t &bytesUsed, size_t &bytesTotal)
{	
    bytesUsed = SPIFFS.usedBytes();
    bytesTotal = SPIFFS.totalBytes();    
}
///////////////////////////////////////
void utilsPrintFsStat(void)
{
    size_t bytesUsed; 
    size_t bytesTotal;
    utilsGetFsStat(bytesUsed, bytesTotal);

    Serial.println("File system status: " + String(bytesUsed) + " bytes used of " + String(bytesTotal));
}
///////////////////////////////////////
String utilsMacToString(byte mac[6])
{
    String s;
    for (byte i = 0; i < 6; ++i)
    {
        char buf[3];
        sprintf(buf, "%02X", mac[i]);
        s += buf;
        if (i < 5) s += ':';
    }
    return s;
}
///////////////////////////////////////
String utilsEpochToString(time_t epoch)
{
    struct tm tm; 
    char buf[80];
    localtime_r(&epoch, &tm);
    sprintf(buf, "%2d.%02d.%02d %02d:%02d:%02d", tm.tm_year-100, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return String(buf);  
}

uint64_t utilsMACArrayToUint64(uint8_t *buffer) 
{
    uint64_t value ;

    value = buffer[5] ;
    value = (value << 8 ) + buffer[4] ;
    value = (value << 8 ) + buffer[3] ;
    value = (value << 8 ) + buffer[2] ;
    value = (value << 8 ) + buffer[1] ;
    value = (value << 8 ) + buffer[0] ;

    return value ;
}
