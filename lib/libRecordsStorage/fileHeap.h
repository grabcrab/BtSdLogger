#include "Arduino.h"

///////////////////////
enum tStorageResult
{
    fhOK = 0,
    fhStorageIsEmpty,
    fhHeaderSeekError,
    fhHeaderOpenError,
    fhHeaderReadError,  
    fhHeaderCrcError,
    fhFileTooSmall,
    fhReadBufferTooSmall,
    fhReadRecordSeekError,
    fhRecordReadError,
    fhRecordCrcError,
    fhTruncAddBytesError,
    fhFileHeapTruncateSizeError,
    fhTruncateError,
    fhRecordAppendError,
    fhBufferCreateError,
    fhInitError,
    fhOpenError,
    fhQueueBusy,
    fhNoSpiffsFreeSpace,
    fhMemCacheError,
    fhLowMemory,
    fhUnknown
};
///////////////////////
struct tHeapFileRecHeader
{
    uint16_t    recSize = 0;
    uint32_t    recordID = 0;
    uint8_t     fCrc8 = 0;
    bool        flag = false;    
    byte        reserved[9];// 9 bytes reserved for the further usage
    uint8_t     hCrc8 = 0;

    tHeapFileRecHeader();
    void print(const char *str);
    void printBytes(const char *str);
};
///////////////////////
bool    fileHeapOpen(String fName);
bool    fileHeapClose(String fName);
bool    fileHeapDrop(String fName);
size_t  fileHeapGetFileSize(const String fName);
String  fileHeapResultToString(tStorageResult errCode);
bool    fileHeapTruncate(String fName, uint16_t truncSize,  tStorageResult &errCode);
bool    fileHeapReadHeader(File f, const String fName, tHeapFileRecHeader *fHeader, size_t fSize, tStorageResult &errCode);
bool    fileHeapReadRecord(String fName, byte *dataRec, uint16_t &recSize, uint32_t &recID, tStorageResult &errCode);
bool    fileHeapWriteRecord(String fName, byte *dataRec, const uint16_t recSize, const uint32_t recID, tStorageResult &errCode);

///////////////////////
