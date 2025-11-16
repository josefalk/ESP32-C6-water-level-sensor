// ============================================
// user-wifi.h
// ============================================
#ifndef USER_WIFI_H
#define USER_WIFI_H

#include <Arduino.h>

void initWiFi();
void handleWiFi();
bool isWiFiConnected();
String getWiFiIP();
void updateSensorData(float distance, float percent);
bool isLedAutoMode();  // Check if LED is in automatic mode

#endif
