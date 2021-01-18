#include <stdlib.h>
#include "esp_log.h"
#include "esp_tls.h"
#include "cJSON.h"

#define LOG_TAG "grabber"

#include "data_grabber.h"

extern const char cloudflare_root_cert_pem_start[] asm("_binary_cloudflare_root_cert_pem_start");
extern const char cloudflare_root_cert_pem_end[]   asm("_binary_cloudflare_root_cert_pem_end");

DataGrabber::DataGrabber()
    : config((esp_http_client_config_t)
             { .url = "https://api.coingecko.com/api/v3/coins/markets?vs_currency=usd&ids=bitcoin",
               .cert_pem = cloudflare_root_cert_pem_start,
               .event_handler = event_handler,
               .user_data = &response })
    , response()
    , btc_price(0)
    , btc_timestamp("")
{
    client = esp_http_client_init(&config);
}

DataGrabber::~DataGrabber()
{
    esp_http_client_cleanup(client);
}

esp_err_t DataGrabber::event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(LOG_TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(LOG_TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(LOG_TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(LOG_TAG, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(LOG_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                printf("%.*s", evt->data_len, (char*)evt->data);
            } else {
                ChunkedResponse* resp = (ChunkedResponse*)evt->user_data;
                resp->write_chunk(evt->data, evt->data_len);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(LOG_TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(LOG_TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

int DataGrabber::update()
{
    response.clear();
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(LOG_TAG, "Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
        if (esp_http_client_is_chunked_response(client)) {
            // The API returns the data in [], trim those out
            cJSON* root = cJSON_ParseWithLength(response.get_data() + 1, response.get_len() - 1);
            if (root) {
                btc_price = cJSON_GetObjectItem(root, "current_price")->valueint;
                strncpy(btc_timestamp, cJSON_GetObjectItem(root, "last_updated")->valuestring, sizeof(btc_timestamp));
                ESP_LOGI(LOG_TAG, "current price: %d updated at %s", btc_price, btc_timestamp);
                cJSON_Delete(root);
            } else {
                ESP_LOGE(LOG_TAG, "Failed to parse JSON");
            }
        }
    }
    return err;
}
