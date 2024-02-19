#include "fileHeap.h"

#include <FS.h>
#include <SPIFFS.h>
#include <unistd.h>
#include <esp_spiffs.h>

#include "libCRC8.h"
#include "libUtils.h"

///////////////////////
tHeapFileRecHeader::tHeapFileRecHeader()
{
    memset(this, 0, sizeof(tHeapFileRecHeader));
}
///////////////////////
void tHeapFileRecHeader::print(const char *str)
{
    Serial.printf(">>>>>[BT-LOG] %s recSize = %d, recordID = %d, fCrc8 = %d, flag = %d, hCrc8 = %d\r\n",
                str, recSize, recordID, fCrc8, flag, hCrc8);
}
///////////////////////
void tHeapFileRecHeader::printBytes(const char *str)
{
    byte *rhPtr = (byte*) this;
    Serial.print(str);
    for (int i = 0; i < sizeof(tHeapFileRecHeader); i++)
    {
        Serial.printf(" %d", rhPtr[i]);
    }
    Serial.println();
}
///////////////////////
size_t fileHeapGetFileSize(const String fName)
{
    size_t fileSize = 0;
    File f = SPIFFS.open(fName, "r");
    if (f)
    {
        fileSize = f.size();    
        f.close();
    }
    return fileSize;  
}
///////////////////////
String fileHeapResultToString(tStorageResult errCode)
{
    switch(errCode)
    {
        case fhOK:                  return "fhOK";
        case fhStorageIsEmpty:      return "fhStorageIsEmpty";    
        case fhHeaderSeekError:     return "fhHeaderSeekError";
        case fhHeaderReadError:     return "fhHeaderReadError";       
        case fhHeaderCrcError:      return "fhHeaderCrcError";
        case fhFileTooSmall:        return "fhFileTooSmall";
        case fhReadBufferTooSmall:  return "fhReadBufferTooSmall";
        case fhReadRecordSeekError: return "fhReadRecordSeekError";
        case fhRecordReadError:     return "fhRecordReadError";
        case fhRecordCrcError:      return "fhRecordCrcError";
        case fhTruncAddBytesError:  return "fhTruncAddBytesError";
        case fhFileHeapTruncateSizeError:  return "fhFileHeapTruncateSizeError";
        case fhTruncateError:       return "fhTruncateError";
        case fhRecordAppendError:   return "fhRecordAppendError";
        case fhBufferCreateError:   return "fhBufferCreateError";
        case fhInitError:           return "fhInitError";
        case fhQueueBusy:           return "fhQueueBusy";        
        case fhOpenError:           return "fhOpenError";
        case fhNoSpiffsFreeSpace:   return "fhNoSpiffsFreeSpace";
        case fhMemCacheError:       return "fhMemCacheError";    
        case fhLowMemory:           return "fhLowMemory";    
        case fhUnknown:             return "fhUnknown";
        
    }
    return "UNKNOWN_ERROR";
}
///////////////////////
bool fileHeapOpen(String fName)
{
    File f = SPIFFS.open(fName, "r");
    if (!f) 
     {
      f = SPIFFS.open(fName, "w");
      if (!f) return false;
     }
    f.close();
    return true;
}
///////////////////////
bool fileHeapClose(String fName)
{
    return true;
}
///////////////////////
bool fileHeapDrop(String fName)
{
    return SPIFFS.remove(fName);
}
///////////////////////
bool fileHeapTruncate(String fName, uint16_t truncSize,  tStorageResult &errCode)
{    
    errCode = fhOK;
    size_t initFileSize = fileHeapGetFileSize(fName);    

    if (!initFileSize)
    {
        errCode = fhStorageIsEmpty;
        return false;
    } 

    const uint16_t minTruncSize = 260;
    byte appBuf[minTruncSize];
    uint16_t rTruncSize = truncSize; 
    uint16_t sizeDelta = 0;
    if (truncSize < minTruncSize)
    {                       
        sizeDelta = minTruncSize - truncSize;
        if (!utilsAppendFileBuf(fName, appBuf, sizeDelta))
        {    
            errCode = fhTruncAddBytesError;
            return false;
        }
    }
    rTruncSize += sizeDelta;
    long newSize = /*fileHeapGetFileSize(fName)*/initFileSize + sizeDelta - rTruncSize;
    
    if (newSize < 0)  
    {            
        errCode = fhFileHeapTruncateSizeError;
        return false;
    }
    String sfName = "/spiffs" + fName;
    int trRes = truncate(sfName.c_str(), newSize);

    if (trRes) 
    { 
        String truncResS = "UNKNOWN";
        switch (errno) 
        {
            case EFBIG:     truncResS = "The length argument was greater than the maximum file size."; break;
            case EINTR:     truncResS = "A signal was caught during execution"; break;
            case EINVAL:    truncResS = "path does not refer to a regular file, or the length specified is incorrect."; break;
            case EIO:       truncResS = "An I/O error occurred while reading from or writing to a file system."; break;
            case EISDIR:    truncResS = "The file specified is a directory. The system cannot perform the requested function on a directory."; break;
            case EROFS:     truncResS = "The file resides on a read-only file system."; break;
        }
        Serial.printf("fileHeapTruncate: truncate() error (%d). Old file size: %d, new file size: %d. Err. message: %s \r\n", trRes, initFileSize, newSize, truncResS.c_str());
        errCode = fhTruncateError;
        return false;      
    }
    return true;
}
///////////////////////
bool fileHeapReadHeader(File f, const String fName, tHeapFileRecHeader *fHeader, size_t fSize, tStorageResult &errCode)
{
    errCode = fhOK;
    size_t seekPos = fSize - sizeof(tHeapFileRecHeader);
    //Serial.println(seekPos);
    if (!f.seek(seekPos))
    {
        Serial.printf("fileHeapReadHeader: file <%s> seek error.\r\n", fName.c_str());
        errCode = fhHeaderSeekError;
        f.close();
        return false;  
    } 
    size_t r = f.read((byte*)fHeader, sizeof(tHeapFileRecHeader));

    if (r!=sizeof(tHeapFileRecHeader))
    {
        Serial.printf("fileHeapReadHeader: file <%s> record header read error.\r\n", fName.c_str());
        errCode = fhHeaderReadError;
        f.close();
        return false;  
    }   
    
    uint8_t c8 = calcCrc8((byte*)fHeader, sizeof(tHeapFileRecHeader)-1);     

    if (c8 != fHeader->hCrc8)
    {
        Serial.printf("fileHeapReadHeader: file <%s> record header CRC error (%d/%d).\r\n", fName.c_str(), c8, fHeader->hCrc8);
        f.close();
        errCode = fhHeaderCrcError;
        return false;  
    }
    return true;
}
///////////////////////
bool fileHeapReadRecord(String fName, byte *dataRec, uint16_t &recSize, uint32_t &recID, tStorageResult &errCode)
{   
    if (!fileHeapGetFileSize(fName))
    {
      errCode = fhStorageIsEmpty;
      return false;
    } 

    errCode = fhOK;
    tHeapFileRecHeader fHeader;
    size_t fSize = fileHeapGetFileSize(fName);
    File f = SPIFFS.open(fName, "r+");
    
    if (!f)
    {
        Serial.printf("fileHeapReadRecord[1]: can't open <%s>\r\n", fName.c_str());
        errCode = fhHeaderOpenError;
        return false;
    }
    
    if (f.size() < sizeof(tHeapFileRecHeader) + 1)
    {
        Serial.printf("fileHeapReadRecord[2]: file size of <%s> is less than the header size.\r\n", fName.c_str());
        SPIFFS.remove(fName);
        if (f.size()) errCode = fhFileTooSmall;
        return false;
    }
    
    if (!fileHeapReadHeader(f, fName, &fHeader, fSize, errCode))
    {
        f.close();
        Serial.println("fileHeapReadRecord[3]: error reading header!!! The file will be removed.");
        SPIFFS.remove(fName);
        errCode = fhHeaderReadError;
        return false;
    }

    if (fHeader.recSize > recSize)
    {
        f.close();
        Serial.printf("fileHeapReadRecord[4]: record size is greater than buf size (%d/%d). The file will be removed.\r\n", fHeader.recSize, recSize);
        SPIFFS.remove(fName);
        errCode = fhReadBufferTooSmall;
        return false;
    }
    
    if (!f.seek(fSize - (sizeof(tHeapFileRecHeader) + fHeader.recSize)))
    {
        Serial.printf("fileHeapReadRecord[5]: file <%s> seek error. The file will be removed.\r\n", fName.c_str());
        f.close();
        SPIFFS.remove(fName);
        errCode = fhReadRecordSeekError;
        return false;  
    } 
    
    size_t r = f.read(dataRec, fHeader.recSize);
    f.close();

    if (r != fHeader.recSize)
    {
        Serial.printf("fileHeapReadRecord[6]: file <%s> read error (%d/%d). The file will be removed.\r\n", fName.c_str(), r, fHeader.recSize);
        SPIFFS.remove(fName);
        errCode = fhRecordReadError;
        return false;  
    }
    uint8_t c8 = calcCrc8(dataRec, fHeader.recSize);

    if (c8 != fHeader.fCrc8)
    {
        Serial.printf("fileHeapReadRecord[7]: file <%s> CRC error. The file will be removed.\r\n", fName.c_str());
        SPIFFS.remove(fName);
        errCode = fhRecordCrcError;
        return false;  
    }
    else 
    {
        //Serial.println("CRC OK");
    }
    
    recSize     = fHeader.recSize;
    recID       = fHeader.recordID;    

    uint16_t truncSize =  fHeader.recSize + sizeof(tHeapFileRecHeader);

    if (!fileHeapTruncate(fName, truncSize, errCode)) 
    {
        Serial.printf("fileHeapReadRecord[8]: file <%s> Truncate error. The file will be removed.\r\n", fName.c_str());
        SPIFFS.remove(fName);        
        return false;  
    }

    return true;
}
///////////////////////
bool fileHeapWriteRecord(String fName, byte *dataRec, const uint16_t recSize, const uint32_t recID, tStorageResult &errCode)
{   
   tHeapFileRecHeader fHeader; 
   uint32_t bufSize = recSize + sizeof(tHeapFileRecHeader);   
   fHeader.recSize = recSize;
   fHeader.recordID = recID;
   fHeader.fCrc8 = calcCrc8(dataRec, recSize);
   byte c8 = calcCrc8((byte*)&fHeader, sizeof(tHeapFileRecHeader)-1); 
   fHeader.hCrc8 = c8;
   
   errCode = fhOK;
   byte *buf = new byte[bufSize];   
   if (buf == NULL) 
    {
        Serial.printf("fileHeapWriteRecord: can't create buffer (%d bytes)!!!\r\n", bufSize);
        errCode = fhBufferCreateError;
        return false;
    }
    memcpy(buf, dataRec, recSize);    
    memcpy(buf + recSize, &fHeader, sizeof(tHeapFileRecHeader));
    
    bool res = utilsAppendFileBuf(fName, buf, bufSize);    
    if (!res) errCode = fhRecordAppendError;
    
    delete buf;
    return res;
}
///////////////////////
