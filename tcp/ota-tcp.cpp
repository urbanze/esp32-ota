#include "ota-tcp.h"

int8_t OTA_TCP::wait(uint16_t time, TCP_CLIENT *tcp)
{
    int64_t th = esp_timer_get_time();
    while (esp_timer_get_time() - th < time*1000)
    {
        if (tcp->available()) {return 1;}
        esp_task_wdt_reset();
        //vTaskDelay(pdMS_TO_TICKS(1));
    }

    return 0;
}

void OTA_TCP::decrypt(uint8_t *data, uint16_t size)
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

void OTA_TCP::iterator(TCP_CLIENT *tcp)
{
    esp_err_t err;
    int64_t t1 = 0, t2 = 0;
    uint32_t total = 0;
    esp_ota_handle_t ota_handle = 0;
    const esp_partition_t *ota_partition = NULL;
    ota_partition = esp_ota_get_next_update_partition(NULL);


    err = esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "OTA begin fail [0x%x]", err);
        tcp->stop(); return;
    }

    t1 = esp_timer_get_time()/1000;
    while (wait(2000, tcp))
    {
        uint16_t avl = tcp->available();
        if (avl > 1024) {avl = 1024;}
        if (_cry && (avl%16)) {vTaskDelay(1); continue;}

        total += avl;
        uint8_t data[1024] = {0};
        tcp->readBytes(data, avl);

        decrypt(data, avl);

        err = esp_ota_write(ota_handle, data, avl);
        if (err != ESP_OK)
        {
            ESP_LOGE(tag, "OTA write fail [%x]", err);
            tcp->stop(); return;
        }
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
            tcp->stop();
            esp_restart();
        }
        else
        {
            ESP_LOGE(tag, "OTA set boot partition fail [0x%x]", err);
            tcp->stop(); return;
        }
    }
    else
    {
        ESP_LOGE(tag, "OTA end fail [0x%x]", err);
        tcp->stop(); return;
    }
}



/**
 * @brief Connect to external TCP server and download binary.
 * 
 * @param [*IP]: External host IP.
 * @param [port]: External host port.
 */
void OTA_TCP::download(const char *IP, uint16_t port)
{
    TCP_CLIENT tcp;

    if (!tcp.connecto(IP, port))
    {
        ESP_LOGE(tag, "Fail to connect");
    }

    ESP_LOGI(__func__, "Connected to external server");
    OTA_TCP::iterator(&tcp);
}

/**
 * @brief Host TCP server and wait client send binary up to 1sec.
 * 
 * This function block the current task up to 1sec, waiting clients.
 * if you need this TCP server to always be active,
 *  it is suggested to put it in any dedicated looping task.
 * 
 * @param [port]: TCP port to listening incoming bytes.
 */
void OTA_TCP::upload(uint16_t port)
{
    TCP_CLIENT tcp;
    TCP_SERVER host;

    host.begin(port);
    if (!tcp.connected())
    {
        tcp.get_sock(host.sv(1));
    }

    if (tcp.available())
    {
        ESP_LOGI(__func__, "Client connected");
        OTA_TCP::iterator(&tcp);
    }
}

/**
 * @brief Init OTA TCP functions.
 * 
 * If string length == 0, crypto (AES 256 ECB) will be disabled.
 * 
 * @param [*key]: AES-256 ECB decrypt key (<=32 chars).
 */
void OTA_TCP::init(const char *key)
{
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

