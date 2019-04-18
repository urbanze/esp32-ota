#include "tcp.h"

int8_t _crypted = 0, exc = 0;
uint8_t _key[16];

void t_ota(void*z)
{
    exc = 1;
    char tag[16];
    if (_crypted)
        {strcpy(tag, "OTA_TCP Crypted");}
    else
        {strcpy(tag, "OTA_TCP");}
    
    WiFiServer server(22180);
    WiFiClient tcp;
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;
    esp_err_t err;

    uint8_t data[2048];
    uint8_t aes_inp[16], aes_out[16];
    int32_t avl, tt;
    int64_t t1, t2;


    mbedtls_aes_context aes;
    if (_crypted)
    {
        mbedtls_aes_init(&aes);
        mbedtls_aes_setkey_enc(&aes, _key, 128);
    }
    

    server.begin();
    ESP_LOGI(tag, "Ready to download update via TCP (%s:22180)...", WiFi.localIP().toString().c_str());
    while (1)
    {
        rtc_wdt_feed();
        esp_task_wdt_reset();

        
        if (!tcp.connected())
        {
            tcp = server.available();
            vTaskDelay(pdMS_TO_TICKS(250));
            continue;
        }

        tcp.printf("ESP32: Connected, downloading...\n");
        ESP_LOGW(tag, "Client connected, downloading update");
        update_partition = esp_ota_get_next_update_partition(NULL);
        err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
        if (err != ESP_OK)
        {
            tcp.printf("ESP32: Fail[1]:%x, restarting...\n", err);
            ESP_LOGE(tag, "Fail[1]:%x, restarting...", err);
            tcp.stop();
            vTaskDelay(pdMS_TO_TICKS(2000));
            esp_restart();
        }

        tt = 0;
        t1 = esp_timer_get_time();
	t2 = t1;
        while (1)
        {
            rtc_wdt_feed();
            esp_task_wdt_reset();

            if (esp_timer_get_time() - t1 > 3000000)
            {
                tcp.printf("ESP32: Client timeout\n");
                ESP_LOGW(tag, "Client timeout");
                break;
            }

            
            avl = tcp.available();
            if (avl)
            {
                vTaskDelay(2);

                if (avl > 2048)
                    {avl = 2048;}

                tt += avl;
                tcp.readBytes(data, avl);

                if (_crypted)
                {

                    for (int16_t j = 0; j < avl; j += 16)
                    {
                        
                        for (int8_t i = 0; i < 16; i++)
                        {
                            aes_inp[i] = (j+i > avl) ? 0 : data[j+i];
                        }

                        mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, aes_inp, aes_out);

                        for (int8_t i = 0; i < 16; i++)
                        {
                            data[j+i] = aes_out[i];
                        }
                    }

                }
                
                err = esp_ota_write(update_handle, data, avl);
				if (err != ESP_OK)
                {
                    tcp.printf("ESP32: Fail[2]:%x\n", err);
                    ESP_LOGE(tag, "Fail[2]:%x", err);
                    tcp.stop();
                    break;
                }

                t1 = esp_timer_get_time();
            }

            vTaskDelay(1);            
        }

	float auxms = (esp_timer_get_time() - t2)/1000;
	ESP_LOGI(tag, "Downloaded %d Bytes in %d ms (%fKB/s)", tt, int32_t(auxms), tt/auxms);

        err = esp_ota_end(update_handle);
	if (err == ESP_OK)
        {

            err = esp_ota_set_boot_partition(update_partition);
            if (err == ESP_OK)
            {
                tcp.printf("ESP32: Update sucess! Restarting...\n");
                ESP_LOGI(tag, "Update sucess! Restarting...");
                tcp.stop();
                vTaskDelay(pdMS_TO_TICKS(3000));
                esp_restart();
            }
            else
            {
                tcp.printf("ESP32: Fail[4]:%x\n", err);
                ESP_LOGE(tag, "Fail[4]:%x", err);
            }
        }
        else
        {
            tcp.printf("ESP32: Fail[3]:%x\n", err);
            ESP_LOGE(tag, "Fail[3]:%x", err);
        }

        tcp.stop();
    }

    ESP_LOGW(tag, "Task deleted");
    exc = 0;
    vTaskDelete(NULL);
}




void OTA_TCP::init()
{
    const char tag[] = "OTA_TCP";
    
    if (exc)
	{ESP_LOGE(tag, "Already called"); return;}

    
    initArduino();


	if (WiFi.getMode() == WIFI_STA)
	{
		for (int8_t i = 0; i < 100; i++)
		{
			if (WiFi.status() == WL_CONNECTED) {break;}
			vTaskDelay(pdMS_TO_TICKS(50));
		}
		
		if (WiFi.status() != WL_CONNECTED)
		{
			ESP_LOGE(tag, "Please, init OTA after WiFi is connected to STA");
			return;
		}
	}
	else if (WiFi.getMode() != WIFI_AP)
	{
		ESP_LOGE(tag, "Please, init OTA after WiFi STA or AP are OK");
		return;
	}
	
    _crypted = 0;   
    xTaskCreatePinnedToCore(t_ota, "t_ota", 10000, NULL, 5, NULL, tskNO_AFFINITY);
}

void OTA_TCP::init(const char key[])
{
    const char tag[] = "OTA_TCP Crypted";
    
    if (exc)
	{ESP_LOGE(tag, "Already called"); return;}
	
    initArduino();

    if (WiFi.getMode() == WIFI_STA)
	{
		for (int8_t i = 0; i < 100; i++)
		{
			if (WiFi.status() == WL_CONNECTED) {break;}
			vTaskDelay(pdMS_TO_TICKS(50));
		}
		
		if (WiFi.status() != WL_CONNECTED)
		{
			ESP_LOGE(tag, "Please, init OTA after WiFi is connected to STA");
			return;
		}
	}
	else if (WiFi.getMode() != WIFI_AP)
	{
		ESP_LOGE(tag, "Please, init OTA after WiFi STA or AP are OK");
		return;
	}

	if (strlen(key) != 16)
	{
		ESP_LOGE(tag, "Invalid key length, need to be 16 chars"); return;
	}


	for (int8_t i = 0; i < 16; i++)
	{
		_key[i] = key[i];
	}
	
    _crypted = 1;
    xTaskCreatePinnedToCore(t_ota, "t_ota", 10000, NULL, 5, NULL, tskNO_AFFINITY);
}



