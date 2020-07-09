# ESP32 Ultimate OTA (HTTP)
ESP32 OTA over many forms!\
Crypted functions use AES-256 ECB.

WiFi library: [WiFi](https://github.com/urbanze/esp32-wifi)

![image](/docs/http_page.png)

## HTTP Performance
* Without Flash write and crypt OFF, can go up to 11.3Mb/s.
* Without Flash write and crypt ON, can go up to 7.3Mb/s.
* With Flash write, can go up to 570Kb/s.

## How it works?
* Basically, OTA HTTP get file (MIME MEDIA bytes) sent to ESP32 and write in OTA partition.
* .PROCESS() will host TCP server, wait new client in HTTP web page, wait upload file and write all new incoming bytes to OTA partition.
* This library will write **ALL BYTES** received in MIME MEDIA. After start, your external software can't send any byte that are not from the binary file.
* If you use this library in separate task, stack of 4096B should be enough.
* This library can do Factory Reset (last binary burned by USB).

## Simple example to use HTTP (Crypto OFF)
```
WF wifi;
OTA_HTTP ota;
wifi.ap_start("wifi", "1234567890");

ota.init(""); //Default port 80 (you can change in second param)
while (1)
{
	ota.process(); //Wait client connection (up to 1sec) and read bytes sent by client in HTTP web page.
}
```

## Simple example to use HTTP (Crypto ON)
Insert your desired key.
```
WF wifi;
OTA_HTTP ota;
wifi.ap_start("wifi", "1234567890");

ota.init("1234567890"); //Default port 80 (you can change in second param)
while (1)
{
	ota.process(); //Wait client connection (up to 1sec) and read bytes sent by client in HTTP web page.
}
```
