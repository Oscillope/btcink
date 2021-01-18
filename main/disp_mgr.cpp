#include <stdio.h>
#include <stddef.h>
#include <cstring>
#ifndef PROGMEM
#define PROGMEM
#endif
#include <gfxfont.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>

#include "disp_mgr.h"

DisplayManager::DisplayManager()
    : io()
    , display(io)
{
    display.init(false);
    display.setRotation(1);
    display.setFont(&FreeSans9pt7b);
    display.fillRect(0, 0, display.width(), 18, EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(0, 12);
    display.println("Status bar goes here");
    display.setTextColor(EPD_BLACK);
    display.println("Booting...");
    display.update();
}

DisplayManager::~DisplayManager()
{
}

void DisplayManager::update_status(int8_t rssi)
{
    display.fillRect(0, 0, display.width(), 18, EPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.fillRect(0, 0, display.width(), 18, EPD_WHITE);
    display.setTextColor(EPD_BLACK);
    display.setCursor(0, 13);
    char disp_str[20];
    snprintf(disp_str, sizeof(disp_str), "RSSI: %d", rssi);
    display.println(disp_str);
    display.updateWindow(0, 0, display.width(), 20);
}

void DisplayManager::update_value(int value)
{
    display.fillRect(0, 18, 128, 40, EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(0, 46);
    char disp_str[20];
    display.setFont(&FreeSansBold18pt7b);
    snprintf(disp_str, sizeof(disp_str), "$%d", value);
    display.println(disp_str);
    display.updateWindow(0, 26, 128, 30);
}

void DisplayManager::update_time(char* newtime)
{
    display.fillRect(0, 72, display.width(), 50, EPD_BLACK);
    display.setTextColor(EPD_WHITE);
    display.setCursor(0, 90);
    char disp_str[64];
    display.setFont(&FreeSans9pt7b);
    snprintf(disp_str, sizeof(disp_str), "Updated at:\n%s", newtime);
    display.println(disp_str);
    display.updateWindow(0, 80, display.width(), 40);
}
