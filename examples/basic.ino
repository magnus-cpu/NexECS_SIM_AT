#include <AT_lib.h>
#include <AT_cmd.h>

#define MODEM_TX_PIN 27
#define MODEM_RX_PIN 26

HardwareSerial SerialAT(2);
ATCommandLib modem(SerialAT, Serial);

void setup() {
  Serial.begin(115200);

  modem.begin(115200, MODEM_RX_PIN, MODEM_TX_PIN);

  modem.sendCommand(AT_CMD_BASIC);
  modem.sendCommand(AT_CMD_ECHO_OFF);
  modem.sendCommand(AT_CMD_CHECK_SIM);
}
void loop() {}
