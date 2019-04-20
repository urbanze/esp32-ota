#ifndef http_H
#define http_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <mbedtls/sha256.h>
#include <mbedtls/aes.h>
#include <esp_ota_ops.h>
#include <soc/rtc_wdt.h>
#include <esp_task_wdt.h>



class OTA_HTTP
{
	private:
			static int8_t _crypted, _exc;
			static uint8_t _key[16];

			static void t_ota_http(void*z);
			

	public:
			void init();
			void init(const char key[]);
			

};




#endif
