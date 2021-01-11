#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#define LOG_TAG "btcink"

// eInk display init
#define PROGMEM
#include <gfxfont.h>
#include <Fonts/FreeSans9pt7b.h>
#include <gdeh0213b73.h>
EpdSpi io;
Gdeh0213b73 display(io);


extern "C" {
void app_main(void)
{
    ESP_LOGI(LOG_TAG, "Hello!");

    display.init(false);
    display.setRotation(1);
    display.setFont(&FreeSans9pt7b);
    display.fillRect(0, 0, 256, 16, EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(0, 10);
    display.println("Status bar goes here");
    display.setTextColor(EPD_BLACK);
    display.println("HELLO WORLD");
    display.update();
}
}
