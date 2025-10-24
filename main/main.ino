#include "user-led.h"
#include "user-screen.h"
#include "user-a02yyuh.h"

A02YYUW sensor(Serial1, 1, 0);  // RX=16, TX=17

void setup() {
  Serial.begin(115200);
  initLED();
  initScreen();

  sensor.begin(9600);
  showText("System Ready!");
  Serial.println("A02YYUW Distance Sensor Test (merged)");
}

void loop() {
  int distance = sensor.readDistance();

  if (distance != -1) {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    ledOn(0, 255, 0); // Green when valid reading
    String text = "Distance: " + String(distance) + " cm";
    showText(text.c_str());  } else {
    ledOn(255, 0, 0); // Red when no reading
    showText("No Data");
  }

  delay(500);
  ledOff();
  delay(100);
}
