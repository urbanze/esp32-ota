#include "ota-http.h"

/**
 * @brief Wait any byte available.
 * 
 * This function return if any byte available.
 * 
 * @param [time]: Max milliseconds to wait.
 * 
 * @return [0]: None byte available in time.
 * @return [1]: Byte available to read.
 */
int8_t OTA_HTTP::wait(uint16_t time)
{
    int64_t th = esp_timer_get_time();
    while (esp_timer_get_time() - th < time*1000)
    {
        if (tcp.available()) {return 1;}
        esp_task_wdt_reset();
        //vTaskDelay(pdMS_TO_TICKS(1));
    }

    return 0;
}

/**
 * @brief Decrypt data received (if enabled)
 * 
 * This function replace all old array data (crypted) with decrypted bytes.
 * 
 * @param [*data]: Crypted array data.
 * @param [size]: Size of array.
 */
void OTA_HTTP::decrypt(uint8_t *data, uint16_t size)
{
    if (_cry)
    {
        uint8_t aes_inp[16] = {0}, aes_out[16] = {0};
        for (uint16_t j = 0; j < size; j += 16)
        {
            for (int8_t i = 0; i < 16; i++)
            {
                aes_inp[i] = (j+i > size) ? 0 : data[j+i];
            }

            mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, aes_inp, aes_out);

            for (int8_t i = 0; i < 16; i++)
            {
                data[j+i] = aes_out[i];
            }
        }
    }
}

/**
 * @brief Detect first boundary of MIME MEDIA.
 * 
 * @param [*data]: Data with boundary.
 * 
 * @return [-1]: Not found.
 * @return [>=0]: Position of first MEDIA byte.
 */
int16_t OTA_HTTP::first_boundary(uint8_t *data)
{
	char *ptr = strstr((char*)data, "octet-stream\r\n\r\n");

	if (ptr != NULL)
	{
		return (uint8_t*)ptr-data+16;
	}

	return -1;
}

/**
 * @brief Detect last boundary of MIME MEDIA.
 * 
 * @param [*data]: Data with boundary.
 * 
 * @return [-1]: Not found.
 * @return [>=0]: Position of last MEDIA byte.
 */
int16_t OTA_HTTP::last_boundary(uint8_t *data, uint16_t size)
{
	for (int16_t i = 0; i < size-4; i++)
	{
		if (data[i] == '\r' && data[i+1] == '\n' && data[i+2] == '-' && data[i+3] == '-' && data[i+4] == '-')
		{
			return i;
		}
	}

	// char *ptr = strstr((char*)data, "\r\n---");

	// if (ptr != NULL)
	// {
	// 	return (uint8_t*)ptr-data;
	// }

	return -1;
}

/**
 * @brief Process data received and manage OTA API.
 * 
 * @param [*data]: Data received after HTTP post /upload.
 * @param [size]: Size of data received after HTTP post /upload.
 */
void OTA_HTTP::iterator(uint8_t *data, uint16_t data_size)
{
    esp_err_t err;
    int64_t t1 = 0, t2 = 0, th = 0;
    uint32_t total = data_size;
    esp_ota_handle_t ota_handle = 0;
    const esp_partition_t *ota_partition = NULL;
    ota_partition = esp_ota_get_next_update_partition(NULL);


    err = esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "OTA begin fail [0x%x]", err);
		tcp.printf("OTA begin fail [0x%x]</body></html>\r\n", err);
		return;
    }


	//First write comming from packet read in process()
	t1 = esp_timer_get_time()/1000;
	decrypt(data, data_size);
	err = esp_ota_write(ota_handle, data, data_size);
	if (err != ESP_OK)
	{
		ESP_LOGE(tag, "OTA write fail [0x%x]", err);
		tcp.printf("OTA write fail [0x%x]</body></html>\r\n", err);
		return;
	}
    
    while (wait(2000))
    {
        uint16_t avl = tcp.available();
        if (avl > 1024) {avl = 1024;}
        if (_cry && (avl%16))
		{
			//When crypto enabled, AES work with blocks of 16B.
			//[Crypted binary file] MOD 16 == 0, but HTTP insert many headers.
			//After all bytes sent, tcp.available() may not be MODULE of 16, causing infinity loop.
			//This instructions break if this loop occurs, removing desnecessary bytes.
			if (!th) {th = esp_timer_get_time();}
			if (esp_timer_get_time() - th < 2000000)
			{
				for (int16_t i = 0; i < (avl%16); i++)
					{tcp.read();}
			}

			vTaskDelay(1);
			continue;
		}

		th = 0;//Crypted loop aux.
		memset(data, 0, 1024);
        tcp.readBytes(data, avl);

		int16_t file_end = last_boundary(data, avl);
		if (file_end > -1) {avl = file_end;}

		total += avl;

        decrypt(data, avl);

        err = esp_ota_write(ota_handle, data, avl);
        if (err != ESP_OK)
        {
            ESP_LOGE(tag, "OTA write fail [0x%x]", err);
			tcp.printf("OTA write fail [0x%x]</body></html>\r\n", err);
            return;
        }

		esp_task_wdt_reset();

		if (file_end > -1) {break;}
    }
    t2 = (esp_timer_get_time()/1000)-2000;
    ESP_LOGI(tag, "Downloaded %dB in %dms", total, int32_t(t2-t1));

    err = esp_ota_end(ota_handle);
    if (err == ESP_OK)
    {
        err = esp_ota_set_boot_partition(ota_partition);
        if (err == ESP_OK)
        {
            ESP_LOGW(tag, "OTA OK, restarting...");
			tcp.printf("OTA OK, restarting...</body></html>\r\n");
			vTaskDelay(1);
            tcp.stop();
            esp_restart();
        }
        else
        {
            ESP_LOGE(tag, "OTA set boot partition fail [0x%x]", err);
			tcp.printf("OTA set boot partition fail [0x%x]</body></html>\r\n", err);
            return;
        }
    }
    else
    {
        ESP_LOGE(tag, "OTA end fail [0x%x]", err);
		tcp.printf("OTA end fail [0x%x]</body></html>\r\n", err);
        return;
    }
}

