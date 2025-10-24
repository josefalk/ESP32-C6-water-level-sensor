#pragma once
#include <Adafruit_NeoPixel.h>

#define LED_PIN 8
#define NUM_LEDS 1

Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void initLED() {
  pixels.begin();
  pixels.clear();
  pixels.show();
}

void ledOn(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

void ledOff() {
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();
}
