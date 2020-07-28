#include "ota-sdmmc.h"



/**
 * @brief Decrypt data received (if enabled)
 * 
 * This function replace all old array data (crypted) with decrypted bytes.
 * 
 * @param [*data]: Crypted array data.
 * @param [size]: Size of array.
 */
void OTA_SDMMC::decrypt(uint8_t *data, uint16_t size)
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
 * @brief Read file and manage OTA API.
 */
void OTA_SDMMC::iterator()
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


    fseek(f, 0, SEEK_END);
    uint32_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    ESP_LOGI(tag, "File size = %d", fsize);


    t1 = esp_timer_get_time()/1000;
    while (1)
    {
        uint8_t data[1024] = {0};
        int16_t avl = fread(data, 1, sizeof(data), f);
        total += avl;


        decrypt(data, avl);

        err = esp_ota_write(ota_handle, data, avl);
        if (err != ESP_OK)
        {
            ESP_LOGE(tag, "OTA write fail [0x%x]", err); break;
        }

        if (total >= fsize)
        {
            ESP_LOGI(tag, "Read end"); break;
        }


        if (total % 51200 <= 500) {ESP_LOGI(tag, "Downloaded %dB", total);}
        esp_task_wdt_reset();
    }
    t2 = (esp_timer_get_time()/1000);
    ESP_LOGI(tag, "Downloaded %dB in %dms", total, int32_t(t2-t1));
    fclose(f);

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
    }
}

/**
 * @brief Download file from SD card.
 */
void OTA_SDMMC::download()
{
    iterator();
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
void OTA_SDMMC::crypto(const char *key="", const char *iv="")
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
 * @brief Init SD Card and detect file
 * 
 * @param [*file_path]: File path.
 * @param [*base_path]: Base path of sdcard. Default is '/sdcard'
 * 
 * Eg: if file [esp32.bin] is at root of SD, [file_path] = esp32.bin
 * Eg: if file [ota.bin] is at folder [ota/esp32/], [file_path] = ota/esp32/ota.bin
 * 
 * @return 0: Fail.
 * @return 1: Sucess.
 */
int8_t OTA_SDMMC::init(const char *file_path, const char *base_path="/sdcard")
{
    esp_err_t err;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot = SDMMC_SLOT_CONFIG_DEFAULT();

    gpio_set_pull_mode(GPIO_NUM_15, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_NUM_2,  GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_NUM_4,  GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_NUM_12, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_NUM_13, GPIO_PULLUP_ONLY);

    esp_vfs_fat_sdmmc_mount_config_t mount;
    mount.format_if_mount_failed = 0;
    mount.max_files = 1;
    mount.allocation_unit_size = 512;

    sdmmc_card_t *sd;
    err = esp_vfs_fat_sdmmc_mount(base_path, &host, &slot, &mount, &sd);
    if (err != ESP_OK)
    {
        if (err == ESP_FAIL)
        {
            ESP_LOGE(tag, "Fail to mount");
        }
        else
        {
            ESP_LOGE(tag, "Fail to init SDCARD (0x%x)", err);
        }
        
        return 0;
    }

    char full_path[64] = {0};
    snprintf(full_path, 64, "%s/%s", base_path, file_path);

    f = fopen(full_path, "rb");
    if (f != NULL)
    {
        return 1;
    }
    else
    {
        ESP_LOGE(tag, "Fail to open file [%s] (%d)", full_path, errno);
    }

    return 0;
}

