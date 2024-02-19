#pragma once
#include "Arduino.h"
#include <FS.h>                                                                              
#include "SPIFFS.h"
#include "libUtils.h"
#include "libCRC8.h"
#include "fileHeap.h"
//#include "memoryBuffer.h"



#define RECORDS_STORAGE_DEFAULT_DB_NAME   "storage"
#define RECORDS_STORAGE_FILE_EXT          ".dat"
#define RECORDS_STORAGE_FILE_PREFIX       "/"
#define RECORDS_STORAGE_MAX_TEXT_SIZE     4096
#define RECORDS_STORAGE_MAX_SPIFFS_COEFF  (0.65)

#define RECORDS_STORAGE_MIN_URG_HEAP    10000
#define RECORDS_STORAGE_MIN_REG_HEAP    15000

#define RECORDS_STORAGE_MAX_BUF_SIZE        5000    
#define RECORDS_STORAGE_MAX_MEM_REC_COUNT   33
#define RECORDS_STORAGE_DEF_TAKE_TO_MS      100

uint32_t defGetTimestamp(void);
////////////////////
enum tStorageType
{
    stUrgent,
    stRegular
};
////////////////////
enum tRecStorageWriteMethod
{
   swmDirect,
   swmCached,
   swmMemOnly 
};
////////////////////
struct tRecBuffer
{   
    byte *buf;
    uint16_t bufSize;  
};
////////////////////
typedef uint32_t fGetTimestamp(void);
////////////////////
class tRecordsStorage
{
    String storageName =   RECORDS_STORAGE_DEFAULT_DB_NAME;
    bool   dbOpened    =  false;
    String fName;            
    String logFolderName = "";
    bool   addSerialToLogName = true;    
    size_t minHeapLevel     = RECORDS_STORAGE_MIN_REG_HEAP;
    tStorageType storageType = stRegular;    
    byte        memCounter = 0;
    tRecBuffer  memCache[RECORDS_STORAGE_MAX_MEM_REC_COUNT];

    bool checkSpiffs(void);    

    void restore(void);
    bool checkHeap(uint16_t nSize);
    
    public:
    tRecStorageWriteMethod writeMethod = swmCached;
    fGetTimestamp   *getTimestamp = defGetTimestamp;

    tRecordsStorage(String sName =  RECORDS_STORAGE_DEFAULT_DB_NAME, tStorageType sT = stRegular);
    ~tRecordsStorage();
    void setFileName(void);
    bool drop(void);
    bool open(void);
    bool close(void);
    tStorageResult addTextRecord(String message);
    tStorageResult getTextRecord(String &message, uint32_t &timeStamp);    

    tStorageResult addBinRecord(byte *buf, uint16_t bufSize);
    tStorageResult getBinRecord(byte *buf, uint16_t &bufSize, uint32_t &timeStamp);   

    tStorageResult addBinRecordSpiffs(byte *buf, uint16_t bufSize);
    tStorageResult getBinRecordSpiffs(byte *buf, uint16_t &bufSize, uint32_t &timeStamp); 

    tStorageResult addBinRecordMem(byte *buf, uint16_t bufSize);
    tStorageResult getBinRecordMem(byte *buf, uint16_t &bufSize);


    tStorageResult maintain(uint16_t &recCount, SemaphoreHandle_t mutex);

    
    size_t fileSize(void);
};


bool mutexTake(SemaphoreHandle_t mutex, uint32_t timeoutMs = RECORDS_STORAGE_DEF_TAKE_TO_MS);
void mutexRelease(SemaphoreHandle_t mutex);

tStorageResult addBinRecordRegular(byte *buf, uint16_t bufSize);
tStorageResult getBinRecordRegular(byte *buf, uint16_t &bufSize);
tStorageResult maintainRegular(uint16_t &recCount);
size_t getFileSizeRegular(void);
//uint8_t getMemCountRegular(void);

tStorageResult addBinRecordUrgent(byte *buf, uint16_t bufSize);
tStorageResult getBinRecordUrgent(byte *buf, uint16_t &bufSize);
tStorageResult maintainUrgent(uint16_t &recCount);
size_t getFileSizeUrgent(void);
//uint8_t getMemCountUrgent(void);

void testRecordsStorageAddRecords(uint16_t recCount = 0, tStorageType sType = stRegular);
void testRecordsStorageReadRecords(tStorageType sType = stRegular);