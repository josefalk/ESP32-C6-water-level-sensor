#ifndef A02YYUW_H
#define A02YYUW_H


#include <HardwareSerial.h>

class A02YYUW {
public:
  A02YYUW(HardwareSerial& serial, int rxPin, int txPin);
  void begin(uint32_t baudRate = 9600);
  int readDistance();  // returns distance in cm, or -1 if invalid

private:
  HardwareSerial& sensorSerial;
  int rxPin, txPin;
};

#endif
