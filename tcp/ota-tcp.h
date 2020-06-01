#ifndef ota_tcp_H
#define ota_tcp_H

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

class OTA_TCP
{
	private:
		const char tag[8] = "OTA_TCP";
		int8_t _cry = 0;
		mbedtls_aes_context aes;

		int8_t wait(uint16_t time, TCP_CLIENT *tcp);
		void decrypt(uint8_t *data, uint16_t size);
		void iterator(TCP_CLIENT *tcp);

		
	public:
		void init(const char key[]);
		void download(const char *IP, uint16_t port);
		void upload(uint16_t port);

};


#endif

