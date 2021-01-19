#include <stdio.h>
#include <stddef.h>
#include <cstring>
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#ifndef PROGMEM
#define PROGMEM
#endif
#include <gfxfont.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>

#include "disp_mgr.h"

#define LOG_TAG "dispman"

DisplayManager::DisplayManager()
    : io()
    , display(io)
    , graph_idx(0)
{
    memset(points, 0, sizeof(points));
    display.init(false);
    display.setRotation(1);
    display.setFont(&FreeMono9pt7b);
    display.fillRect(0, 0, display.width(), 18, EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(0, 12);
    display.println("Status bar goes here");
    display.setTextColor(EPD_BLACK);
    display.println("Booting...");
    display.update();

    /* Battery ADC setup */
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
    adc1_config_width(ADC_WIDTH_12Bit);

}

DisplayManager::~DisplayManager()
{
}

void DisplayManager::update_status(int8_t rssi)
{
    display.fillRect(0, 0, display.width(), 18, EPD_BLACK);
    display.setFont(&FreeMono9pt7b);
    display.fillRect(0, 0, display.width(), 18, EPD_WHITE);
    display.setTextColor(EPD_BLACK);
    display.setCursor(0, 13);
    uint32_t voltage = (uint32_t)(adc1_get_raw(ADC1_CHANNEL_7) * 1.76f);
    char disp_str[64];
    snprintf(disp_str, sizeof(disp_str), "vBat:%4umV  WiFi:", voltage);
    for (int i = -80; i < rssi; i += 16) {
        strcat(disp_str, ")");
    }
    display.println(disp_str);
    display.updateWindow(0, 0, display.width(), 20);
}

void DisplayManager::update_graph(uint32_t value, uint32_t high, uint32_t low)
{
    points[graph_idx] = value;
    if (graph_idx < NUM_GRAPH_POINTS - 1) {
        graph_idx++;
    } else {
        graph_idx = 0;
    }
    display.fillRect(118, 18, 128, 80, EPD_BLACK);
    for (uint8_t i = graph_idx; i < NUM_GRAPH_POINTS + graph_idx; i++) {
        uint32_t scaled_point = 0;
        if (i < NUM_GRAPH_POINTS && points[i]) {
            scaled_point = ((points[i] - low) * 100 / (high - low));
        } else if (i >= NUM_GRAPH_POINTS && points[i - NUM_GRAPH_POINTS]) {
            scaled_point = ((points[i - NUM_GRAPH_POINTS] - low) * 100 / (high - low));
        } else {
            continue;
        }
        ESP_LOGD(LOG_TAG, "pixel x %u y %u", (118 + (i - graph_idx)), (18 + (100 - scaled_point)));
        display.drawPixel(118 + (i - graph_idx), 18 + (100 - scaled_point), EPD_WHITE);
    }
    display.updateWindow(118, 26, 128, 82);
}

void DisplayManager::update_value(uint32_t value)
{
    display.fillRect(0, 18, 118, 40, EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(0, 46);
    char disp_str[20];
    display.setFont(&FreeSansBold18pt7b);
    snprintf(disp_str, sizeof(disp_str), "$%d", value);
    display.println(disp_str);
    display.updateWindow(0, 26, 118, 30);
}

void DisplayManager::update_time(time_t newtime)
{
    display.fillRect(0, 72, display.width(), 50, EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(0, 90);
    char disp_str[72];
    display.setFont(&FreeSans9pt7b);
    snprintf(disp_str, sizeof(disp_str), "Updated at:\nUTC %s", ctime(&newtime));
    display.println(disp_str);
    display.updateWindow(0, 80, display.width(), 40);
}
