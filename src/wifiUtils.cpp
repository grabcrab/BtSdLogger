#include "Arduino.h"
#include "libWiFi.h"
#include "sdOled.h"

const String ssid = "ENAiKOON-Technik";
const String pass = "EN2020ik";
const String timeUrl = "http://aziot.xyz/time.php";

static bool wasTimeSync = false;

void wifiWakeUpJob(void)
{
    Serial.printf(">>>>>[BT-LOG] Connecting to Wi-Fi network: %s/%s\r\n", ssid.c_str(), pass.c_str()); 
    wifiInit(ssid, pass);
}

void wifiBootJob(void)
{
    bool isDone = false;
    while(true)
    {
        Serial.printf(">>>>>[BT-LOG] Connecting to Wi-Fi network: %s/%s\r\n", ssid.c_str(), pass.c_str()); 
        oledPrintWiFi(ssid, pass, false);
        wifiInit(ssid, pass);
        while (!wifiIsConnected())  
        {
            delay(1000);
            oledPrintWiFi(ssid, pass, false);
        }


        Serial.println(">>>>>[BT-LOG] Updating time..."); 
        oledPrintWiFi(ssid, pass, true);
        while (true)
        {
            if (wifiIsConnected())
                if (wifiSyncTime(timeUrl))
                {
                    isDone = true;
                    break;
                }
            oledPrintWiFi(ssid, pass, true);
        }

        if (isDone) break;
    }
   
    Serial.println(">>>>>[BT-LOG] wifiBootJob>>>Time has been updated:");
    esp32TimePrint();        
    wasTimeSync = true;           
}

static unsigned long lastTimeSyncMs = 0;
#define TIME_SYNC_INTERVAL_MS  (30*60*1000)
void wifiSyncTime(void)
{
    if ((millis() - lastTimeSyncMs > TIME_SYNC_INTERVAL_MS) || (!wasTimeSync))
    {
        Serial.println(">>>>>[BT-LOG] wifiSyncTime: STARTED");
        if (!wifiIsConnected())  
        {
            Serial.println(">>>>>[BT-LOG] wifiSyncTime: NO WIFI");
            return;
        }
        if (wifiSyncTime(timeUrl))
        {
            Serial.println(">>>>>[BT-LOG] wifiSyncTime>>>Time has been updated:");
            esp32TimePrint();  
            wasTimeSync = true;
            lastTimeSyncMs = millis();
        }
        else
        {
            Serial.println(">>>>>[BT-LOG] wifiSyncTime: SYNC ERROR");
            return;
        }
        Serial.println(">>>>>[BT-LOG] wifiSyncTime: COMPLETED");
    }
}

