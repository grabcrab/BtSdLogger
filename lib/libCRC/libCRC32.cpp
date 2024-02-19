#include "libCRC32.h"
//////////////////////////////////////////////////////////////
uint32_t calcCrc32(File &file, uint32_t &charcnt)
{
    Serial.print(String("calcCrc32 started. File size: ") +  file.size() + " ");
    uint32_t val, crc;
    uint8_t i;
    uint32_t m1 = millis(); 
    uint32_t startMillis =  millis();
    const unsigned long crcTO = 30000;
    charcnt = 0;
    
      
    crc = 0xFFFFFFFF;

    byte *buf = new byte[CRC32_FILE_BUF_SIZE];
    
      
     while (file.available())
      {
        uint8_t c;
        
        uint16_t bytesRead = file.read(buf, CRC32_FILE_BUF_SIZE);  

        for (int j = 0; j < bytesRead; j++)
         {
            c = buf[j];
            val=(crc^c)&0xFF;
            for(i=0; i<8; i++) val = val & 1 ? (val>>1)^CRC32_POLY : val>>1;
            crc = val^crc>>8;  
            charcnt++;
         }        
         delay(10);   
         if (millis() - startMillis > crcTO)
          {
            Serial.println("File reading timeout.");
            break;
          }
         if (bytesRead) startMillis =  millis();
      }

      delete buf;
      Serial.println(String("finished in ") + (millis()-m1) + " ms");
 
      return crc^0xFFFFFFFF;
}
//////////////////////////////////////////////////////////////
uint32_t calcCrc32(String fName, uint32_t &charcnt)
{
   File file = SPIFFS.open(fName, "r");
   
      if (!file)
       {
          Serial.println(String("calcCrc32: error opening file: ") + fName);  
          return 0;
       }
    uint32_t res = calcCrc32(file, charcnt); 
    file.close();
    return res;
}
//////////////////////////////////////////////////////////////
uint32_t calcCrc32Buf(byte *buf, uint16_t len)
{
    uint32_t val, crc;
    uint8_t i;

    crc = 0xFFFFFFFF;
    while(len--){
        val=(crc^*buf++)&0xFF;
        for(i=0; i<8; i++){
            val = val & 1 ? (val>>1)^0xEDB88320 : val>>1;
        }
        crc = val^crc>>8;
    }
    return crc^0xFFFFFFFF;
}
//////////////////////////////////////////////////////////////