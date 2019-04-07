#ifndef tcp_H
#define tcp_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <mbedtls/sha256.h>
#include <mbedtls/aes.h>
#include <esp_ota_ops.h>



class OTA_TCP
{

	public:	void init();
			void init(const char key[]);

};


#endif