/**
 * @brief Create HTTP web page, wait client and process OTA data.
 * 
 * This function will listen the port (.init parameter) up to 1sec.
 * If you need this HTTP web page of all project lifetime, put in infinity loop!
 */
void OTA_HTTP::process()
{
	esp_task_wdt_reset();

    if (!tcp.connected())
    {
        tcp.get_sock(host.sv(1));
    }

	vTaskDelay(pdMS_TO_TICKS(5));
    if (!tcp.available()) {tcp.stop(); return;}
	vTaskDelay(pdMS_TO_TICKS(50));
	ESP_LOGI(tag, "Client connected [%d]", tcp.available());


	uint8_t data[1024] = {0};
	uint32_t avl = tcp.available();
	if (avl > 1024) {avl = 1024;}
	
	tcp.readBytes(data, avl);


	if (strstr((char*)data, "GET /") != NULL)
	{
		int16_t size;
		uint8_t macs[6] = {0};
		char app_sha[32] = {0};
		const esp_partition_t* papp = esp_ota_get_running_partition();
		
		esp_efuse_mac_get_default(macs);
		int64_t tnow = esp_timer_get_time()/1000;
		uint32_t rnow = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
		const esp_app_desc_t *app_desc = esp_ota_get_app_description();
		esp_ota_get_app_elf_sha256(app_sha, 32);
		
		
		size = snprintf((char*)data, 1024,
			"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
			"<!DOCTYPE html>"
			"<html lang=\"en\"><head><title>ESP32 Generic OTA</title>"
			"<link rel=\"shortcut icon\" href=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAY"
			"AAAAf8/9hAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAAD2EAAA9hAag/p2kAAAAZdEVYdFNvZnR"
			"3YXJlAHBhaW50Lm5ldCA0LjAuMTM0A1t6AAABZUlEQVQ4T52SPUsDQRCGN0aDud0zKlb+AsUqIKiFvZUgaKmgjaQSGxv"
			"Rs1EQ94OATar019gJ2uztnidBUmopaKU29vEj51wynUI2PjDFvHPvzNyw5L8wnkx5PClj6k5J6VEqzLEn7TpKjtSaQ56"
			"INqiwj76KllF1Ic0VpZ6FqQ0m7Ksv4nks9IZxPcGUPWfStpgwz1TGM51CoAfJ2RUlwX2hk/8iCAu+ireYNG9gTiHasPo"
			"tNNFUmgfQn2Cji2wAOpA0zXnVpEylTbpG8w2mD2zSDWFaVMRHf073hF5iPFLQYJeJeMVXdg7++xBM7cwMU7MDLuDnPcn"
			"5wm6C+SvbBJrWx6uNEaz1Bo5XgamfEC8Qqyg7AHeAa+9kZlj5snh6PYkVB8IwDy9sDy79zritkLUwjxUHgmAA1t6HqXf"
			"ZO0fVkTDNM24OKLcnpK6HUXUE1oRLbzN5s4hKfzAVTY/VmiVM+4SQH2Ymn5My67oNAAAAAElFTkSuQmCC\"/>"
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
			"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/></head><body>");
		tcp.write(data, size);

		size = snprintf((char*)data, 1024,
			"<font size='5' color='black'><b>Current Device info</b></font><br><br>"
			"<b>WiFi STA MAC:</b> %02x:%02x:%02x:%02x:%02x:%02x<br>"
			"<b>Free RAM:</b> %uB<br>"
			"<b>UP-Time:</b> %lldms<br>"
			"<b>APP SHA-256:</b> %s<br>"
			"<b>APP Version:</b> %s<br>"
			"<b>IDF Version:</b> %s<br>"
			"<b>Current APP:</b> %s<br>"

			, macs[0], macs[1], macs[2], macs[3], macs[4], macs[5]
			, rnow
			, tnow
			, app_sha
			, app_desc->version
			, app_desc->idf_ver
			, papp->label);
		tcp.write(data, size);

		size = snprintf((char*)data, 1024,
			"<br><br><br><form enctype=\"multipart/form-data\" action=\"/upload\" method=\"post\">"
			"<font size='5' color='black'><b>Update new APP: </b></font>"
			"<input type=\"file\" name=\"file\" accept='.bin'> "
			"<input type=\"submit\" value=\"Send file\" onclick=\"return confirm('Confirm?')\">"
			"</form><br><br>"

			"<form action=\"/factory\" method=\"post\">"
			"<font size='5' color='red'><b>Factory reset: </b></font>"
			"<input type=\"submit\" value=\"Start\" onclick=\"return confirm('Factory reset?')\">"
			"</form><br><br>");
		tcp.write(data, size);


		tcp.printf("</body></html>\r\n");
	}
	else if (strstr((char*)data, "POST /") != NULL)
	{

		tcp.printf(
			"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
			"<!DOCTYPE html>"
			"<html lang=\"en\"><head><title>ESP32 Generic OTA</title>");

		tcp.printf(
			"<link rel=\"shortcut icon\" href=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAY"
			"AAAAf8/9hAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAAD2EAAA9hAag/p2kAAAAZdEVYdFNvZnR"
			"3YXJlAHBhaW50Lm5ldCA0LjAuMTM0A1t6AAABZUlEQVQ4T52SPUsDQRCGN0aDud0zKlb+AsUqIKiFvZUgaKmgjaQSGxv"
			"Rs1EQ94OATar019gJ2uztnidBUmopaKU29vEj51wynUI2PjDFvHPvzNyw5L8wnkx5PClj6k5J6VEqzLEn7TpKjtSaQ56"
			"INqiwj76KllF1Ic0VpZ6FqQ0m7Ksv4nks9IZxPcGUPWfStpgwz1TGM51CoAfJ2RUlwX2hk/8iCAu+ireYNG9gTiHasPo"
			"tNNFUmgfQn2Cji2wAOpA0zXnVpEylTbpG8w2mD2zSDWFaVMRHf073hF5iPFLQYJeJeMVXdg7++xBM7cwMU7MDLuDnPcn"
			"5wm6C+SvbBJrWx6uNEaz1Bo5XgamfEC8Qqyg7AHeAa+9kZlj5snh6PYkVB8IwDy9sDy79zritkLUwjxUHgmAA1t6HqXf"
			"ZO0fVkTDNM24OKLcnpK6HUXUE1oRLbzN5s4hKfzAVTY/VmiVM+4SQH2Ymn5My67oNAAAAAElFTkSuQmCC\"/>");

		tcp.printf(
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
			"<meta http-equiv='refresh' content='20' >"
			"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/></head><body>"
			"<b>Auto-refresh in 20sec...</b><br><br>");
					
		
		vTaskDelay(pdMS_TO_TICKS(10));
		if (strstr((char*)data, "POST /upload") != NULL)
		{
			int16_t file = first_boundary(data); //Pointer to first .bin byte.
			int16_t fsz = 1024-file; //How many bytes remain after remove http header.

			if (file > -1)
			{
				memcpy(data, data+file, fsz);
				for (int16_t i = fsz; i < 1024; i++)
					{data[i] = tcp.read();}

				OTA_HTTP::iterator(data, 1024);
			}
			else
			{
				ESP_LOGE(tag, "Fail to find start of file");
				tcp.printf("Fail to find start of file</body></html>\r\n");
			}
		}
		else if (strstr((char*)data, "POST /factory") != NULL)
		{
			esp_partition_iterator_t pit;
			const esp_partition_t* fapp;
			esp_err_t err;


			pit = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
			if (pit != NULL)
			{
				fapp = esp_partition_get(pit);
				esp_partition_iterator_release(pit);
				err = esp_ota_set_boot_partition(fapp);
				if (err != ESP_OK)
				{
					ESP_LOGE(tag, "Fail to set factory partition [0x%x]", err);
					tcp.printf("Fail to set factory partition [0x%x]</body></html>\r\n", err);
				}

				ESP_LOGW(tag, "Sucess! Restarting to factory APP...");
				tcp.printf("Sucess! Restarting to factory APP...</body></html>\r\n");
				tcp.stop();
				esp_restart();
			}
			else
			{
				ESP_LOGE(tag, "Fail to find factory partition, restarting...");
				tcp.printf("Fail to find factory partition, restarting...</body></html>\r\n");
			}
		}
	}

	vTaskDelay(1);
	tcp.flush();
	tcp.stop();
}

/**
 * @brief Init OTA HTTP functions.
 * 
 * If string length == 0, crypto (AES 256 ECB) will be disabled.
 * 
 * @param [*key]: AES-256 ECB decrypt key (<=32 chars).
 * @param [port]: Port to use in HTTP web server. (eg: 8080, 80, etc)
 */
void OTA_HTTP::init(const char *key, uint16_t port=80)
{
	_port = port;
    host.begin(_port);

    if (strlen(key))
	{
		char key2[32] = {0};
        _cry = 1;
        
        strncpy(key2, key, sizeof(key2));

        mbedtls_aes_init(&aes);
        mbedtls_aes_setkey_enc(&aes, (uint8_t*)key2, 256);
	}
	else
	{
		_cry = 0;
	}
}

