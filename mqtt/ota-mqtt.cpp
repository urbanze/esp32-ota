#include "ota-mqtt.h"

int16_t OTA_MQTT::received = 0;
uint8_t OTA_MQTT::data[256] = {0};

/**
 * @brief Process binary received from topic subscribed.
 */
void OTA_MQTT::mqtt_events(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    //ESP_LOGI(__func__, "%d", event_id);

    if (event_id == MQTT_EVENT_CONNECTED)
    {
        received = 1;
    }
    else if (event_id == MQTT_EVENT_DATA)
    {
        esp_mqtt_event_handle_t event = esp_mqtt_event_handle_t(event_data);

        memcpy(data, event->data, event->data_len);
        received = event->data_len;
        
        //Need to wait iterator() finish flash write to return this function.
        int64_t th = esp_timer_get_time();
        while (esp_timer_get_time() - th < 5000*1000)
        {
            if (!received) {return;}
            esp_task_wdt_reset();
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
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
        if (received > 0) {return 1;}
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1));
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

            mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, aes_inp, aes_out);

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


    err = esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "OTA begin fail [0x%x]", err);
        esp_mqtt_client_unsubscribe(client, "#"); return;
    }

    t1 = esp_timer_get_time()/1000;
    while (wait(3000))
    {
        uint16_t avl = received;
        if (_cry && (avl%16)) {vTaskDelay(1); continue;}

        total += avl;

        decrypt(data, avl);

        err = esp_ota_write(ota_handle, data, avl);
        if (err != ESP_OK)
        {
            ESP_LOGE(tag, "OTA write fail [0x%x]", err);
            esp_mqtt_client_unsubscribe(client, "#"); return;
        }

        esp_task_wdt_reset();
        received = 0;
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
            esp_mqtt_client_unsubscribe(client, "#"); return;
        }
    }
    else
    {
        ESP_LOGE(tag, "OTA end fail [0x%x]", err);
        esp_mqtt_client_unsubscribe(client, "#"); return;
    }
}

/**
 * @brief Subscribe to MQTT topic and download binary.
 * 
 * @attention This library unsubscribe ALL others topics before start. You need to reopen topics if OTA fail.
 * 
 * @param [*topic]: MQTT topic containing binary.
 */
void OTA_MQTT::download(const char *topic)
{
    memset(data, 0, sizeof(data));
    received = 0;

    wait(5000);//wait connect
    received = 0;
    esp_mqtt_client_subscribe(client, topic, 2);

    ESP_LOGI(tag, "Downloading...");
    iterator();
}

/**
 * @brief Init OTA MQTT functions.
 * 
 * If [key] string length == 0, crypto (AES 256 ECB) will be disabled.
 * 
 * @param [*host]: MQTT host IP and PORT, need 'mqtt://'. Eg: 'mqtt://192.168.0.100:1883'
 * @param [*user]: User.
 * @param [*pass]: Password.
 * @param [*id]: Client ID. Default is 'ESP32_' + last 3B of MAC. Eg: 'ESP32_c0c549'
 * @param [*key]: AES-256 ECB decrypt key (<=32 chars).
 */
void OTA_MQTT::init(const char *host, const char *user="", const char *pass="", const char *id="")
{
    char key[32] = {0};
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

    esp_mqtt_client_config_t mqtt_cfg;
    memset(&mqtt_cfg, 0, sizeof(mqtt_cfg));
    mqtt_cfg.uri = host;
    mqtt_cfg.buffer_size = 256;

    if (strlen(user)) {mqtt_cfg.username  = user;}
    if (strlen(pass)) {mqtt_cfg.password  = pass;}
    if (strlen(id))   {mqtt_cfg.client_id = id;}
    
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, esp_mqtt_event_id_t(ESP_EVENT_ANY_ID), mqtt_events, client);
    esp_mqtt_client_start(client);
}

