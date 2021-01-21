#include <stdio.h>
#include <cstring>
#include "esp_err.h"
#include "esp_interface.h"
#include "esp_wifi_types.h"
#include "freertos/portmacro.h"
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
#include "data_grabber.h"
#include "disp_mgr.h"

#define LOG_TAG "btcink"
#define SD_MOUNT "/sdcard"

extern "C" {
    void app_main(void);
}

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

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

static void wifi_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(LOG_TAG, "Connect failed, retrying");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(LOG_TAG, "Got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void app_main(void)
{
    esp_err_t ret = 0;
    ESP_LOGI(LOG_TAG, "Hello!");
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // Initialize WiFi
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));

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
    s_wifi_event_group = xEventGroupCreate();
    DataGrabber* grabber = new DataGrabber();
    DisplayManager* dispman = new DisplayManager();

    esp_wifi_start();
    esp_wifi_connect();
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_handler,
                                                        NULL,
                                                        NULL));

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    while (1) {
        wifi_ap_record_t ap_info;
        esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
        if (err == ESP_OK) {
            dispman->update_status(ap_info.rssi);
        }
        if (grabber->update() == ESP_OK) {
            dispman->update_time(grabber->get_timestamp());
            dispman->update_value(grabber->get_price(), grabber->get_high(), grabber->get_low());
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
