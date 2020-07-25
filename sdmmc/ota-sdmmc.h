#ifndef ota_sdmmc_H
#define ota_sdmmc_H

#include <esp_err.h>
#include <esp_log.h>
#include <mbedtls/sha256.h>
#include <mbedtls/aes.h>
#include <esp_ota_ops.h>
#include <esp_task_wdt.h>
#include <esp_partition.h>
#include <string.h>

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <driver/sdmmc_host.h>
#include <sdmmc_cmd.h>
#include "esp_vfs_fat.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>


class OTA_SDMMC
{
	private:
		const char tag[10] = "OTA_SDMMC";
		int8_t _cry = 0;
		uint8_t _firstiv[16] = {0};
		uint8_t _iv[16] = {0};
		mbedtls_aes_context aes;
		FILE *f;

		
		void decrypt(uint8_t *data, uint16_t size);
		void iterator();


	public:
		int8_t init(const char *base_path);
		void crypto(const char *key, const char *iv);
		void download(const char *file);

};


#endif

