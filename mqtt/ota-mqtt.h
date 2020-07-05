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


class OTA_MQTT
{
	private:
		static void mqtt_events(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

		const char tag[9] = "OTA_MQTT";
		int8_t _cry = 0;
		mbedtls_aes_context aes;
		esp_mqtt_client_handle_t client;
		static int16_t received;
		static uint8_t data[256];
		
		int8_t wait(uint16_t time);
		void decrypt(uint8_t *data, uint16_t size);
		void iterator();

		
	public:
		void init(const char *host, const char *user, const char *pass, const char *id);
		void download(const char *topic);

};


#endif

