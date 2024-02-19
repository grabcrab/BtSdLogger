#include "recordsStorage.h"


///////////////////////////////////////////
tRecordsStorage::tRecordsStorage(String sName, tStorageType sT)
{
    storageName = sName;    
    setFileName();
    storageType = sT;
    switch(storageType)
    {
        case stUrgent:
            minHeapLevel = RECORDS_STORAGE_MIN_URG_HEAP;
        break;
        case stRegular:
            minHeapLevel = RECORDS_STORAGE_MIN_REG_HEAP;
        break;
    }
    open();
}
///////////////////////////////////////////
tRecordsStorage::~tRecordsStorage()
{
    close();
}
///////////////////////////////////////////
bool tRecordsStorage::checkSpiffs(void)
{  
  return ((float) SPIFFS.usedBytes() < RECORDS_STORAGE_MAX_SPIFFS_COEFF * (float) SPIFFS.totalBytes());
}
///////////////////////////////////////////
bool tRecordsStorage::drop(void)
{  
    SPIFFS.end();
    delay(10);
    SPIFFS.begin();
    return fileHeapDrop(fName); 
}
///////////////////////////////////////////
void tRecordsStorage::setFileName(void)
{
  fName = String(RECORDS_STORAGE_FILE_PREFIX) + storageName + String(RECORDS_STORAGE_FILE_EXT);
}
///////////////////////////////////////////
bool tRecordsStorage::open(void)
{
    if (dbOpened)
      return true;    

    dbOpened = fileHeapOpen(fName);
    return dbOpened;
}
///////////////////////////////////////////
bool tRecordsStorage::close(void)
{  
    dbOpened = false;    
    return fileHeapClose(fName);    
}
///////////////////////////////////////////
tStorageResult tRecordsStorage::addTextRecord(String message)
{      
    tStorageResult resCode;
    uint32_t timeStamp = getTimestamp();

    if (!checkSpiffs()) return fhNoSpiffsFreeSpace;

    if (!open()) 
    {        
        return fhOpenError;
    }
  
    bool wrRes = fileHeapWriteRecord(fName, (byte*)message.c_str(), message.length()+1, timeStamp, resCode);

    return resCode; 
}
///////////////////////////////////////////
tStorageResult tRecordsStorage::getTextRecord(String &message, uint32_t &timeStamp)
{
    if (!open()) 
    {    
       return fhOpenError;
    } 

    uint16_t bufSize = RECORDS_STORAGE_MAX_TEXT_SIZE; 
    byte *buf = new byte[bufSize];

    if (buf == NULL) 
    {
        Serial.printf("tRecordsStorage::getTextRecord: WARNING!!! Low RAM memory (%d/%d)\r\n", bufSize, ESP.getMaxAllocHeap());
        bufSize = ESP.getMaxAllocHeap(); 
        buf = new byte[bufSize];
    }

    tStorageResult resCode = getBinRecordSpiffs(buf, bufSize, timeStamp);
    if (resCode != fhOK)
    {
        buf[bufSize] = 0;
        message = (char*)buf;      
    } 
    else message = "";

    delete buf;
    return resCode;
}
///////////////////////////////////////////
tStorageResult tRecordsStorage::addBinRecordSpiffs(byte *buf, uint16_t bufSize)
{
    tStorageResult resCode;
    uint32_t timeStamp = getTimestamp();
    if (!checkSpiffs()) return fhNoSpiffsFreeSpace;

    if (!open()) 
    {
        return fhOpenError;
    } 
      
    bool wrRes = fileHeapWriteRecord(fName, buf, bufSize, timeStamp, resCode);

    if (!wrRes) 
        restore();

    return resCode;
}
///////////////////////////////////////////
tStorageResult tRecordsStorage::getBinRecordSpiffs(byte *buf, uint16_t &bufSize, uint32_t &timeStamp)
{  
    tStorageResult resCode;
    unsigned long startMillis = millis();
    if (!open()) 
    {    
       return fhOpenError;
    } 

    bool rdRes = fileHeapReadRecord(fName, buf, bufSize, timeStamp, resCode);

    if (!rdRes) 
        if (resCode != fhStorageIsEmpty)
            restore();

    return resCode;
}
///////////////////////////////////////////
size_t tRecordsStorage::fileSize(void)
{
    return fileHeapGetFileSize(fName);
}
///////////////////////////////////////////
tStorageResult tRecordsStorage::addBinRecord(byte *buf, uint16_t bufSize)
{  
    switch(writeMethod)
    {
        case swmDirect:   return addBinRecordSpiffs(buf, bufSize);        
        case swmCached:      
        case swmMemOnly:  return addBinRecordMem(buf, bufSize);
    }
    return fhUnknown;
}
///////////////////////////////////////////
tStorageResult tRecordsStorage::getBinRecord(byte *buf, uint16_t &bufSize, uint32_t &timeStamp)   
{
    timeStamp = 0;
    tStorageResult res = getBinRecordMem(buf, bufSize);
    if (res != fhStorageIsEmpty)
        return res;

    return  getBinRecordSpiffs(buf, bufSize, timeStamp);  
}
///////////////////////////////////////////
void tRecordsStorage::restore(void)
{
    Serial.printf("tRecordsStorage::restore: restoring the <%s> storage after fail...\r\n", storageName.c_str());
    drop();
    Serial.println("tRecordsStorage::restore: restoring completed");
}
///////////////////////////////////////////
tStorageResult tRecordsStorage::maintain(uint16_t &recCount, SemaphoreHandle_t mutex)
{
    const byte maxRecCount = RECORDS_STORAGE_MAX_MEM_REC_COUNT;
    recCount = 0;
    tStorageResult res = fhOK;

    if (writeMethod != swmCached) return fhOK;

    byte *buf = new byte[RECORDS_STORAGE_MAX_BUF_SIZE];
    uint16_t bufSize;

    for (int i = 0; i < maxRecCount; i++)
    {     
        bufSize = RECORDS_STORAGE_MAX_BUF_SIZE;
        if (!mutexTake(mutex)) continue;
        res = getBinRecordMem(buf, bufSize);
        mutexRelease(mutex);
        if (res == fhStorageIsEmpty)
        {
            res = fhOK;
            break;  
        }
        
        res = addBinRecordSpiffs(buf, bufSize);
        delay(10);
        if (res != fhOK) break;
        recCount++;      
    }

    delete buf; 
    return res;
}
///////////////////////////////////////////
bool tRecordsStorage::checkHeap(uint16_t nSize)
{
    size_t freeHeap = ESP.getFreeHeap() - nSize;
    size_t maxHeap = ESP.getMaxAllocHeap();
    if (freeHeap <  minHeapLevel) return false;
    if (nSize > maxHeap) return false;
    return true;
}
///////////////////////////////////////////
tStorageResult tRecordsStorage::addBinRecordMem(byte *buf, uint16_t bufSize)
{
    if (!checkHeap(bufSize)) return fhLowMemory;
  
    if (memCounter > RECORDS_STORAGE_MAX_MEM_REC_COUNT - 1)
        return fhLowMemory;

    memCache[memCounter].buf = new byte[bufSize];
    memcpy(memCache[memCounter].buf, buf, bufSize);
    memCache[memCounter].bufSize =  bufSize;

    memCounter++;
  
    return fhOK; 
}
///////////////////////////////////////////
tStorageResult tRecordsStorage::getBinRecordMem(byte *buf, uint16_t &bufSize)
{  
    tStorageResult res = fhOK;
    if (!memCounter)   
        return fhStorageIsEmpty;
  
    memCounter--;

    if (bufSize > memCache[memCounter].bufSize)
        bufSize = memCache[memCounter].bufSize;
    else 
        res = fhReadBufferTooSmall;

    memcpy(buf, memCache[memCounter].buf, bufSize);  
  
    delete memCache[memCounter].buf;

    return res; 
}