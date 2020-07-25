#include "ota-mqtt.h"

QueueHandle_t OTA_MQTT::qbff;
int8_t OTA_MQTT::_connected = 0;

/**
 * @brief Process binary received from topic subscribed.
 */
esp_err_t OTA_MQTT::mqtt_events(esp_mqtt_event_handle_t event)
{
    //ESP_LOGI(__func__, "%d", event->event_id);

    if (event->event_id == MQTT_EVENT_CONNECTED)
    {
        _connected = 1;
    }
    else if (event->event_id == MQTT_EVENT_DISCONNECTED)
    {
        _connected = 0;
        xQueueReset(qbff);
    }
    else if (event->event_id == MQTT_EVENT_DATA)
    {
        for (uint16_t i = 0; i < event->data_len; i++)
        {
            uint8_t b = event->data[i];
            if (xQueueSend(qbff, &b, pdMS_TO_TICKS(5000)) == pdFALSE)
            {
                ESP_LOGE(__func__, "Data ERROR");
                esp_restart();
            }
        }
    }

    return ESP_OK;
}

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
int8_t OTA_MQTT::wait(uint16_t time)
{
    int64_t th = esp_timer_get_time();
    while (esp_timer_get_time() - th < time*1000)
    {
        if (uxQueueMessagesWaiting(qbff) > 0) {return 1;}

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
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
void OTA_MQTT::decrypt(uint8_t *data, uint16_t size)
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

            mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, 16, _iv, aes_inp, aes_out);

            for (int8_t i = 0; i < 16; i++)
            {
                data[j+i] = aes_out[i];
            }
        }
    }
}

/**
 * @brief Process data received and manage OTA API.
 */
void OTA_MQTT::iterator()
{
    esp_err_t err;
    int64_t t1 = 0, t2 = 0;
    uint32_t total = 0;
    esp_ota_handle_t ota_handle = 0;
    const esp_partition_t *ota_partition = NULL;
    ota_partition = esp_ota_get_next_update_partition(NULL);

    for (uint8_t i = 0; i < 16; i++)
    {
        _iv[i] = _firstiv[i];
    }

    err = esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "OTA begin fail [0x%x]", err);
        return;
    }

    t1 = esp_timer_get_time()/1000;
    while (wait(3000))
    {
        uint16_t avl = uxQueueMessagesWaiting(qbff);
        if (_cry && (avl%16)) {vTaskDelay(1); continue;}

        total += avl;
        uint8_t data[512] = {0};
        for (uint16_t i = 0; i < avl; i++)
        {
            uint8_t b = 0;
            xQueueReceive(qbff, &b, 0);
            data[i] = b;
        }

        decrypt(data, avl);

        err = esp_ota_write(ota_handle, data, avl);
        if (err != ESP_OK)
        {
            ESP_LOGE(tag, "OTA write fail [0x%x]", err);
            t1 -= 3000; break;
        }

        if (total % 51200 <= 500) {ESP_LOGI(tag, "Downloaded %dB", total);}
        esp_task_wdt_reset();
    }
    t2 = (esp_timer_get_time()/1000)-3000;
    ESP_LOGI(tag, "Downloaded %dB in %dms", total, int32_t(t2-t1));

    err = esp_ota_end(ota_handle);
    if (err == ESP_OK)
    {
        err = esp_ota_set_boot_partition(ota_partition);
        if (err == ESP_OK)
        {
            ESP_LOGW(tag, "OTA OK, restarting...");
            esp_restart();
        }
        else
        {
            ESP_LOGE(tag, "OTA set boot partition fail [0x%x]", err);
        }
    }
    else
    {
        ESP_LOGE(tag, "OTA end fail [0x%x]", err);
        while (wait(3000))
        {
            vTaskDelay(pdMS_TO_TICKS(1));
            xQueueReset(qbff);
        }
    }
}

/**
 * @brief Subscribe to MQTT topic and download binary.
 * 
 * @param [*topic]: MQTT topic containing binary.
 */
void OTA_MQTT::download(const char *topic)
{
    esp_mqtt_client_start(client);

    int64_t th = esp_timer_get_time();
    while (esp_timer_get_time() - th < 5000*1000)
    {
        if (_connected) {break;}

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (_connected)
    {
        esp_mqtt_client_subscribe(client, topic, 0);
        ESP_LOGI(tag, "Downloading...");
        iterator(); //If OTA OK, library will restart().

        ESP_LOGE(tag, "Downloading FAIL");
        esp_mqtt_client_stop(client);
    }
    else
    {
        ESP_LOGE(tag, "MQTT not connected");
        esp_mqtt_client_stop(client);
    }
}

/**
 * @brief Enable AES-256 CBC crypto.
 * 
 * @attention IV is modified by MBEDTLS.
 * 
 * @attention Key must be 32 Chars.
 * @attention IV must be 16 Chars.
 * 
 * @param [*key]: AES key.
 * @param [*iv]: Initial IV.
 */
void OTA_MQTT::crypto(const char *key="", const char *iv="")
{
    _cry = 0;

    if (strlen(key) != 32) {ESP_LOGE(tag, "Key must be 32 Chars. Crypto OFF."); return;}
    if (strlen(iv)  != 16) {ESP_LOGE(tag, "IV must be 16 Chars. Crypto OFF."); return;}


    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, (uint8_t*)key, 256);
    
    for (uint8_t i = 0; i < 16; i++)
    {
        _firstiv[i] = iv[i];
    }

    _cry = 1;
}

/**
 * @brief Init OTA MQTT functions.
 * 
 * @param [*host]: MQTT host IP and PORT, need 'mqtt://' and ':port'. Eg: 'mqtt://192.168.0.100:1883'
 * @param [*user]: MQTT User.
 * @param [*pass]: MQTT Password.
 * @param [*id]:   Client ID. Default is 'ESP32_' + last 3B of MAC. Eg: 'ESP32_c0c549'
 */
void OTA_MQTT::init(const char *host, const char *user="", const char *pass="", const char *id="")
{
    esp_mqtt_client_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.uri = host;
    cfg.buffer_size = 1024;
    cfg.event_handle = mqtt_events;

    if (strlen(user)) {cfg.username  = user;}
    if (strlen(pass)) {cfg.password  = pass;}
    if (strlen(id))   {cfg.client_id = id;}
    
    client = esp_mqtt_client_init(&cfg);
    qbff = xQueueCreate(512, sizeof(uint8_t));
}

