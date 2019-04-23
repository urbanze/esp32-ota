#include <http.h>
#include <tcp.h>
#include <WiFi.h>


void setup()
{
  WiFi.mode(WIFI_STA);//WiFi in STATION mode
  WiFi.begin("", "");//Connect in your wifi

  OTA_HTTP ota;//Create OTA_HTTP object
  ota.init();//Initialize OTA webpage in your IP and PORT 8080 (like: 192.168.x.x:8080)
}

void loop()
{

}
