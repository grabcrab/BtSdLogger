#include "recordsStorage.h"
///////////////////////////////////////////
uint32_t defGetTimestamp(void)
{
    return millis();
}
///////////////////////////////////////////
static void setBuffer(byte *buf, uint16_t sz)
{
    char cnt = 'a';
    for (auto i = 0; i < sz; i++)
    {
        buf[i] = (byte) cnt;
        cnt++;
        if (cnt == 'z')
        cnt = 'a';
    }
    buf[sz-1] = 0;
    buf[sz-2] = '0';
    buf[sz-3] = '0';
    buf[sz-4] = '0';
}
///////////////////////////////////////////
void recWriterTestTask(void*)
{
    uint32_t cntr = 0;
    while(1)
    {
        String uStr = "TASK_URGENT_" +String(cntr);
        addBinRecordUrgent((byte*)uStr.c_str(), uStr.length() + 1);
        String rStr = "TASK_REGULAR_" +String(cntr);
        addBinRecordRegular((byte*)rStr.c_str(), rStr.length() + 1);
        delay(10000);
        cntr++;
    }
}
///////////////////////////////////////////
bool testRecordsMaintain(uint16_t cntr)
{
    if (cntr%10) return true;

    bool res = true;
    uint16_t mCount;
    unsigned long ms = millis();
    tStorageResult mRes;
    mRes = maintainUrgent(mCount);  
    long msRes = millis() - ms;
    
    Serial.printf("URGENT: MAINTAINED (recCount = %d, mRes = %s, ms = %d)\r\n", mCount, fileHeapResultToString(mRes).c_str(), msRes);   
    if (mRes != fhOK) 
    {
    Serial.println("URGENT Maintain ERROR!!!");
    res = false;
    }
    
    ms = millis();  
    mRes = maintainRegular(mCount);   
    msRes = millis() - ms;
    
    Serial.printf("REGULAR: MAINTAINED (recCount = %d, mRes = %s, ms = %d)\r\n", mCount, fileHeapResultToString(mRes).c_str(), msRes);   
    if (mRes != fhOK) 
    {
    Serial.println("REGULAR Maintain ERROR!!!");
    res = false;
    }
 
    return res;
}
///////////////////////////////////////////
void testRecordsStorageAddRecords(uint16_t recCount,  tStorageType sType)
{
    byte buf[3000];
    
    tStorageResult resCode;
    uint16_t rSize;
    uint16_t i = 0; 
    uint16_t mCounter = 0;
    
    size_t initHeap = ESP.getFreeHeap();
    size_t fileSize;

    String testName;
    uint16_t maintainPeriod = 10;
    switch(sType)
    {
        case stUrgent: testName = "URGENT"; break;
        case stRegular: testName = "REGULAR"; break;
    }
    xTaskCreatePinnedToCore(recWriterTestTask, "recWriterTestTask", 3000, NULL, 7,NULL,  APP_CPU_NUM); 
    while(1)
    {
        uint32_t timeStamp = i;
        
        rSize = random(50, 100);

        setBuffer(buf, rSize);
        long ms = millis();
        switch(sType)
        {
            case stUrgent:  resCode = addBinRecordUrgent(buf, rSize);  break;
            case stRegular: resCode = addBinRecordRegular(buf, rSize);  break;
        }
 
        switch(sType)
        {
            case stUrgent: fileSize = getFileSizeUrgent(); break;
            case stRegular: fileSize = getFileSizeRegular(); break;
        }

        delay(10);

        long msRes = millis() - ms;
        if (resCode == fhOK)
        {
        Serial.printf("%s WRITE ###%d,\tfsize = %d,\trecSize = %d,\t%d ms,\t%d HEAP (%d)\t%d/%d SPIFFS\r\n", 
                      testName.c_str(), i, fileSize, rSize, msRes, ESP.getFreeHeap(), initHeap - ESP.getFreeHeap(), SPIFFS.usedBytes(), SPIFFS.totalBytes());
        }
        else 
        {
            if (resCode == fhNoSpiffsFreeSpace)
            Serial.printf("testRecordsStorageAddRecords: COMPLETED (%s)\r\n", fileHeapResultToString(resCode).c_str());                               
            else         
            Serial.printf("testRecordsStorageAddRecords: ERROR!!! %s\r\n", fileHeapResultToString(resCode).c_str());                     
            break;
        }
        i++;
        if (i>=recCount)
        {         
            mCounter = 30000; 
        }

        if (!testRecordsMaintain(i)) break;
        
        if (i>=recCount)
        {
            Serial.printf("testRecordsStorageAddRecords: COMPLETED (i = %d, recCount = %d)\r\n", i, recCount);   
            break;
        }  
    }
    Serial.println("------");
    switch(sType)
    {
        case stUrgent: fileSize = getFileSizeUrgent(); break;
        case stRegular: fileSize = getFileSizeRegular(); break;
    }
    Serial.println("=====================================");
    Serial.printf("AFTER WRITE: fsize = %d,\trecSize = %d,\t%d HEAP (%d)\t%d/%d SPIFFS(%.1f%)\r\n", 
         fileSize, rSize, ESP.getFreeHeap(), initHeap - ESP.getFreeHeap(), (SPIFFS.usedBytes()), SPIFFS.totalBytes(),
              ((float)(SPIFFS.usedBytes())))/((float)SPIFFS.totalBytes());  
    Serial.println("=====================================");
    testRecordsMaintain(0);
}
///////////////////////////////////////////
void testRecordsStorageReadRecords(tStorageType sType)
{
    byte buf[3000];   
    tStorageResult resCode;
    uint16_t rSize;
    int i = 0;
    size_t initHeap = ESP.getFreeHeap();
    size_t fileSize;

        String testName;
    switch(sType)
    {
        case stUrgent: testName = "URGENT"; break;
        case stRegular: testName = "REGULAR"; break;
    }
    
    while (true)
    {
        uint32_t timeStamp;
        rSize = 3000;
        long ms = millis();   

        if (!testRecordsMaintain(i)) break; 

        switch(sType)
        {
            case stUrgent:  resCode = getBinRecordUrgent(buf, rSize);  break;
            case stRegular: resCode = getBinRecordRegular(buf, rSize); break;
        }
     
        long msRes = millis() - ms;

        if (resCode == fhStorageIsEmpty)
        {
            Serial.printf("testRecordsStorageReadRecords: COMPLETED: %d records\r\n", i+1); 
            break;
        }
        
        switch(sType)
        {
            case stUrgent: fileSize = getFileSizeUrgent(); break;
            case stRegular: fileSize = getFileSizeRegular(); break;
        }
        
        if (resCode == fhOK)
        {        
            Serial.printf("<<%d>>", i); Serial.print("<<<");Serial.print((char*)buf); Serial.println(">>>");        
        } 
        else 
        {
            Serial.printf("READ ERROR!!! %s\r\n", fileHeapResultToString(resCode).c_str());   
            break;
        }
        i++;
    }

    Serial.println("=====================================");
    switch(sType)
    {
        case stUrgent: fileSize = getFileSizeUrgent(); break;
        case stRegular: fileSize = getFileSizeRegular(); break;
    }  
    
    Serial.printf("AFTER READ: fsize = %d,\trecSize = %d,\t%d HEAP (%d)\t%d/%d SPIFFS(%.1f%)\r\n", 
          fileSize, rSize, ESP.getFreeHeap(), initHeap - ESP.getFreeHeap(), (SPIFFS.usedBytes()), SPIFFS.totalBytes(),
              ((float)(SPIFFS.usedBytes())))/((float)SPIFFS.totalBytes());  
    Serial.println("=====================================");
}
