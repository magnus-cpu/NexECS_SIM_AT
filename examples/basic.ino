#include <AT_lib.h>
#include <AT_cmd.h>
#include "client_cert.h"

#define MODEM_TX_PIN 27
#define MODEM_RX_PIN 26

HardwareSerial SerialAT(2);
AT_Lib modem(SerialAT, Serial);

void setup()
{
  Serial.begin(115200);

  modem.begin(115200, MODEM_RX_PIN, MODEM_TX_PIN);

  if (!modem.waitForPBDONE())
  {
    Serial.println("Boot failure.");
  }
  if (!modem.modemReady())
  {
    Serial.println("Modem not ready.");
  }

  modem.sendCommand(AT_CMD_BASIC);
  modem.sendCommand(AT_CMD_ECHO_OFF);
  modem.sendCommand(AT_CMD_CHECK_SIM);

  
 // Delete specific certificate
  bool deleted = modem.deleteCertificate("client_key.der");

  if (deleted) {
    Serial.println("Certificate removed from SIM7600.");
  } else {
    Serial.println("Failed to remove certificate.");
  }

  // Get list of certificates on SIM7600
  String certs = modem.listCertificates();
  Serial.println("Certificates stored:");
  Serial.println(certs);

  // Upload certificate only if missing
  bool success = modem.uploadCertificateIfMissing("client_key.der", client_cert, client_cert_len);
  if (success)
  {
    Serial.println("Certificate is ready on SIM7600!");
  }
  else
  {
    Serial.println("Certificate upload failed.");
  }
}
void loop() {}
