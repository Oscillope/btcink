#include <stdlib.h>
#include "esp_log.h"
#include "esp_tls.h"
#include "cJSON.h"

#define LOG_TAG "grabber"

#include "data_grabber.h"

extern const char digicert_root_cert_pem_start[] asm("_binary_digicert_root_cert_pem_start");
extern const char digicert_root_cert_pem_end[]   asm("_binary_digicert_root_cert_pem_end");

DataGrabber::DataGrabber()
    : config((esp_http_client_config_t)
             { .url = "https://www.bitstamp.net/api/v2/ticker/btcusd/",
               .cert_pem = digicert_root_cert_pem_start,
               .event_handler = event_handler,
               .user_data = &response })
    , response()
    , btc_price(0)
    , btc_high(0)
    , btc_low(0)
    , btc_timestamp(0)
{
    client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Connection", "close");
}

DataGrabber::~DataGrabber()
{
    esp_http_client_cleanup(client);
}

esp_err_t DataGrabber::event_handler(esp_http_client_event_t *evt)
{
    ChunkedResponse* resp = (ChunkedResponse*)evt->user_data;
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(LOG_TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(LOG_TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(LOG_TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(LOG_TAG, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(LOG_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            resp->write_chunk(evt->data, evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(LOG_TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(LOG_TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

int DataGrabber::update()
{
    response.clear();
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGD(LOG_TAG, "Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
        cJSON* root = cJSON_ParseWithLength(response.get_data(), response.get_len());
        if (root) {
            btc_price = atoi(cJSON_GetObjectItem(root, "last")->valuestring);
            btc_high = atoi(cJSON_GetObjectItem(root, "high")->valuestring);
            btc_low = atoi(cJSON_GetObjectItem(root, "low")->valuestring);
            btc_timestamp = atol(cJSON_GetObjectItem(root, "timestamp")->valuestring);
            ESP_LOGI(LOG_TAG, "current price: %u updated at %lu", btc_price, btc_timestamp);
            cJSON_Delete(root);
        } else {
            ESP_LOGE(LOG_TAG, "Failed to parse JSON");
        }
    }
    esp_http_client_close(client);
    return err;
}
