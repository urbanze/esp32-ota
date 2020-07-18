#ifndef ota_mqtt_H
#define ota_mqtt_H

#include <esp_err.h>
#include <esp_log.h>
#include <mbedtls/sha256.h>
#include <mbedtls/aes.h>
#include <esp_ota_ops.h>
#include <esp_task_wdt.h>
#include <esp_partition.h>
#include <string.h>
#include <mqtt_client.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>


class OTA_MQTT
{
	private:
		static esp_err_t mqtt_events(esp_mqtt_event_handle_t event);

		const char tag[9] = "OTA_MQTT";
		static int8_t _connected;
		int8_t _cry = 0;
		uint8_t _firstiv[16] = {0};
		uint8_t _iv[16] = {0};
		static QueueHandle_t qbff;
		mbedtls_aes_context aes;
		esp_mqtt_client_handle_t client;

		
		int8_t wait(uint16_t time);
		void decrypt(uint8_t *data, uint16_t size);
		void iterator();

		
	public:
		void init(const char *host, const char *user, const char *pass, const char *id);
		void crypto(const char *key, const char *iv);
		void download(const char *topic);

};


#endif

