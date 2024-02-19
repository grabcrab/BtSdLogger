#pragma once
#include "Arduino.h"
#include "SSD1306.h"     //  OLED

//       OLED       Pin
#define  SDA          4   //  Serial Data
#define  SCL         15   //  Serial Clock
#define  oRST        16   //  Reset
#define  OLED_ADDR   0x3C   //  OLED display TWI address

void oledInit(bool first = true);
void oledPrintWiFi(String ssid, String pass, bool stat);
void oledSdError(void);
void oledStatus(uint32_t bCount);
void oledPowerOff(void);
void oledPwr(bool pwr);
void oledRestart(void);
void oledHello(String dName);
