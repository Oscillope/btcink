#include <stdio.h>
#include "esp_err.h"
#include "esp_interface.h"
#include "esp_wifi_types.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "cJSON.h"
#include "nvs_flash.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"

#define LOG_TAG "btcink"
#define SD_MOUNT "/sdcard"

extern "C" {
    void app_main(void);
}

// eInk display init
#define PROGMEM
#include <gfxfont.h>
#include <Fonts/FreeSans9pt7b.h>
#include <gdeh0213b73.h>
EpdSpi io;
Gdeh0213b73 display(io);

void read_config(void)
{
    wifi_config_t wifi_config = {
        .sta = {
            .threshold = {
                .authmode = WIFI_AUTH_WPA_PSK,
            },

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    FILE* f = fopen(SD_MOUNT"/config.txt", "r");
    if (!f) {
        ESP_LOGE(LOG_TAG, "Failed to open config.txt (error %d)", errno);
        return;
    }
    char* config = (char*)calloc(sizeof(char), 1024);
    if (!config) {
        ESP_LOGE(LOG_TAG, "Failed to allocate config memory");
    }
    size_t bytes = fread(config, sizeof(char), 1024, f);
    cJSON* root = cJSON_ParseWithLength(config, bytes);

    char* ssid = cJSON_GetObjectItem(root, "ssid")->valuestring;
    char* psk = cJSON_GetObjectItem(root, "psk")->valuestring;
    ESP_LOGI(LOG_TAG, "Set SSID to %s", ssid);
    strncpy((char*)(wifi_config.sta.ssid), ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)(wifi_config.sta.password), psk, sizeof(wifi_config.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    cJSON_Delete(root);
    free(config);
    fclose(f);
}

void app_main(void)
{
    int ret = 0;
    ESP_LOGI(LOG_TAG, "Hello!");
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // Initialize WiFi
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));

    display.init(false);
    display.setRotation(1);
    display.setFont(&FreeSans9pt7b);
    display.fillRect(0, 0, 256, 16, EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(0, 10);
    display.println("Status bar goes here");
    display.setTextColor(EPD_BLACK);
    display.println("HELLO WORLD");
    //display.update();

    // Read SD card for wifi config
    sdmmc_host_t sd_host = SDMMC_HOST_DEFAULT();
    sd_host.slot = SDMMC_HOST_SLOT_1;
    sdmmc_slot_config_t sd_slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    sdmmc_card_t* card;
    sd_slot_config.width = 1;
    sd_slot_config.flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = { false, 8, 0 };
    ret = esp_vfs_fat_sdmmc_mount(SD_MOUNT, &sd_host, &sd_slot_config, &mount_config, &card);
    if (ret == ESP_OK) {
        sdmmc_card_print_info(stdout, card);
        ESP_LOGI(LOG_TAG, "SD mounted! Reading wifi config.");
        read_config();
    } else {
        ESP_LOGI(LOG_TAG, "No SD card detected");
    }

    esp_wifi_start();
    esp_wifi_connect();

}
