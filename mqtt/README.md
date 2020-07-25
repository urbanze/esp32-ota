# ESP32 Ultimate OTA (MQTT)
ESP32 OTA over many forms!\
Crypted functions use AES-256 CBC.

WiFi library: [WiFi](https://github.com/urbanze/esp32-wifi)


## How it works?
* Basically, OTA MQTT get file (bytes) in specific topic that sent to ESP32 and write in OTA partition.
* This library will write **ALL BYTES** received. After start, your external software can't send any byte that are not from the binary file.
* If you use this library in separate task, stack of 4096B should be enough.


## Simple OTA MQTT without USER/PASS/ID (Crypto OFF)
```
WF wifi;
OTA_MQTT ota;

wifi.sta_connect("wifi", "1234567890");


//Without (user/pass/id). Attention: Dont forget 'mqtt://' and ':port' in host
ota.init("mqtt://192.168.0.101:1883");

//Download binary in topic 'ota'
ota.download("ota");
```

## Simple OTA MQTT with USER/PASS/ID (Crypto OFF)
```
WF wifi;
OTA_MQTT ota;

wifi.sta_connect("wifi", "1234567890");


//With (user/pass/id). Attention: Dont forget 'mqtt://' and ':port' in host
ota.init("mqtt://192.168.0.101:1883", "guest", "12345", "ESP32");

//Download binary in topic 'ota'
ota.download("ota");
```

## Simple OTA MQTT without USER/PASS/ID (Crypto ON)
```
WF wifi;
OTA_MQTT ota;

wifi.sta_connect("wifi", "1234567890");


//Without (user/pass/id). Attention: Dont forget 'mqtt://' and ':port' in host
ota.init("mqtt://192.168.0.101:1883");

//Set AES-256-CBC KEY and initial IV.
ota.crypto("12345678901234567890123456789012", "0123456789012345");

//Download binary in topic 'ota'
ota.download("ota");
```

## MQTT Python script to send .bin
You can use a simple Python script to send binary file to specific topic with retain flag=1.
```
import paho.mqtt.client as mqtt
import time


#Read binary in this path (change to your case)
f = open("/home/[user]/esp32.bin", "rb")
fd = f.read()
f.close()
array = bytearray(fd)

#Send to broker (127.0.0.1:1883) in topic 'ota'
client = mqtt.Client("destkop")
client.connect("127.0.0.1", 1883)
client.publish("ota", array, 2, 1)

client.loop_start()
time.sleep(2)
client.loop_stop()
```
