#ifndef ota_http_H
#define ota_http_H

#include <esp_err.h>
#include <esp_log.h>
#include <mbedtls/sha256.h>
#include <mbedtls/aes.h>
#include <esp_ota_ops.h>
#include <esp_task_wdt.h>
#include <esp_partition.h>

#ifndef tcp_H
#include "esp32-tcp/tcp.h"
#include "esp32-tcp/tcp.cpp"
#endif

class OTA_HTTP
{
	private:
		const char tag[9] = "OTA_HTTP";
		int8_t _cry = 0;
		uint16_t _port = 80;
		mbedtls_aes_context aes;
		TCP_CLIENT tcp;
		TCP_SERVER host;

		int8_t wait(uint16_t time);
		void decrypt(uint8_t *data, uint16_t size);
		int16_t first_boundary(uint8_t *data);
		int16_t last_boundary(uint8_t *data, uint16_t size);
		void iterator(uint8_t *data, uint16_t data_size);


	public:
		void init(const char *key, uint16_t port);
		void process();
		

};




#endif