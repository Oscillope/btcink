#ifndef DISP_MGR_H
#define DISP_MGR_H

// eInk display init
#define PROGMEM
#include <gdeh0213b73.h>

class DisplayManager {
public:
    DisplayManager(void);
    ~DisplayManager(void);

    void update_status(int8_t rssi);
    void update_value(int value);
    void update_time(char* newtime);

private:
    EpdSpi io;
    Gdeh0213b73 display;
};

#endif // DISP_MGR_H
