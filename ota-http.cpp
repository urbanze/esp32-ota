#include "http.h"

int8_t OTA_HTTP::_exc = 0;
int8_t OTA_HTTP::_crypted = 0;
uint8_t OTA_HTTP::_key[16] = {0};


void OTA_HTTP::t_ota_http(void*z)
{
   
    _exc = 1;
    char tag[32];
    if (_crypted)
	{strcpy(tag, "OTA_HTTP Crypted");}
    else
	{strcpy(tag, "OTA_HTTP");}
    
    WiFiServer server(8080);
    WiFiClient tcp;
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;
    esp_err_t err;

    char html[512];
    uint8_t data[2048];
    //uint8_t aes_inp[16], aes_out[16];
    int32_t avl, tt;
    int64_t t1, t2;


    mbedtls_aes_context aes;
    if (_crypted)
    {
		mbedtls_aes_init(&aes);
		mbedtls_aes_setkey_enc(&aes, _key, 128);
    }
    

    
    server.begin();
    ESP_LOGI(tag, "Ready to download new updates via HTTP (http://%s:8080)", WiFi.localIP().toString().c_str());
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
		
		ESP_LOGI(tag, "Client connected");
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
			//ESP_LOGI(tag, "GET Request: %s", html);
			
			if (strstr(html, "GET /") != NULL)
			{

			char txt[2048];// char sha[64];
			const esp_partition_t* papp = esp_ota_get_running_partition();
			//const esp_app_desc_t *dapp = esp_ota_get_app_description();
			//esp_ota_get_app_elf_sha256(sha, 64);
			
			
			
			snprintf(txt, 2048, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
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
			tcp.printf("%s", txt);


			
			
			snprintf(txt, 2048, "<font size='5' color='black'><b>Current Device & APP info</b></font><br>"
						"<b>Total Free RAM:</b> %u Bytes<br>"
						"<b>UP-Time:</b> %lld ms<br>"
						"<b>Last reset reason:</b> (comming soon)<br>"
						"<b>APP SHA-256:</b> (comming soon)<br>"
						"<b>IDF Version:</b> (comming soon)<br>"
						"<b>Current APP:</b> %s<br>"






						, heap_caps_get_free_size(MALLOC_CAP_INTERNAL), int64_t(esp_timer_get_time()/1000), papp->label);
			tcp.printf("%s", txt);




			snprintf(txt, 2048, "<br><br><br><form enctype=\"multipart/form-data\" action=\"/upload\" method=\"post\">"
						"<font size='5' color='black'><b>Update new APP: </b></font>"
						"<input type=\"file\" name=\"file\" accept='.bin'> "
						"<input type=\"submit\" value=\"Send file\" onclick=\"return confirm('This will update ESP32, sure?')\">"
						"</form><br><br>"

						"<form action=\"/factory\" method=\"post\">"
						"<font size='5' color='red'><b>Factory reset: </b></font>"
						"<input type=\"submit\" value=\"Start\" onclick=\"return confirm('This will reset ESP32 to factory APP (last since Serial Flash), sure?')\">"
						"</form><br><br>");
			
						



					
			tcp.printf("%s</body></html>\r\n", txt);
			vTaskDelay(pdMS_TO_TICKS(2));
			tcp.stop();
			}
		}
		else if (strstr(html, "POST") != NULL)
		{
			//ESP_LOGI(tag, "POST Request: %s", html);
			
			const char txt[] ="HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
							"<!DOCTYPE html>"
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
							"<meta http-equiv='refresh' content='15' >"
							"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/></head><body>"
							"<b>HTML auto-refresh in 15sec...</b><br><br>";
						
			tcp.printf("%s", txt);
			
			if (strstr(html, "POST /upload HTTP") != NULL)
			{
				for (int32_t i = 0; i < avl; i++)
				{
					if (data[i] == 0xE9)
					{
					
					ESP_LOGI(tag, "Magic Byte found in %d of %d", i, avl);


					

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
						tcp.printf("Fail to start partition [0x%x], restarting...</body></html>\r\n", err);
						ESP_LOGE(tag, "Fail to start partition [0x%x], restarting...", err);
						vTaskDelay(pdMS_TO_TICKS(5));
						tcp.stop();
						vTaskDelay(pdMS_TO_TICKS(1000));
						esp_restart();
					}
					
					err = esp_ota_write(update_handle, data, avl);
					if (err != ESP_OK)
					{
						tcp.printf("Fail to start partition writes [0x%x]</body></html>\r\n", err);
						ESP_LOGE(tag, "Fail to start writes [0x%x]", err);
						vTaskDelay(pdMS_TO_TICKS(5));
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
						tcp.printf("Client data timeout<br>");
						ESP_LOGW(tag, "Client data timeout");
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
							tcp.printf("Fail to write in partition [0x%x]<br>", err);
							ESP_LOGE(tag, "Fail to write in partition [0x%x]", err);
							break;
						}
						t1 = esp_timer_get_time();
						
						}

						vTaskDelay(1);
					}

					float auxms = (t1 - t2);
					auxms /= 1000000;
					

					tcp.printf("Downloaded %d Bytes in %.3fsec (%.3fKB/s)<br>", tt, auxms, (tt/auxms/1000));
					ESP_LOGI(tag, "Downloaded %d Bytes in %.3fsec (%.3fKB/s)", tt, auxms, (tt/auxms/1000));
					
					
					err = esp_ota_end(update_handle);
					if (err == ESP_OK)
					{
						err = esp_ota_set_boot_partition(update_partition);
						if (err == ESP_OK)
						{
						tcp.printf("Update sucess! Restarting...</body></html>\r\n");
						ESP_LOGI(tag, "Update sucess! Restarting...");
						vTaskDelay(pdMS_TO_TICKS(5));
						tcp.stop();
						vTaskDelay(pdMS_TO_TICKS(1000));
						esp_restart();
						}
						else
						{
						tcp.printf("Fail to set boot partition [0x%x]</body></html>\r\n", err);
						ESP_LOGE(tag, "Fail to set boot partition [0x%x]", err);
						vTaskDelay(pdMS_TO_TICKS(5));
						}
					}
					else
					{
						tcp.printf("Fail to finish update [0x%x]</body></html>\r\n", err);
						ESP_LOGE(tag, "Fail to finish update [0x%x]", err);
						vTaskDelay(pdMS_TO_TICKS(5));
					}

					break;
					}

					if (i == avl-1)
					{
					//tcp.printf("Magic Byte 0xE9 not found, check this binary</body></html>\r\n");
					ESP_LOGW(tag, "Magic Byte 0xE9 not found, check this binary");
					//vTaskDelay(pdMS_TO_TICKS(5));
					//tcp.stop();
					//vTaskDelay(pdMS_TO_TICKS(5));
					}
				}
			}

			if (strstr(html, "POST /factory HTTP") != NULL)
			{
				ESP_LOGI(tag, "Factory reseting...");
				
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
						tcp.printf("Fail to set factory partition [0x%x], restarting...</body></html>\r\n", err);
						ESP_LOGE(tag, "Fail to set factory partition [0x%x], restarting...", err);
						tcp.stop();
						vTaskDelay(pdMS_TO_TICKS(1000));
						esp_restart();
					}

					tcp.printf("Sucess! Restarting to factory app...</body></html>\r\n");
					ESP_LOGI(tag, "Sucess! Restarting to factory app...");
					tcp.stop();
					vTaskDelay(pdMS_TO_TICKS(1000));
					esp_restart();
				}
				else
				{
					tcp.printf("Fail to find factory partition, restarting...</body></html>\r\n");
					ESP_LOGE(tag, "Fail to find factory partition, restarting...");
					tcp.stop();
					vTaskDelay(pdMS_TO_TICKS(1000));
					esp_restart();
				}

			
			}

		}

		
		tcp.flush();
		tcp.stop();
    }

    ESP_LOGW(tag, "Task deleted");
    _exc = 0;
    vTaskDelete(NULL);

}



void OTA_HTTP::init()
{
    const char tag[] = "OTA_HTTP";
    
    
    
    if (_exc)
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
    xTaskCreatePinnedToCore(&OTA_HTTP::t_ota_http, "t_ota_http", 10000, NULL, 5, NULL, tskNO_AFFINITY);
}

void OTA_HTTP::init(const char key[])
{
    const char tag[] = "OTA_HTTP Crypted";
    
	ESP_LOGE(tag, "In development, please use the NO-CRYPTED method. Starting NO-CRYPTED method...");
	OTA_HTTP::init();
	return;

    if (_exc)
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
    xTaskCreatePinnedToCore(&OTA_HTTP::t_ota_http, "t_ota_http", 10000, NULL, 5, NULL, tskNO_AFFINITY);
}



