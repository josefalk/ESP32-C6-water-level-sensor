// ============================================
//  A02YYUW.cpp
// ============================================

#include "A02YYUW.h"

// Constructor: store reference to HardwareSerial and pin assignments
A02YYUW::A02YYUW(HardwareSerial &serial, int rxPin, int txPin)
  : serial(serial), rxPin(rxPin), txPin(txPin) {}

void A02YYUW::begin(unsigned long baud) {
  // Initialize UART with given baud rate and 8N1 format
  serial.begin(baud, SERIAL_8N1, rxPin, txPin);
}

float A02YYUW::getDistance() {
  // The A02YYUW requires sending 0x55 as a trigger byte
  serial.write(0x55);
  delay(50);  // Sensor response time

  // Sensor always replies with 4 bytes: FF | high | low | checksum
  if (serial.available() >= 4) {

    uint8_t data[4];

    // First byte must be frame header 0xFF
    if (serial.peek() == 0xFF) {

      // Read full 4-byte frame
      for (int i = 0; i < 4; i++)
        data[i] = serial.read();

      // Validate checksum: (byte0 + byte1 + byte2) & 0xFF == byte3
      int sum = (data[0] + data[1] + data[2]) & 0xFF;
      if (sum == data[3]) {

        // Combine high + low byte into distance (mm)
        int dist = (data[1] << 8) + data[2];
        return dist / 10.0;  // Convert mm → cm
      }

    } else {
      // If header doesn't match, discard one byte and resync
      serial.read();
    }
  }

  // No valid reading
  return -1;
}
