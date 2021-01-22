#include <stdio.h>
#include <stddef.h>
#include <cstring>
#include "driver/adc.h"
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

#define DEFAULT_VREF 1100

DisplayManager::DisplayManager()
    : io()
    , display(io)
    , charging(false)
    , graph_idx(0)
{
    memset(points, 0, sizeof(points));
    display.init(false);
    display.setRotation(1);
    display.fillScreen(EPD_WHITE);
    display.update();
    display.setFont(&FreeMono9pt7b);
    display.fillRect(0, 0, display.width(), 18, EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(0, 12);
    display.println("Status bar goes here");
    display.setTextColor(EPD_BLACK);
    display.println("Booting...");
    display.update();

    /* Battery ADC setup */
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
    adc1_config_width(ADC_WIDTH_12Bit);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_12Bit, DEFAULT_VREF, &adc_chars);
    v_bat_last = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_7), &adc_chars) * 2;

}

DisplayManager::~DisplayManager()
{
}

void DisplayManager::full_refresh()
{
    display.update();
}

void DisplayManager::update_status(int8_t rssi)
{
    display.fillRect(0, 0, display.width(), 18, EPD_BLACK);
    display.setFont(&FreeMono9pt7b);
    display.fillRect(0, 0, display.width(), 18, EPD_WHITE);
    display.setTextColor(EPD_BLACK);
    display.setCursor(0, 13);
    uint32_t v_bat = (uint32_t)(esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_7), &adc_chars) * 2);
    uint32_t v_chg = (uint32_t)(esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_5), &adc_chars) * 2);
    ESP_LOGI(LOG_TAG, "vBat: %u last: %u vChg: %u", v_bat, v_bat_last, v_chg);
    char disp_str[64];
    const char* bat_str;
    if (v_chg > 4300 && v_bat > v_bat_last) { // We're charging
        charging = true;
        v_bat_last = v_bat;
    } else if (v_chg <= v_bat && v_bat < v_bat_last) {
        charging = false;
        v_bat_last = v_bat;
    }
    switch (v_bat_last) {
    case 4150 ... 4400:
        bat_str = "+{||||}-";
        break;
    case 3900 ... 4149:
        bat_str = (charging ? "+{<<<<}-" : "+{||||}-");
        break;
    case 3600 ... 3899:
        bat_str = (charging ? "+{ <<<}-" : "+{ |||}-");
        break;
    case 3300 ... 3599:
        bat_str = (charging ? "+{ <<<}-" : "+{  ||}-");
        break;
    case 3000 ... 3299:
        bat_str = (charging ? "+{  <<}-" : "+{   |}-");
        break;
    case 2700 ... 2999:
        bat_str = (charging ? "+{   <}-" : "+{    }-");
        break;
    default:
        bat_str = "+[????]-";
        break;
    }
    snprintf(disp_str, sizeof(disp_str), "%s    WiFi:", bat_str);
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
    display.fillRect(118, 18, 132, 80, EPD_BLACK);
    for (uint8_t i = graph_idx; i < NUM_GRAPH_POINTS + graph_idx; i++) {
        uint32_t scaled_point = 0;
        if (i < NUM_GRAPH_POINTS && points[i]) {
            scaled_point = ((points[i] - low) * 80 / (high - low));
        } else if (i >= NUM_GRAPH_POINTS && points[i - NUM_GRAPH_POINTS]) {
            scaled_point = ((points[i - NUM_GRAPH_POINTS] - low) * 80 / (high - low));
        } else {
            continue;
        }
        ESP_LOGD(LOG_TAG, "pixel x %u y %u", (118 + (i - graph_idx)), (18 + (80 - scaled_point)));
        // Make the line 2 pixels thick so it's more visible
        display.drawPixel(118 + (i - graph_idx), 17 + (80 - scaled_point), EPD_WHITE);
        display.drawPixel(118 + (i - graph_idx), 18 + (80 - scaled_point), EPD_WHITE);
    }
    display.updateWindow(118, 26, 132, 82);
}

void DisplayManager::update_value(uint32_t value, uint32_t high, uint32_t low)
{
    display.fillRect(0, 18, 118, 80, EPD_BLACK);
    update_graph(value, high, low);
    display.setTextColor(EPD_WHITE);
    display.setCursor(0, 46);
    char disp_str[20];
    display.setFont(&FreeSansBold18pt7b);
    snprintf(disp_str, sizeof(disp_str), "$%d", value);
    display.println(disp_str);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(0, 70);
    snprintf(disp_str, sizeof(disp_str), "High: $%d", high);
    display.println(disp_str);
    snprintf(disp_str, sizeof(disp_str), "Low: $%d", low);
    display.println(disp_str);
    display.updateWindow(0, 26, 118, 82);
}

void DisplayManager::update_time(time_t newtime)
{
    display.fillRect(0, 102, display.width(), 24, EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(0, 120);
    char disp_str[72];
    display.setFont(&FreeSans9pt7b);
    snprintf(disp_str, sizeof(disp_str), "UTC %s", ctime(&newtime));
    display.println(disp_str);
    display.updateWindow(0, 102, display.width(), 20);
}
