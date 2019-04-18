#include "tcp.h"

int8_t _crypted = 0, exc = 0;
uint8_t _key[16];

void t_ota_http(void*z)
{
    exc = 1;
    char tag[32];
    if (_crypted)
        {strcpy(tag, "OTA_HTTP Crypted");}
    else
        {strcpy(tag, "OTA_HTTP");}
    
    WiFiServer server(80);
    WiFiClient tcp;
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;
    esp_err_t err;

    char html[512];
    int8_t  dwl = 0;
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
    ESP_LOGI(tag, "Ready to download update via HTTP (8080)...");
    while (1)
    {
        rtc_wdt_feed();
        esp_task_wdt_reset();
    
        dwl = 0;
        if (!tcp.connected())
        {
            tcp = server.available();
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }
	
	ESP_LOGW(tag, "Client connected to html");
	avl = tcp.available();
	if (avl > 2048)
	    {avl = 2048;}
	
	tcp.readBytes(data, avl);

	int16_t html_avl = (avl > 512) ? 512 : avl;
	for (int32_t i = 0; i < html_avl; i++)
	{
	    html[i] = (data[i] > 127) ? 0 : data[i];
	}


	if (strstr(html, "GET") != NULL)
	{
	    ESP_LOGI(tag, "GET Request");
	    const char txt[] =	"<!DOCTYPE html>"
				"<html lang=\"en\"><head><title>ESP32_GOTA HTTP</title>"
				"<link rel=\"shortcut icon\" href=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAY"
				"AAAAf8/9hAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAAD2EAAA9hAag/p2kAAAAZdEVYdFNvZnR"
				"3YXJlAHBhaW50Lm5ldCA0LjAuMTM0A1t6AAABZUlEQVQ4T52SPUsDQRCGN0aDud0zKlb+AsUqIKiFvZUgaKmgjaQSGxv"
				"Rs1EQ94OATar019gJ2uztnidBUmopaKU29vEj51wynUI2PjDFvHPvzNyw5L8wnkx5PClj6k5J6VEqzLEn7TpKjtSaQ56"
				"INqiwj76KllF1Ic0VpZ6FqQ0m7Ksv4nks9IZxPcGUPWfStpgwz1TGM51CoAfJ2RUlwX2hk/8iCAu+ireYNG9gTiHasPo"
				"tNNFUmgfQn2Cji2wAOpA0zXnVpEylTbpG8w2mD2zSDWFaVMRHf073hF5iPFLQYJeJeMVXdg7++xBM7cwMU7MDLuDnPcn"
				"5wm6C+SvbBJrWx6uNEaz1Bo5XgamfEC8Qqyg7AHeAa+9kZlj5snh6PYkVB8IwDy9sDy79zritkLUwjxUHgmAA1t6HqXf"
				"ZO0fVkTDNM24OKLcnpK6HUXUE1oRLbzN5s4hKfzAVTY/VmiVM+4SQH2Ymn5My67oNAAAAAElFTkSuQmCC\"/>"
				"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
				"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/></head><body>"

				"<form enctype=\"multipart/form-data\" action=\"\" method=\"POST\">"
				"<input type=\"file\" name=\"file\">"
				"<br><input type=\"submit\">"
				"</form>"
				

				
				"</body></html>";

			
	    tcp.printf("%s\r\n", txt);
	    vTaskDelay(pdMS_TO_TICKS(5));
	    tcp.stop();
	}

	else if (strstr(html, "POST") != NULL)
	{
	    for (int32_t i = 0; i < avl; i++)
	    {
		if (data[i] == 0xE9)
		{

		    ESP_LOGW(tag, "Magic byte found in %d of %d (%d)", i, avl, avl-i);
		    

		    avl -= i;
		    tt = avl;
		    t2 = esp_timer_get_time();
		    for (int32_t j = 0; j < avl; j++)
		    {
			data[j] = data[j+i];
		    }

		    //ESP_LOGW(tag, "First 4B: [0x%x, 0x%x, 0x%x, 0x%x]", data[i], data[i+1], data[i+2], data[i+3]);

		    
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
		    
		    err = esp_ota_write(update_handle, data, avl);
		    if (err != ESP_OK)
		    {
			tcp.printf("ESP32: Fail[2]:%x\n", err);
			ESP_LOGE(tag, "Fail[2]:%x", err);
			tcp.stop();
			break;
		    }

		    t1 = esp_timer_get_time();
		    while (1)
		    {
			rtc_wdt_feed();
			esp_task_wdt_reset();
			

			if (esp_timer_get_time() - t1 > 3000000)
			{
			    tcp.printf("ESP32: Client timeout");
			    ESP_LOGW(tag, "Client timeout");
			    tcp.stop();
			    break;
			}
			
			avl = tcp.available();
			if (avl)
			{

			    if (avl > 2048)
				{avl = 2048;}

			    		    
			    tcp.readBytes(data, avl);

			    for (int16_t j = 0; j < avl-2; j++)
			    {
				if (data[j] == '\r' && data[j+1] == '\n' && data[j+2] == '-')
				{
				    
				    avl = j;
				    break;
				}
			    }


			    
			    tt += avl;
			    err = esp_ota_write(update_handle, data, avl);
			    if (err != ESP_OK)
			    {
				tcp.printf("ESP32: Fail[3]:%x\n", err);
				ESP_LOGE(tag, "Fail[3]:%x", err);
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
			tcp.printf("ESP32: Fail[5]:%x\n", err);
			ESP_LOGE(tag, "Fail[5]:%x", err);
		    }

		    break;
		}
	    }
	}


	tcp.flush();
	tcp.stop();
    }

    ESP_LOGW(tag, "Task deleted");
    exc = 0;
    vTaskDelete(NULL);
}




void OTA_HTTP::init()
{
    const char tag[] = "OTA_HTTP";
    
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
    xTaskCreatePinnedToCore(t_ota_http, "t_ota_http", 10000, NULL, 5, NULL, tskNO_AFFINITY);
}

void OTA_HTTP::init(const char key[])
{
    const char tag[] = "OTA_HTTP Crypted";
    
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
    xTaskCreatePinnedToCore(t_ota_http, "t_ota_http", 10000, NULL, 5, NULL, tskNO_AFFINITY);
}



