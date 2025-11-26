// ============================================
//  user-led.cpp
// ============================================
#include "user-led.h"

#define LED_PIN 8          // GPIO used for NeoPixel data line
#define NUM_LEDS 1         // Only one RGB LED is used
bool ledAutoMode = false;
// Create NeoPixel instance (GRB order, 800 kHz protocol)
Adafruit_NeoPixel led(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void initLED() {
  // Initialize NeoPixel driver and clear any previous state
  led.begin();
  led.show();   // Apply initial “off” state
}

void ledOn(uint8_t r, uint8_t g, uint8_t b) {
  // Set the single LED to the given RGB color
  led.setPixelColor(0, led.Color(r, g, b));
  led.show();   // Push color to LED
}

void ledOff() {
  // Turn off all pixels (color = 0)
  led.clear();
  led.show();   // Apply change
}

void setLedAutoMode(bool state) {
    ledAutoMode = state;
}

bool getLedAutoMode() {
    return ledAutoMode;
}