#include <http.h>
#include <tcp.h>
#include <WiFi.h>


void setup()
{
  WiFi.mode(WIFI_STA);//WiFi in STATION mode
  WiFi.begin("", "");//Connect in your wifi

  OTA_TCP ota;//Create OTA_TCP objetct
  ota.init();//Initialize OTA TCP in your IP and port 22180
}

void loop()
{

}
