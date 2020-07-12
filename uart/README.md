# ESP32 Ultimate OTA
ESP32 OTA over many forms!\
Crypted functions use AES-256 CBC.

## How it works?
* Basically, OTA UART get file (bytes) sent to ESP32 and write in OTA partition.
* When ESP32 is ready to read new bytes, will send 2 chars "\r\n" in TX pin.
* Your external software that send binary, **need to wait "\r\n" confirmation before send new packet**
* You can't send >1024 Bytes before ESP32 send confirmation "\r\n".
* This library will write **ALL BYTES** received. After start, your external software can't send any byte that are not from the binary file.

## Simple DOWNLOAD OTA UART (Crypto OFF)
Download file from UART.
```
OTA_UART ota;

//Init UART_1 with 115000b/s in pin 23/22
ota.init(UART_NUM_1, 115200, 23, 22);

//Start to read new bytes and write in flash.
ota.download();
```

## Simple DOWNLOAD OTA UART (Crypto ON)
Download file from UART.
```
OTA_UART ota;

//Init UART_1 with 115000b/s in pin 23/22
ota.init(UART_NUM_1, 115200, 23, 22);

//Set AES-256-CBC KEY and initial IV.
ota.crypto("12345678901234567890123456789012", "0123456789012345");

//Start to read new bytes and write in flash.
ota.download();
```

