#ifndef DATA_GRABBER_H
#define DATA_GRABBER_H
#include "esp_http_client.h"
#include "chunked_response.h"

class DataGrabber {
public:
    DataGrabber(void);
    ~DataGrabber(void);

    int update(void);
    uint32_t get_price(void) { return btc_price; }
    uint32_t get_high(void) { return btc_high; }
    uint32_t get_low(void) { return btc_low; }
    time_t get_timestamp(void) { return btc_timestamp; }

private:
    static esp_err_t event_handler(esp_http_client_event_t *evt);
    const esp_http_client_config_t config;
    esp_http_client_handle_t client;
    ChunkedResponse response;
    uint32_t btc_price;
    uint32_t btc_high;
    uint32_t btc_low;
    time_t btc_timestamp;
};

#endif //DATA_GRABBER_H
