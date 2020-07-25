# ESP32 Ultimate OTA (SD Card)
ESP32 OTA over many forms!\
Crypted functions use AES-256 CBC.

## How it works?
* Basically, OTA SD Card (SDMMC) get file (bytes) in specific path and write in OTA partition.
* If you use this library in separate task, stack of 4096B should be enough.

## Pins
This library use SDMMC_SLOT_1 with internal PULL-UPs. You may need to put external pull-ups.

* CMD = GPIO_NUM_15.
* CLK = GPIO_NUM_14.
* D0  = GPIO_NUM_2.
* D1  = GPIO_NUM_4.
* D2  = GPIO_NUM_12.
* D3  = GPIO_NUM_13.
* CD = Not used.
* WP = Not used.

## Simple OTA SD (Crypto OFF)
```
OTA_SDMMC ota;

//Init SD card
if (ota.init())
{
    ota.download("esp32.bin"); //Download file esp32.bin (root)
}
```

## Simple OTA SD (Crypto ON)
```
OTA_SDMMC ota;

//Init SD card
if (ota.init())
{
    //Set AES-256-CBC Key and IV.
    ota.crypto("12345678901234567890123456789012", "0123456789012345");

    //Download file cbc.bin (root)
    ota.download("cbc.bin");
}
```
