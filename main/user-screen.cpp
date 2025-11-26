// ============================================
// user-screen.cpp 
// Manages the OLED display and screen toggle button
// ============================================

#include "user-screen.h"
#include "user-wifi.h"    // Needed for WiFi status display
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1   // No reset pin
#define I2C_SDA 3          // OLED SDA pin
#define I2C_SCL 2          // OLED SCL pin
#define OLED_ADDR 0x3C     // Common SSD1306 I2C address
#define BUTTON_PIN 18      // GPIO for screen ON/OFF button

// Create OLED driver instance
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool screenAvailable = false;  // True only if OLED detected on I2C
bool screenOn = true;          // Current display power state
bool lastButtonState = HIGH;   // Used for button edge detection
// Global variable for scrolling
int scrollX = SCREEN_WIDTH; // Start just off the right edge


void initScreen() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Button uses internal pull-up

  Wire.begin(I2C_SDA, I2C_SCL);

  // Check if an OLED display responds on the I2C bus
  Wire.beginTransmission(OLED_ADDR);
  if (Wire.endTransmission() == 0) {
    screenAvailable = true;

    // Initialize SSD1306 driver
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
      Serial.println("SSD1306 init failed!");
      screenAvailable = false;
      return;
    }

    // Basic startup message
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Screen Ready");
    display.display();
    screenOn = true;

    Serial.println("OLED connected successfully!");
  } else {
    // Allow system to run even with no screen attached
    Serial.println("⚠️ No OLED detected on I2C — continuing without display.");
    screenAvailable = false;
  }
}

// Toggle ON/OFF state of OLED (button or web)
void toggleScreen() {
  if (!screenAvailable) return;

  screenOn = !screenOn;

  if (screenOn) {
    display.ssd1306_command(SSD1306_DISPLAYON);
    Serial.println("🟢 Screen turned ON");
  } else {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    Serial.println("⚫ Screen turned OFF");
  }
}

// Print simple text (clears previous content)
void showText(const String &text) {
  if (!screenAvailable || !screenOn) return;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(text);
  display.display();
}

// Handle physical button that toggles display power
void handleScreenButton() {
  static unsigned long lastPressTime = 0;
  bool currentState = digitalRead(BUTTON_PIN);

  // Detect falling edge (HIGH -> LOW)
  if (lastButtonState == HIGH && currentState == LOW) {
    unsigned long now = millis();

    // Debounce + ignore rapid presses
    if (now - lastPressTime > 500) {
      toggleScreen();
      lastPressTime = now;
    }
  }

  lastButtonState = currentState;
}

// Draw water level bar + distance + WiFi info
void showWaterLevel(float distance, float fullDistance, float emptyDistance) {
  if (!screenAvailable || !screenOn) return;

  // Convert raw distance to percentage (clamped)
  float percent = (emptyDistance - distance) /
                  (emptyDistance - fullDistance) * 100;
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;

  // Bar dimensions
  int x = 0;
  int y = 10;
  int width = 125;
  int height = 12;

  // Width of filled level bar
  int fillWidth = map(percent, 0, 100, 0, width);

  display.clearDisplay();

  // Draw tank bar outline + fill
  display.drawRect(x, y, width, height, SSD1306_WHITE);
  display.fillRect(x + 1, y + 1, fillWidth - 2, height - 2, SSD1306_WHITE);

  // Show distance line
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Distance: ");
  display.print(distance, 1);
  display.println(" cm");

  // Show percentage line
  display.setCursor(0, 25);
  display.print("Level: ");
  display.print(percent);
  display.print("% ");

  // Append WiFi status (IP last octet or AP)
  if (isWiFiConnected()) {
    String ip = getWiFiIP();
    int lastDot = ip.lastIndexOf('.');
    if (lastDot != -1) {
      String lastOctet = ip.substring(lastDot + 1);
      display.print("(");
      display.print(lastOctet);
      display.print(")");
    }
  } else {
    display.print("(AP)");  // AP mode indicator
  }

  display.display();
}

// Set screen ON/OFF from web interface
void setScreenState(bool state) {
  if (!screenAvailable) return;
  if (screenOn == state) return;  // Avoid redundant I2C commands

  screenOn = state;

  if (screenOn) {
    display.ssd1306_command(SSD1306_DISPLAYON);
    Serial.println("🟢 Screen turned ON (via web)");
  } else {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    Serial.println("⚫ Screen turned OFF (via web)");
  }
}

// Query current screen state (OFF also if no OLED attached)
bool getScreenState() {
  return screenOn && screenAvailable;
}
