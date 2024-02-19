#pragma once
#include "Arduino.h"
#include "FS.h"
#include "SPIFFS.h"

#define CRC32_POLY            0xEDB88320
#define CRC32_FILE_BUF_SIZE   1000

uint32_t calcCrc32(String fName, uint32_t &charcnt);
uint32_t calcCrc32(File &file, uint32_t &charcnt);
uint32_t calcCrc32Buf(byte *buf, uint16_t len);


