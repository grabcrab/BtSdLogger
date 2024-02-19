#pragma once
#include <Arduino.h>

///////////////////////////////////////

String      utilsUint64ToString(uint64_t input);
String      utilsUint64ToHexString(uint64_t input);
uint64_t    utilsGetDeviceID64(void);
String      utilsGetDeviceID64Hex(void);
String      utilsParseParamValue(const String str, String paramName);
void        utilsPrintFile(String fName);
void        utilsPrintBuf(byte *buf, uint16_t size, bool dec = false);
bool        utilsAppendFile(String dstFileName, String srcFileName);
bool        utilsAppendFileBuf(String dstFileName, byte *buf, uint32_t bufSize);
void        utilsGetFsStat(size_t &bytesUsed, size_t &bytesTotal);
void        utilsPrintFsStat(void);
//void        utilsPrintWireDevices(void);
String      utilsMacToString(byte mac[6]);
String      utilsEpochToString(time_t epoch);
void        utilsSPIFFSDir(void);
uint64_t    utilsMACArrayToUint64(uint8_t *buffer);

