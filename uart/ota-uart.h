#ifndef ota_uart_H
#define ota_uart_H

#include <esp_err.h>
#include <esp_log.h>
#include <mbedtls/sha256.h>
#include <mbedtls/aes.h>
#include <esp_ota_ops.h>
#include <esp_task_wdt.h>
#include <esp_partition.h>
#include <driver/uart.h>
#include <string.h>

class OTA_UART
{
	private:
		const char tag[9] = "OTA_UART";
		int8_t _cry = 0;
		uint8_t _firstiv[16] = {0};
		uint8_t _iv[16] = {0};
		mbedtls_aes_context aes;
		uart_port_t _uart;

		int8_t wait(uint16_t time);
		void confirm();
		void decrypt(uint8_t *data, uint16_t size);
		
		
	public:
		void init(uart_port_t uart, uint32_t baud, int8_t pin_tx, int8_t pin_rx);
		void crypto(const char *key, const char *iv);
		void download();

};


#endif

