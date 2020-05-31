# ESP32 Generic OTA
ESP32 OTA over many forms!\
WiFi library: [WiFi](https://github.com/urbanze/esp32-wifi)\
TCP library: [TCP](https://github.com/urbanze/esp32-tcp)

## TCP Performance
* Without Flash write and crypt OFF, can go up to 11.3Mb/s.
* Without Flash write and crypt ON, can go up to 7.3Mb/s.
* With Flash write, can go up to 450Kb/s.


### Simple DOWNLOAD OTA TCP
Download file from external TCP server.
```
WF wifi;
OTA_TCP ota;
wifi.sta_connect("rpi4", "1234567890");

ota.init("");
ota.download("192.168.4.2", 18000);
```

### Simple UPLOAD OTA TCP
Wait upload file from remote client on port 15000.
```
WF wifi;
OTA_TCP ota;
wifi.sta_connect("rpi4", "1234567890");

ota.init("");

while (1)
{
	ota.upload(15000);
}
```

