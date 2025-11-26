// ============================================
//  user-led.h
// ============================================

#pragma once
#include <Adafruit_NeoPixel.h>
extern bool ledAutoMode;
void initLED();
void ledOn(uint8_t r, uint8_t g, uint8_t b);
void ledOff();
void setLedAutoMode(bool state);
bool getLedAutoMode();
