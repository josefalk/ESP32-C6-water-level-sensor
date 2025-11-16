// ============================================
// user-screen.h
// ============================================
#ifndef USER_SCREEN_H
#define USER_SCREEN_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void initScreen();
void toggleScreen();
void setScreenState(bool state);  // Add this line
bool getScreenState();            // Add this line
void showText(const String &text);
void handleScreenButton();
void showWaterLevel(float distance, float fullDistance, float emptyDistance);

#endif