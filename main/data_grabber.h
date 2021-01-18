#ifndef DATA_GRABBER_H
#define DATA_GRABBER_H
#include "esp_http_client.h"
#include "chunked_response.h"

class DataGrabber {
public:
    DataGrabber(void);
    ~DataGrabber(void);

    int update(void);
    int get_price(void) { return btc_price; }
    char* get_timestamp(void) { return btc_timestamp; }

private:
    static esp_err_t event_handler(esp_http_client_event_t *evt);
    const esp_http_client_config_t config;
    esp_http_client_handle_t client;
    ChunkedResponse response;
    int btc_price;
    char btc_timestamp[32];
};

#endif //DATA_GRABBER_H
