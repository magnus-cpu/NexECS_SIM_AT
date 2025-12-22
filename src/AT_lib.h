#ifndef ATCLIB_H
#define ATCLIB_H

#include <Arduino.h>

class ATLib
{
public:
  ATLib(HardwareSerial &modemSerial, Stream &dubugSerial);

  bool begin(unsigned long baud, int8_t rxPin, int8_t txPin);
  String sendCommand(const char *command, uint16_t timeout = 500);
  String sendFormatted(const char *format, const char *value, uint16_t timeout = 500);

private:
  HardwareSerial &_modemSerial;
  Stream &_debugSerial;
};

#endif
