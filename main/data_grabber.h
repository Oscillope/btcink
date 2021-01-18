#ifndef DATA_GRABBER_H
#define DATA_GRABBER_H
#include "esp_http_client.h"

class DataGrabber {
public:
    DataGrabber(void);
    ~DataGrabber(void);

    int update(void);

private:
    static esp_err_t event_handler(esp_http_client_event_t *evt);
    const esp_http_client_config_t config;
    esp_http_client_handle_t client;
};

#endif //DATA_GRABBER_H
