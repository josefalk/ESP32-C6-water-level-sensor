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
bool isLedAutoMode();

// Calibration functions
float getFullDistance();
float getEmptyDistance();
void setCalibration(float full, float empty);

// Tank dimension functions
String getTankShape();
float getTankLength();
float getTankWidth();
float getTankDiameter();
void setTankDimensions(String shape, float length, float width, float diameter);

// Volume calculation
float calculateWaterVolume(float currentDistance);

#endif