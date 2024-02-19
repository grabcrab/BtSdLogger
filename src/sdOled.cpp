#include "sdOled.h"
#include "libESP32Time.h"

SSD1306         display(OLED_ADDR, SDA, SCL);
//  OLED screen text rows:
#define  row1     0     //  y=0 is top row of size 1 text
#define  row2    10
#define  row3    20
#define  row4    30
#define  row5    40
#define  row6    50     //  row7 at 70 is too low

extern float readVbat(void);

void oledInit(bool first)
{
 if (first)
 {
  pinMode(oRST,OUTPUT);
  digitalWrite(oRST, LOW);  
  delay(50);
  digitalWrite(oRST, HIGH);
 
  display.init();
  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.flipScreenVertically();
  display.clear();
  display.display();
 }
}

static int dCnt = 0;

void oledPrintWiFi(String ssid, String pass, bool stat)
{   
    oledInit(false);
    display.clear(); 
    display.drawString(5, row1, "Connect Wi-Fi:" );
    display.drawString(5, row2, ssid);
    display.drawString(5, row3, pass);
    String sStr = "connect";
    if (stat) sStr = "get time";
    for (int i = 0; i < dCnt; i++) sStr += ".";
    display.drawString(5, row4, sStr);
    display.display();
    dCnt++;
    if (dCnt > 3) dCnt = 0;
}


void oledSdError(void)
{   
    oledInit(false);
    display.clear(); 
    display.drawString(5, row1, "ERROR!!!" );
    display.drawString(5, row2, "Can't init");
    display.drawString(5, row3, "SD card");
    display.display();
}


static unsigned long lastOledUpdated = 0;

void oledStatus(uint32_t bCount)
{    
    uint16_t vBat = readVbat() * 1000;
    if (vBat < 1000) 
    {
        delay(5);
        vBat = readVbat() * 1000;
    }

    String sBat = String(vBat);
    bool batLow = false;
    if ((vBat > 2000) && (vBat < 3200)) 
    {
        sBat = "[!!]" + sBat;
        batLow = true;
    }

    if (vBat < 2000) sBat = "";
    

    oledInit(false);
    if (millis() - lastOledUpdated < 1000) return;
    lastOledUpdated = millis();
    char buf[60];    
    int year, month, day, hour, minute, second;
    esp32TimeGet(second, minute, hour, day, month, year);
    sprintf(buf, "%02d:%02d:%02d", hour, minute, second);

    /*char v_buf[60];
    uint16_t vBat = read_battery();
    sprintf(v_buf, "VDD = %u", vBat);*/

    String s_bCount = String(bCount);
    //if (bCount > 10000)    
      //s_bCount = String(bCount/1000) + "kb";

    display.clear();     
    display.drawString(5, row1, buf);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(125, row1, String(sBat));
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(5, row2, "Log writing");
    display.drawString(5, row3, "bytes:");
    display.drawString(5, row4, s_bCount);
    if (batLow)
        display.drawString(5, row5, "!!!BAT LOW!!!");

//    display.drawString(5, row5, v_buf);
    display.display();
}


void oledPowerOff(void)
{
    display.clear();  
    display.drawString(5, row2, "POWER OFF...");
    display.display();
}

void oledRestart(void)
{
    display.clear();  
    display.drawString(5, row2, "RESTART...");
    display.display();
}

void oledPwr(bool pwr)
{
  if (pwr) 
     {
        Serial.println("\n\n>>>>>[BT-LOG] OLED ON\n");
        oledInit(true);
    }
  else 
   {
    Serial.println("\n\n>>>>>[BT-LOG] OLED OFF\n");
    digitalWrite(oRST, LOW);  
   }
}


void oledHello(String dName)
{
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, row2, "BLUETOOTH");
    display.drawString(64, row3, "SERIAL");
    display.drawString(64, row4, "LOGGER");
    display.drawString(64, row5, dName);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.display();

}

