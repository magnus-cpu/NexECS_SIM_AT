#include "AT_lib.h"

ATLib::ATLib(hardwareSerial& modemSerial, Stream& debugSerial) 
: _modemSerial(modemSerial), _debugSerial(debugSerial) {}

bool ATLib::begin(unsigned long baud, int8_t rxPin, int8_t txPin){
  _modemSerial.begin(baud, SERIAL_8N1, rxpin, txPin);
  delay(100);
  return true;
}

String ATLib::sendCommand(const char* command, uint16_t timeout) {
  _modemSerial.println(command);
  delay(timeout);

  String response = "";
  while (_modemSerial.available()) {
    response += _modemSerial.readString();
  }

  _debugSerial.println(response);
  return response;
}

String ATLib::sendFormatted(const char* format, const char* value, uint16_t timeout) {
  char buffer[128];
  snprintf(buffer, sizeof(buffer), format, value);
  return sendCommand(buffer, timeout);
}
