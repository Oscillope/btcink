#ifndef DISP_MGR_H
#define DISP_MGR_H

// eInk display init
#define PROGMEM
#include <gdeh0213b73.h>

#define NUM_GRAPH_POINTS 128

class DisplayManager {
public:
    DisplayManager(void);
    ~DisplayManager(void);

    void update_status(int8_t rssi);
    void update_graph(uint32_t value, uint32_t high, uint32_t low);
    void update_value(uint32_t value);
    void update_time(time_t newtime);

private:
    EpdSpi io;
    Gdeh0213b73 display;
    uint32_t points[NUM_GRAPH_POINTS];
    uint8_t graph_idx;
};

#endif // DISP_MGR_H
