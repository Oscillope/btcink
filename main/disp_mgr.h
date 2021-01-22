#ifndef DISP_MGR_H
#define DISP_MGR_H

#ifndef PROGMEM
#define PROGMEM
#endif
#include <gdeh0213b73.h>

#include "esp_adc_cal.h"

#define NUM_GRAPH_POINTS 128

class DisplayManager {
public:
    DisplayManager(void);
    ~DisplayManager(void);

    void full_refresh(void);

    void update_status(int8_t rssi);
    void update_value(uint32_t value, uint32_t high, uint32_t low);
    void update_time(time_t newtime);

private:
    void update_graph(uint32_t value, uint32_t high, uint32_t low);

    EpdSpi io;
    Gdeh0213b73 display;
    esp_adc_cal_characteristics_t adc_chars;
    uint32_t v_bat_last;
    bool charging;
    uint32_t points[NUM_GRAPH_POINTS];
    uint8_t graph_idx;
};

#endif // DISP_MGR_H
