#include "recordsStorage.h"


tRecordsStorage *regularStorage = NULL;
tRecordsStorage *urgentStorage  = NULL;

SemaphoreHandle_t regularMutex;
SemaphoreHandle_t urgentMutex;

///////////////////////////////////////////
bool mutexTake(SemaphoreHandle_t mutex, uint32_t timeoutMs)
{
  unsigned long  startMs = millis();

  while (millis() - startMs < timeoutMs) 
    {
        if (xSemaphoreTake(mutex, ( TickType_t ) 10 ) == pdTRUE )
            {
                    return true;
            }
    }
   return false;
}
///////////////////////////////////////////
void mutexRelease(SemaphoreHandle_t mutex)
{
    xSemaphoreGive(mutex);
}
///////////////////////////////////////////
#define TAKE_REGULAR    if (!mutexTake(regularMutex)) return fhQueueBusy
#define TAKE_URGENT     if (!mutexTake(urgentMutex)) return fhQueueBusy
#define RELEASE_REGULAR mutexRelease(regularMutex)
#define RELEASE_URGENT  mutexRelease(urgentMutex)
static bool initStorages(void)
{
    if ((regularStorage != NULL)&&(urgentStorage  != NULL)) return true;

    if (regularStorage == NULL)
    {
        regularStorage = new tRecordsStorage("regular", stRegular);
        if (regularStorage != NULL)
        {
            regularStorage->writeMethod = swmCached;
        }
    }

    if (urgentStorage == NULL)
    {
        urgentStorage = new tRecordsStorage("urgent", stUrgent);
        if (urgentStorage != NULL)
        {
            urgentStorage->writeMethod = swmCached;
        }
    }
    regularMutex    = xSemaphoreCreateMutex();    
    urgentMutex     = xSemaphoreCreateMutex();    
    if ((regularStorage != NULL)&&(urgentStorage  != NULL)) return true;
    ets_printf("initStorages: ERROR!!! Can't create objects.");
    return false;
}
#define STORAGES_INIT if (!initStorages()) return fhInitError
///////////////////////////////////////////
tStorageResult addBinRecordRegular(byte *buf, uint16_t bufSize)
{
    STORAGES_INIT;    
    tStorageResult sRes;
    TAKE_REGULAR;
    sRes = regularStorage->addBinRecordMem(buf, bufSize);
    RELEASE_REGULAR;
    return sRes;
}
///////////////////////////////////////////
tStorageResult getBinRecordRegular(byte *buf, uint16_t &bufSize)
{
    STORAGES_INIT;    
    tStorageResult sRes;
    TAKE_REGULAR;
    sRes = regularStorage->getBinRecordMem(buf, bufSize);
    RELEASE_REGULAR;
    if (sRes != fhStorageIsEmpty) return sRes;
    uint32_t timeStamp;
    sRes = regularStorage->getBinRecordSpiffs(buf, bufSize, timeStamp);
    return sRes;
}   
///////////////////////////////////////////
tStorageResult maintainRegular(uint16_t &recCount)
 {
    STORAGES_INIT;    
    return regularStorage->maintain(recCount, regularMutex);
 }
///////////////////////////////////////////
size_t getFileSizeRegular(void)
{
    STORAGES_INIT;
    return regularStorage->fileSize();
}
///////////////////////////////////////////
tStorageResult addBinRecordUrgent(byte *buf, uint16_t bufSize)
{
    STORAGES_INIT;    
    tStorageResult sRes;
    TAKE_URGENT;
    sRes = urgentStorage->addBinRecordMem(buf, bufSize);
    RELEASE_URGENT;
    return sRes;
}
///////////////////////////////////////////
tStorageResult getBinRecordUrgent(byte *buf, uint16_t &bufSize)
{
    STORAGES_INIT;    
    tStorageResult sRes;
    TAKE_URGENT;
    sRes = urgentStorage->getBinRecordMem(buf, bufSize);
    RELEASE_URGENT;
    if (sRes != fhStorageIsEmpty) return sRes;
    uint32_t timeStamp;
    sRes = urgentStorage->getBinRecordSpiffs(buf, bufSize, timeStamp);
    return sRes;
}   
///////////////////////////////////////////
tStorageResult maintainUrgent(uint16_t &recCount)
 {
    STORAGES_INIT;    
    return urgentStorage->maintain(recCount, urgentMutex);     
 }
///////////////////////////////////////////
size_t getFileSizeUrgent(void)
{
    STORAGES_INIT;
    return urgentStorage->fileSize();
}
///////////////////////////////////////////