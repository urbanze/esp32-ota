# ESP32 Generic OTA
ESP32 OTA over many forms!\
Crypted functions use AES-256 ECB.

WiFi library: [WiFi](https://github.com/urbanze/esp32-wifi)

## TCP Performance
* Without Flash write (just TCP) and crypt OFF, can go up to 11.3Mb/s.
* Without Flash write (just TCP) and crypt ON, can go up to 7.3Mb/s.
* With Flash write, can go up to 450Kb/s.

## How it works?
* Basically, OTA TCP get file (bytes) sent to ESP32 and write in OTA partition.
* OTA DOWNLOAD will connect to an external TCP server and write all new incoming bytes to OTA partition.
* OTA UPLOAD will host TCP server, wait new client and write all new incoming bytes to OTA partition.
* This library will write **ALL BYTES** received. After start, your external software can't send any byte that are not from the binary file.

## Simple DOWNLOAD OTA TCP (Crypto OFF)
Download file from external TCP server.
```
WF wifi;
OTA_TCP ota;
wifi.sta_connect("rpi4", "1234567890"); //Connect in external WiFi.

ota.init(""); //Init OTA TCP with crypto OFF.
ota.download("192.168.4.2", 18000); //Download OTA hosted in '192.168.4.2:18000'
```

## Simple DOWNLOAD OTA TCP (Crypto ON)
Download file from external TCP server.
```
WF wifi;
OTA_TCP ota;
wifi.sta_connect("rpi4", "1234567890"); //Connect in external WiFi.

ota.init("1234567890"); //Init OTA TCP with crypto ON.
ota.download("192.168.4.2", 18000); //Download OTA hosted in '192.168.4.2:18000'
```

## Simple UPLOAD OTA TCP (Crypto OFF)
Wait upload file from remote client on port 15000.
```
WF wifi;
OTA_TCP ota;
wifi.sta_connect("rpi4", "1234567890"); //Connect in external WiFi.

ota.init(""); //Init OTA TCP with crypto OFF.

while (1)
{
	ota.upload(15000); //Wait client to send OTA binary in port 15000.
}
```

