#pragma once
#include "Arduino.h"

#define  SD_CLK     18
#define  SD_MISO    19
#define  SD_MOSI    23
#define  SD_CS      5


bool initSD(void);

bool sdAppendFile(const String fName, String txt);
bool sdAppendFile(String txt, int num);
bool sdAddChar(char c, int num);
void sdWriteHeader(void);
void sdFlush(void);

String sdGetPrefix(void);
