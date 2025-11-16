// ============================================
//  main.ino
// ============================================

#include "user-led.h"      // LED control (manual/auto modes + RGB output)
#include "user-screen.h"   // OLED display + button handling
#include "user-wifi.h"     // Wi-Fi manager + web server update functions
#include "A02YYUW.h"       // Ultrasonic distance sensor driver

HardwareSerial mySerial(2);           // Use UART2 for the A02YYUW sensor
A02YYUW sensor(mySerial, 4, 5);       // RX=4, TX=5 (sensor uses serial)

const float FULL_DISTANCE  = 30.0;    // Distance when tank is considered 100% full
const float EMPTY_DISTANCE = 200.0;   // Distance when tank is considered empty

void setup() {
  Serial.begin(115200);     // Debug output
  sensor.begin(9600);       // A02YYUW baud rate

  initLED();                // Prepare RGB LED / WS2812
  initScreen();             // Initialize OLED and UI
  initWiFi();               // Start Wi-Fi AP/STA + web server

  showText("System Ready!"); // Show startup message
}

void loop() {
  handleWiFi();             // Process Wi-Fi events + web requests
  handleScreenButton();     // Check button input for screen navigation

  float distance = sensor.getDistance();   // Read ultrasonic sensor value (cm)

  if (distance > 0) {       // Valid reading
    Serial.printf("Distance: %.1f cm\n", distance);

    // Convert raw distance to percentage (0–100%)
    float percent = (EMPTY_DISTANCE - distance) /
                    (EMPTY_DISTANCE - FULL_DISTANCE) * 100;
    percent = constrain(percent, 0, 100);

    updateSensorData(distance, percent);   // Expose values to web UI

    showWaterLevel(distance,               // Draw tank level on screen
                    FULL_DISTANCE,
                    EMPTY_DISTANCE);

    // Auto LED color logic (only if auto mode is enabled)
    if (isLedAutoMode()) {
      if (percent < 20)       ledOn(255, 0, 0);   // Low → Red
      else if (percent < 70)  ledOn(255, 255, 0); // Medium → Yellow
      else                    ledOn(0, 255, 0);   // High → Green
    }

  } else {
    // Sensor returned an invalid value
    showText("No reading");
    
    if (isLedAutoMode())
      ledOn(255, 0, 0);       // Show alert state

    updateSensorData(0, 0);   // Push "invalid" state to the web UI
  }

  delay(500);                 // Fixed sampling interval
}
