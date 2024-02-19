#include "libWiFi.h"
////////////////////////////////
String wifiSsid = "";
String wifiPass = "";
bool   wifiConnected = false; 
bool   wifiInternetConnected = false;

unsigned long lastPingedGoogleMs  = 0;
unsigned long lastSyncTimeMs = 0;



////////////////////////////////
bool wifiIsConnected(void)
{
  return WiFi.isConnected();
}
////////////////////////////////
bool wifiIsInternetConnected(void)
{
  return wifiPingGoogle();
}
////////////////////////////////
void wifiUnhandled_evt(WiFiEvent_t event) 
{
  //Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) 
   {
    case SYSTEM_EVENT_STA_CONNECTED:
        wifiStationConnected_evt(event);
    break;
    
    case SYSTEM_EVENT_WIFI_READY: 
      Serial.println(">>>>>[BT-LOG] WiFi interface ready");
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      Serial.println(">>>>>[BT-LOG] Completed scan for access points");
      break;
    case SYSTEM_EVENT_STA_START:
      Serial.println(">>>>>[BT-LOG] WiFi client started");
      break;
    case SYSTEM_EVENT_STA_STOP:
      Serial.println(">>>>>[BT-LOG] WiFi clients stopped");
      wifiConnected = false; 
      wifiInternetConnected = false;
      break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      Serial.println(">>>>>[BT-LOG] Authentication mode of access point has changed");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
        wifiGotIP_evt(event);    
      break;
    case SYSTEM_EVENT_STA_LOST_IP:
      Serial.println(">>>>>[BT-LOG] Lost IP address and IP address is reset to 0");
      wifiConnected = false; 
      wifiInternetConnected = false;
      break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
      Serial.println(">>>>>[BT-LOG] WiFi Protected Setup (WPS): succeeded in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
      Serial.println(">>>>>[BT-LOG] WiFi Protected Setup (WPS): failed in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
      Serial.println(">>>>>[BT-LOG] WiFi Protected Setup (WPS): timeout in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
      Serial.println(">>>>>[BT-LOG] WiFi Protected Setup (WPS): pin code in enrollee mode");
      break;
    case SYSTEM_EVENT_AP_START:
      Serial.println(">>>>>[BT-LOG] WiFi access point started");
      break;
    case SYSTEM_EVENT_AP_STOP:
      Serial.println(">>>>>[BT-LOG] WiFi access point  stopped");
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      Serial.println(">>>>>[BT-LOG] Client connected");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      Serial.println(">>>>>[BT-LOG] Client disconnected");
      break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
      Serial.println(">>>>>[BT-LOG] Assigned IP address to client");
      break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
      Serial.println(">>>>>[BT-LOG] Received probe request");
      break;
    case SYSTEM_EVENT_GOT_IP6:
      Serial.println(">>>>>[BT-LOG] IPv6 is preferred");
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      wifiStationDisconnected_evt(event);  
      break;
   
  }//switch
}//void wifiUnhandled_evt(WiFiEvent_t event)  
////////////////////////////////////////////
void wifiStationConnected_evt(WiFiEvent_t event)
{
  Serial.println(">>>>>[BT-LOG] Connected to AP successfully!");
}//void wifiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
////////////////////////////////////////////
void wifiGotIP_evt(WiFiEvent_t event)
{
  Serial.print(">>>>>[BT-LOG] WiFi connected. IP address: ");
  Serial.print(WiFi.localIP().toString().c_str());
  Serial.print(" SSID: ");
  Serial.print(WiFi.SSID());
  Serial.print(" RSSI: ");
  Serial.println(WiFi.RSSI());
  wifiConnected = true;
  //wifiInternetConnected = 

//  CURRENT_WIFI_SSID = WiFi.SSID();
//  CURRENT_WIFI_PASS = WiFi.psk();
    
}//void wifiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
////////////////////////////////////////////
static unsigned long lastDisconnectPrintedMs = 0;
void wifiStationDisconnected_evt(WiFiEvent_t event)
{
    if ((millis() - lastDisconnectPrintedMs) > 30000)
    {
        Serial.println(">>>>>[BT-LOG] Disconnected from WiFi access point");
        Serial.println(String(">>>>>[BT-LOG] WiFi lost connection. Trying to Reconnect: ") + wifiSsid + " " + wifiPass);  
        lastDisconnectPrintedMs = millis();
    }
    wifiConnected = false; 
    wifiInternetConnected = false;
    
    WiFi.begin(wifiSsid.c_str(), wifiPass.c_str());  
}//void wifiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
////////////////////////////////////////////////////////////
void wifiInit(String ssid, String pass)
{
   wifiSsid = ssid;
   wifiPass = pass;
      
   WiFi.disconnect(true);

   delay(100);

   WiFi.onEvent(wifiUnhandled_evt);
   //WiFi.onEvent(wifiStationConnected_evt,  SYSTEM_EVENT_STA_CONNECTED);
   //WiFi.onEvent(wifiGotIP_evt, SYSTEM_EVENT_STA_GOT_IP);
   //WiFi.onEvent(wifiStationDisconnected_evt, SYSTEM_EVENT_STA_DISCONNECTED);
   

   WiFi.begin(wifiSsid.c_str(), wifiSsid.c_str());

}//bool wifiInit(void)
////////////////////////////////////////////
bool wifiPingGoogle(void)
{
  if (!wifiIsConnected()) return false;

  if (lastPingedGoogleMs&&(millis()-lastPingedGoogleMs<WIFI_GOOGLE_PING_INTERVAL_MS)) return true;
  
  unsigned long ms = millis();

  String payload = wifiGetString(WIFI_PING_LINK);

  
  if(payload == WIFI_PING_TEXT) 
    {
        Serial.println(String(">>>>>[BT-LOG] Internet Connection check done in ") + (millis()-ms) + " ms. Result: CONNECTED");
        lastPingedGoogleMs = millis();
        return true;
    }
 Serial.println(String(">>>>>[BT-LOG] Internet Connection check done in ") + (millis()-ms) + " ms. Result: !!!NOT CONNECTED!!! (GET request error)");
 
 return false;

}
///////////////////////////////////////
String wifiGetString(String fileLink)
{
  HTTPClient http;

  String payload = "0";

  http.begin(fileLink);
  int httpCode = http.GET();  
  if(httpCode > 0) 
    {
      if(httpCode == HTTP_CODE_OK) 
        {       
          payload = http.getString();                 
          return payload;
        } 
      else 
        {
          Serial.println(String("wifiGetString failed[1], error code = ") + httpCode + " URL = " + fileLink);
          return "";
        }
    }
 Serial.println(String("wifiGetString failed[2], error code = ") + httpCode + " URL = " + fileLink);
 return "";
}
///////////////////////////////////////
int8_t wifiGetRSSI(void)
{
  return WiFi.RSSI();
}
///////////////////////////////////////
uint8_t wifiGetSSPercents(void)
{
  int8_t dBm = wifiGetRSSI();

  if(dBm <= -100)
        return 0;
  else if(dBm >= -50)
        return 100;

  return  2 * (dBm + 100);
}
///////////////////////////////////////
void wifiDisconnect(void)
{
   WiFi.disconnect(true);         
}
///////////////////////////////////////
bool wifiWaitConnection(uint32_t timeoutMs)
{  
  unsigned long m = millis();

  while(millis()-m < timeoutMs)
    if (wifiIsConnected()) 
    { 
      return true;
    }
  else delay(50);
  
  return false;
}
/////////////////////
String wifiGetTimeString(String url)
{
  String res = wifiGetString(url);
  return res;
}
/////////////////////
time_t wifiGetTime(String url)
{
  unsigned long m = millis();
  String s = wifiGetTimeString(url);
  Serial.println("Time string:");
  Serial.println(s);
  unsigned long mm = millis()-m;

  if (s == "") return 0;

  time_t res = (time_t)s.toDouble();
  if (res<MIN_VALID_TIMESTAMP) return 0;
  return res;
}
/////////////////////
bool wifiSyncTime(String url)
{
  time_t epoch = wifiGetTime(url);
  if (!epoch) return false;  
  esp32TimeSet(epoch);
  return true;
}
