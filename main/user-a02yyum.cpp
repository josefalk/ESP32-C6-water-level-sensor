#include "user-a02yyuh.h"

A02YYUW::A02YYUW(HardwareSerial& serial, int rxPin, int txPin)
  : sensorSerial(serial), rxPin(rxPin), txPin(txPin) {}

void A02YYUW::begin(uint32_t baudRate) {
  sensorSerial.begin(baudRate, SERIAL_8N1, rxPin, txPin);
}

int A02YYUW::readDistance() {
  if (sensorSerial.available() >= 4) {
    uint8_t buf[4];
    sensorSerial.readBytes(buf, 4);

    if (buf[0] == 0xFF) {
      int distance = ((buf[1] << 8) + buf[2]) / 10;
      return distance;
    }
  }
  return -1; // invalid or no data
}
