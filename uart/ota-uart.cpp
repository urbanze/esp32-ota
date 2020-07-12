#include "ota-uart.h"

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
int8_t OTA_UART::wait(uint16_t time)
{
    int64_t th = esp_timer_get_time();
    size_t avl = 0;

    while (esp_timer_get_time() - th < time*1000)
    {
        esp_task_wdt_reset();
        uart_get_buffered_data_len(_uart, &avl);
        if (avl > 0) {return 1;}
    }

    return 0;
}

/**
 * @brief Send one (confirm packet), indicating: ESP32 is ready to receive new packet.
 */
void OTA_UART::confirm()
{
    uart_write_bytes(_uart, "\r", 1);
    uart_write_bytes(_uart, "\n", 1);
    //uart_wait_tx_done(_uart, pdMS_TO_TICKS(25));
}

/**
 * @brief Decrypt data received (if enabled)
 * 
 * This function replace all old array data (crypted) with decrypted bytes.
 * 
 * @param [*data]: Crypted array data.
 * @param [size]: Size of array.
 */
void OTA_UART::decrypt(uint8_t *data, uint16_t size)
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
 * @brief Read bytes sent to ESP32 and write in OTA partition.
 */
void OTA_UART::download()
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
    while (wait(2000))
    {
        confirm();

        size_t avl = 0;
        uart_get_buffered_data_len(_uart, &avl);

        if (avl > 1024) {avl = 1024;}
        if (_cry && (avl%16)) {vTaskDelay(1); continue;}

        total += avl;
        uint8_t data[1024] = {0};
        uart_read_bytes(_uart, data, avl, 0);

        decrypt(data, avl);

        err = esp_ota_write(ota_handle, data, avl);
        if (err != ESP_OK)
        {
            ESP_LOGE(tag, "OTA write fail [%x]", err);
            return;
        }

        esp_task_wdt_reset();
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
            esp_restart();
        }
        else
        {
            ESP_LOGE(tag, "OTA set boot partition fail [0x%x]", err);
            return;
        }
    }
    else
    {
        ESP_LOGE(tag, "OTA end fail [0x%x]", err);
        return;
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
void OTA_UART::crypto(const char *key="", const char *iv="")
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
 * @brief Init UART to listening OTA upadtes.
 * 
 * If string (key) length == 0, crypto (AES 256 ECB) will be disabled.
 * 
 * @param [uart]: UART port number. (eg: UART_NUM_1)
 * @param [baud]: UART baud rate.
 * @param [pin_tx]: GPIO TX pin.
 * @param [pin_rx]: GPIO RX pin.
 * @param [*key]: AES-256 ECB decrypt key (<=32 chars).
 */
void OTA_UART::init(uart_port_t uart, uint32_t baud, int8_t pin_tx, int8_t pin_rx)
{
    uart_config_t cfg;
    cfg.baud_rate = baud;
	cfg.data_bits = UART_DATA_8_BITS;
	cfg.parity = UART_PARITY_DISABLE;
	cfg.stop_bits = UART_STOP_BITS_1;
	cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
	cfg.rx_flow_ctrl_thresh = 0;
	cfg.use_ref_tick = 0;
    _uart = uart;

	uart_param_config(uart, &cfg);
	uart_set_pin(uart, gpio_num_t(pin_tx), gpio_num_t(pin_rx), UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(uart, 1024, 0, 0, NULL, 0);
}

