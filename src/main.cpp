#include <Arduino.h>
#include <BluetoothSerial.h>
#include "SPIFFS.h"

#include "fileWriter.h"
#include "sdOled.h"
#include "libUtils.h"
#include "libESP32Time.h"
#include "wifiUtils.h"

BluetoothSerial SerialBT;

#define  SDA          4    
#define  SCL         15     
#define  oRST        16     
#define  OLED_ADDR   0x3C   
#define  LED_PIN     2
#define  VBAT_PIN    35

RTC_DATA_ATTR bool firstBoot = true;

unsigned long lastCharReadMs = 0;
uint32_t bytesCount = 0;

String getBleDeviceName(void)
{
    return "BTLOG_" + utilsGetDeviceID64Hex();
}

static float prevVbat = 0;
extern "C" int rom_phy_get_vdd33();
float readVbat(void)
{    
    float internalBatReading = rom_phy_get_vdd33();
   
    float vBat = (((float)internalBatReading/(float)6245)*3.3);

    if (vBat < 2.0) 
    {
        if (prevVbat)
            vBat = prevVbat;
    }
    else 
    {
        prevVbat = vBat;
    }

    return vBat;
}


void restoreSD(void)
{
    oledPwr(true);    
    while ((!initSD())||(!sdAppendFile("<<SD reconnect>>", 0)))
    {
        oledSdError();
        if (digitalRead(0) == LOW)
        {
            delay(1000);
            if (digitalRead(0) == LOW)
            {
                oledRestart();
                delay(5000);
                ESP.restart();
            }
        }
        String dtS = sdGetPrefix();
        SerialBT.printf("------SD ERROR!!! <%s\r\n", dtS.c_str());
        Serial.printf(">>>>>[BT-LOG] SD ERROR!!! <%s\r\n", dtS.c_str());
        delay(1000);
    }
    oledStatus(bytesCount);
}

void firstBootJob(void)
{
    if (!firstBoot)
    {
        Serial.println("\n\n\n\n>>>>>[BT-LOG] WAKE UP\n\n");  
        wifiWakeUpJob();
        oledHello(getBleDeviceName());
        delay(5000);        
        return;
    }
    Serial.println("\n\n\n\n>>>>>[BT-LOG] BOOT\n\n");  
    firstBoot = false;
    oledHello(getBleDeviceName());
    delay(5000);
    wifiBootJob();
}

void setup() 
{
    pinMode(oRST,OUTPUT);
    //pinMode(LoRa_CS,OUTPUT);  
    pinMode(LED_PIN,OUTPUT);  
    pinMode(0, INPUT);
    digitalWrite(LED_PIN, LOW);
    Serial.begin(115200);
    //digitalWrite(LoRa_CS, DeSelect);  
    Serial1.setRxBufferSize(5000);
    Serial1.begin(115200, SERIAL_8N1, 25);
    
    String btName = getBleDeviceName();
    if (SerialBT.begin(btName))
    {
        Serial.printf(">>>>>[BT-LOG] BT Serial started <%s>\r\n", btName.c_str());
    }
    else 
    {
          Serial.printf(">>>>>[BT-LOG] ERROR starting BT Serial <%s>!!!!\r\n", btName.c_str());
    }
    oledInit();
    firstBootJob();
  
    Serial.println(">>>>>[BT-LOG] Initializing file system done");   

    if (!initSD())
    {
        Serial.println(">>>>>[BT-LOG] SD error!!!");
        restoreSD();        
    }
    
    Serial.println(">>>>>[BT-LOG] <<<< SETUP COMPLETED >>>>");
    sdWriteHeader(); 
}

static unsigned long lastBlinkMs = 0;
static unsigned long lastStillPrintMs = 0;
static unsigned long lastLowBatPrintMs = 0;
void checkVoltage(void)
{
    int vBat = readVbat() * 1000;
    if (vBat < 2000) return;
    if (vBat > 3300) return;
    if (millis() - lastLowBatPrintMs > 30000)
    {
        char buf[128];
        int year, month, day, hour, minute, second;
        esp32TimeGet(second, minute, hour, day, month, year);  
        sprintf(buf, "\r\n\n\n>>>>>[BT-LOG] [%04d-%02d-%02d %02d:%02d:%02d] LOW VOLTAGE: %d mV\r\n\n\n", year, month, day, hour, minute, second, vBat);
        SerialBT.println(buf);
        Serial.print(buf);
        sdAppendFile(buf, 0);
        lastLowBatPrintMs = millis();
    }
}

void processStillAlive(void)
{
    checkVoltage();

    if (millis() - lastCharReadMs < 3000) 
    {        
        lastStillPrintMs = millis();
        return;
    }
    
    if (millis() - lastBlinkMs > 3000)
    {
        lastBlinkMs = millis();
        digitalWrite(LED_PIN, HIGH);
        delay(20);
        digitalWrite(LED_PIN, LOW);
    }

    if (millis() - lastStillPrintMs > 30000)
    {
        lastStillPrintMs = millis();
        float vBat = readVbat();

        String dtS = sdGetPrefix();
        SerialBT.printf("------BT-ALIVE [%.2f] <%s\r\n", vBat, dtS.c_str());
        Serial.printf(">>>>>[BT-LOG] BT-ALIVE [%.2f] <%s\r\n", vBat, dtS.c_str());
    }

}

static bool oledOn = true;
void loop()
{
    char c = Serial1.read();
    if ((c>=0)&&(c<255)) 
    {
        if (!sdAddChar(c, 0))
        {
           restoreSD();
        }
        Serial.print(c);
        SerialBT.print(c);
        bytesCount++;
        digitalWrite(LED_PIN, HIGH);
        lastCharReadMs = millis();
    }  
    else 
    {
        oledStatus(bytesCount);
        digitalWrite(LED_PIN, LOW);
        delay(1);
        wifiSyncTime();
        processStillAlive();
    }
    if (digitalRead(0) == LOW)
    {
        delay(1000);
        if (digitalRead(0) == HIGH)
        {
            oledOn = !oledOn;
            oledPwr(oledOn);
            oledStatus(bytesCount);
            return;
        }
        
        delay(4000);
        if (digitalRead(0) == LOW)
        {            
            oledPwr(true);
            oledPowerOff();
            Serial.println(">>>>>[BT-LOG] Deep sleep...");
            sdFlush();
            delay(5000);            
            esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, LOW);
            esp_deep_sleep_start();
        }
    }
}   

