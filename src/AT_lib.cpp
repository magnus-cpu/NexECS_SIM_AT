#include "AT_Lib.h"
#include <time.h>
#include "Sim76xx_mqtt_errors.h"

AT_Lib::AT_Lib(HardwareSerial &modemSerial, Stream &debugSerial)
    : _modemSerial(modemSerial), _debugSerial(debugSerial) {}

bool AT_Lib::begin(unsigned long baud, int8_t rxPin, int8_t txPin)
{
  _modemSerial.begin(baud, SERIAL_8N1, rxPin, txPin);
  delay(100);
  return true;
}

// =====================================================
// BASIC COMMAND SEND
// =====================================================
String AT_Lib::sendCommand(const char *command, uint32_t timeout)
{
  _modemSerial.println(command);
  return readUntilTimeout(timeout);
}

String AT_Lib::sendFormatted(const char *format, const char *value, uint32_t timeout)
{
  char buffer[128];
  snprintf(buffer, sizeof(buffer), format, value);
  return sendCommand(buffer, timeout);
}

// =====================================================
// TIMEOUT-BASED READER
// =====================================================
String AT_Lib::readUntilTimeout(uint32_t timeout)
{
  String response = "";
  uint32_t start = millis();

  while (millis() - start < timeout)
  {
    while (_modemSerial.available())
    {
      char c = _modemSerial.read();
      response += c;
      _debugSerial.write(c);
    }
  }
  _debugSerial.println();
  return response;
}

// =====================================================
// WAIT FOR PB DONE
// =====================================================
bool AT_Lib::waitForPBDONE(uint32_t timeout)
{
  _debugSerial.println("Waiting for PB DONE...");

  uint32_t start = millis();
  String buffer = "";

  while (millis() - start < timeout)
  {
    while (_modemSerial.available())
    {
      char c = _modemSerial.read();
      buffer += c;
      _debugSerial.write(c);

      if (buffer.indexOf("PB DONE") >= 0)
      {
        _debugSerial.println("\n[PBDONE] Modem finished booting!");
        return true;
      }
    }
  }

  _debugSerial.println("\n[ERROR] Timeout waiting for PB DONE!");
  return false;
}

// =====================================================
// AUTOMATIC MODEM READY CHECK
// Checks: AT → OK
// =====================================================
bool AT_Lib::modemReady(uint32_t timeout)
{
  uint32_t start = millis();

  while (millis() - start < timeout)
  {
    _debugSerial.println("Checking modem ready...");
    String res = sendCommand("AT", 300);

    if (res.indexOf("OK") >= 0)
    {
      _debugSerial.println("[READY] Modem responded to AT.");
      return true;
    }
    delay(200);
  }

  _debugSerial.println("[ERROR] Modem not responding to AT.");
  return false;
}

// =====================================================
// Non-blocking MQTT Poll
// =====================================================
static bool isLikelyJson(const char *buf, uint16_t len)
{
  if (len < 2)
    return false;

  // Trim leading whitespace
  while (len && (*buf == ' ' || *buf == '\n' || *buf == '\r' || *buf == '\t'))
  {
    buf++;
    len--;
  }

  return (buf[0] == '{' && buf[len - 1] == '}') ||
         (buf[0] == '[' && buf[len - 1] == ']');
}

void AT_Lib::mqttPoll()
{
  while (_modemSerial.available())
  {
    String line = _modemSerial.readStringUntil('\n');
    line.trim();
    _debugSerial.println(line);

    // ================================
    // START OF MQTT RX
    // ================================
    if (line.startsWith("+CMQTTRXSTART"))
    {
      rxState = RX_IDLE;
      received = 0;
      rxTopic[0] = 0;
      rxPayload[0] = 0;
      continue;
    }

    // ================================
    // TOPIC HEADER
    // ================================
    if (line.startsWith("+CMQTTRXTOPIC:"))
    {
      rxState = RX_TOPIC;
      continue;
    }

    // ================================
    // PAYLOAD HEADER
    // ================================
    if (line.startsWith("+CMQTTRXPAYLOAD:"))
    {
      rxState = RX_PAYLOAD;
      received = 0;
      rxPayload[0] = 0;
      continue;
    }

    // ================================
    // END OF MQTT RX
    // ================================
    if (line.startsWith("+CMQTTRXEND"))
    {
      if (_mqttCallback &&
          rxTopic[0] &&
          rxPayload[0] &&
          isLikelyJson(rxPayload, received))
      {
        _mqttCallback(rxTopic, rxPayload, received);
      }
      else
      {
        _debugSerial.println("[MQTT] Invalid or empty payload ignored");
      }

      rxState = RX_IDLE;
      received = 0;
      continue;
    }

    // ================================
    // DATA LINES
    // ================================
    if (rxState == RX_TOPIC)
    {
      line.toCharArray(rxTopic, sizeof(rxTopic));
    }
    else if (rxState == RX_PAYLOAD)
    {
      uint16_t len = line.length();

      if ((received + len) < (MQTT_PAYLOAD_MAX - 1))
      {
        memcpy(rxPayload + received, line.c_str(), len);
        received += len;
        rxPayload[received] = '\0';
      }
      else
      {
        _debugSerial.println("[MQTT] Payload overflow, truncated");
      }
    }
  }
}

// =====================================================
// Non-blocking SMS Poll (process one line per call)
// =====================================================
void AT_Lib::smsPoll()
{
    static String rx;

    while (_modemSerial.available())
    {
        char c = _modemSerial.read();

        // Ignore CR/LF
        if (c == '\r' || c == '\n')
            continue;

        rx += c;

        // Prevent buffer overflow / heap abuse
        if (rx.length() > 128)
            rx.remove(0, rx.length() - 128);

        // Look for +CMTI indication
        int p = rx.indexOf("+CMTI:");
        if (p < 0)
            continue;

        // Find comma separating storage and index
        int comma = rx.indexOf(',', p);
        if (comma < 0)
            continue;

        // Parse index digits
        int i = comma + 1;
        while (i < rx.length() && isDigit(rx[i]))
            i++;

        // Index not fully received yet
        if (i == comma + 1)
            continue;

        uint8_t index = rx.substring(comma + 1, i).toInt();

        _debugSerial.print("[SMS] index no: ");
        _debugSerial.println(index);

        String sender, time, msg;
        if (readSMS(index, sender, time, msg))
        {
            if (_smsCallback)
            {
                _smsCallback(
                    sender.c_str(),
                    time.c_str(),
                    msg.c_str()
                );
            }

            deleteSMS(index);
        }

        // Remove processed portion from buffer
        rx.remove(0, i);
    }
}


// =====================================================
// Unified poll() → MQTT + SMS
// =====================================================
void AT_Lib::poll()
{
  mqttPoll();
  smsPoll();
}

bool AT_Lib::syncTimeOnTimezone(uint32_t timeout)
{
  _debugSerial.println("[TIME] Checking timezone auto-update status...");

  String tzr = sendCommand("AT+CTZR?", 1000);
  bool ctzuEnabled = false;

  // Expected: +CTZR: 1
  if (tzr.indexOf("+CTZR:") >= 0)
  {
    int idx = tzr.indexOf(":");
    if (idx > 0)
    {
      int val = tzr.substring(idx + 1).toInt();
      ctzuEnabled = (val == 1);
    }
  }

  if (!ctzuEnabled)
  {
    _debugSerial.println("[TIME] CTZU disabled. Enabling and rebooting...");

    sendCommand("AT+CTZU=1", 1000);
    sendCommand("AT&W", 1000);

    if (!rebootModem(timeout))
    {
      _debugSerial.println("[TIME][ERROR] Modem reboot failed.");
      return false;
    }
  }
  else
  {
    _debugSerial.println("[TIME] CTZU already enabled.");
  }

  _debugSerial.println("[TIME] Reading network time...");
  String resp = sendCommand("AT+CCLK?", timeout);

  int q1 = resp.indexOf('"');
  int q2 = resp.lastIndexOf('"');

  if (q1 < 0 || q2 <= q1)
  {
    _debugSerial.println("[TIME][ERROR] Invalid CCLK response.");
    return false;
  }

  String t = resp.substring(q1 + 1, q2);

  // Expected: yy/MM/dd,hh:mm:ss±zz
  if (t.length() < 17)
  {
    _debugSerial.println("[TIME][ERROR] Time string malformed.");
    return false;
  }

  struct tm tm{};
  tm.tm_year = t.substring(0, 2).toInt() + 100;
  tm.tm_mon = t.substring(3, 5).toInt() - 1;
  tm.tm_mday = t.substring(6, 8).toInt();
  tm.tm_hour = t.substring(9, 11).toInt();
  tm.tm_min = t.substring(12, 14).toInt();
  tm.tm_sec = t.substring(15, 17).toInt();

  time_t epoch = mktime(&tm);

  // Sanity check (Nov 2023+)
  if (epoch < 1700000000)
  {
    _debugSerial.println("[TIME][ERROR] Network time not valid.");
    return false;
  }

  struct timeval now;
  now.tv_sec = epoch;
  now.tv_usec = 0;

  settimeofday(&now, nullptr);

  _debugSerial.print("[TIME] Synced successfully. Epoch=");
  _debugSerial.println(epoch);

  return true;
}

bool AT_Lib::rebootModem(uint32_t timeout)
{
  _debugSerial.println("[MODEM] Rebooting modem...");
  sendCommand("AT+CFUN=1,1", 1000);

  delay(2000); // modem resets UART

  return waitForPBDONE(timeout) && modemReady(timeout);
}

// =====================================================
// LIST STORED CERTIFICATES
// Sends AT+CCERTLIST and returns modem response
// =====================================================
String AT_Lib::listCertificates(uint32_t timeout)
{
  _debugSerial.println("Fetching list of certificates...");
  String response = sendCommand("AT+CCERTLIST", timeout);

  // Optional: you can parse the response here if needed
  // Example: split lines, extract names, etc.

  _debugSerial.println("[CERT LIST]");
  _debugSerial.println(response);
  return response;
}

// =====================================================
// UPLOAD CERTIFICATES TO SIM7600
// Sends AT+CCERTDOWN and returns modem response
// =====================================================
bool AT_Lib::uploadCertificate(const char *filename, const uint8_t *data, uint32_t length, uint32_t timeout)
{
  _debugSerial.printf("Uploading certificate: %s (%u bytes)\n", filename, length);

  // Send AT+CCERTDOWN command with filename and length
  String cmd;
  cmd = "AT+CCERTDOWN=\"" + String(filename) + "\"," + String(length);
  _modemSerial.println(cmd);

  // Wait for '>' prompt from modem
  uint32_t start = millis();
  bool ready = false;
  while (millis() - start < timeout)
  {
    if (_modemSerial.available())
    {
      String resp = _modemSerial.readStringUntil('\n');
      _debugSerial.println(resp);
      if (resp.indexOf('>') >= 0)
      {
        ready = true;
        break;
      }
    }
  }

  if (!ready)
  {
    _debugSerial.println("[ERROR] Modem not ready for certificate upload!");
    return false;
  }

  // Send raw certificate bytes
  for (uint32_t i = 0; i < length; i++)
  {
    _modemSerial.write(data[i]);
  }

  _debugSerial.println("\n[INFO] Certificate bytes sent, waiting for OK...");

  // Wait for final OK or error
  start = millis();
  while (millis() - start < timeout)
  {
    if (_modemSerial.available())
    {
      String resp = _modemSerial.readStringUntil('\n');
      _debugSerial.println(resp);
      if (resp.indexOf("OK") >= 0)
      {
        _debugSerial.println("[SUCCESS] Certificate uploaded!");
        return true;
      }
      if (resp.indexOf("ERROR") >= 0)
      {
        _debugSerial.println("[ERROR] Certificate upload failed!");
        return false;
      }
    }
  }

  _debugSerial.println("[ERROR] Timeout waiting for modem response after upload!");
  return false;
}

// =====================================================
// UPLOAD CERTIFICATE IF MISSING
// Use this to upload a missing certificate to modem
// =====================================================
bool AT_Lib::uploadCertificateIfMissing(const char *filename, const uint8_t *data, uint32_t length, uint32_t timeout)
{
  // Step 1: List existing certificates
  String certs = listCertificates(timeout);

  // Step 2: Check if the certificate is already present
  if (certs.indexOf(filename) >= 0)
  {
    _debugSerial.printf("[INFO] Certificate '%s' already exists, skipping upload.\n", filename);
    return true; // Already present, consider success
  }

  // Step 3: Upload certificate
  return uploadCertificate(filename, data, length, timeout);
}

// =====================================================
// DELETE CERTIFICATE
// This removes the stored sll cert file in the modem
// =====================================================
bool AT_Lib::deleteCertificate(const char *filename)
{
  String cmd = "AT+CCERTDELE=\"";
  cmd += filename;
  cmd += "\"";

  String res = sendCommand(cmd.c_str(), 3000);

  if (res.indexOf("OK") >= 0)
  {
    _debugSerial.println("[DELETE OK] Certificate deleted.");
    return true;
  }

  _debugSerial.println("[DELETE FAIL] Certificate NOT deleted.");
  return false;
}

// =====================================================
// PARSE MQTT RESULTS
// =====================================================
bool AT_Lib::parseMqttResult(const String &response, const char *prefix, SIM76xx_mqtt_err_t *errOut)
{
  String tag = String("+") + prefix + ":";

  int idx = response.indexOf(tag);
  if (idx < 0)
  {
    _debugSerial.printf("[MQTT] %s not found\n", prefix);
    return false;
  }

  int lineEnd = response.indexOf('\n', idx);
  String line = (lineEnd > 0) ? response.substring(idx, lineEnd) : response.substring(idx);

  int comma = line.indexOf(',');
  if (comma < 0)
  {
    _debugSerial.printf("[MQTT] %s malformed\n", prefix);
    return false;
  }

  int err = line.substring(comma + 1).toInt();
  SIM76xx_mqtt_err_t mqttErr = (SIM76xx_mqtt_err_t)err;

  if (errOut)
  {
    *errOut = mqttErr;
  }

  _debugSerial.printf("[MQTT %s] %d → %s\n",
                      prefix,
                      err,
                      SIM76xx_mqtt_err_str(mqttErr));

  return (mqttErr == SIM76xx_MQTT_OK);
}

// =====================================================
// WAIT FOR PROMPT FROM MODEM
// use this to tell next commant to wait for a specified
// promt before timeout.
// =====================================================
bool AT_Lib::waitPrompt(char prompt, uint32_t timeout)
{
  uint32_t start = millis();
  while (millis() - start < timeout)
  {
    if (_modemSerial.available())
    {
      char c = _modemSerial.read();
      _debugSerial.write(c);
      if (c == prompt)
        return true;
    }
  }
  return false;
}

// =====================================================
// MQTT START
// This start the mqtt services
// =====================================================
bool AT_Lib::mqttStart(uint32_t timeout)
{
  String r = sendCommand("AT+CMQTTSTART", timeout);
  return r.indexOf("OK") >= 0;
}

// =====================================================
// MQTT STOP
// This stops the mqtt service
// =====================================================
bool AT_Lib::mqttStop(uint32_t timeout)
{
  String r = sendCommand("AT+CMQTTSTOP", timeout);
  return r.indexOf("OK") >= 0;
}

// =====================================================
// MQTT ACQUIRE
// This acquire the client name
// Use this after calling mqtt start
// =====================================================
bool AT_Lib::mqttAcquire(uint8_t clientId, const char *clientName)
{
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "AT+CMQTTACCQ=%d,\"%s\"", clientId, clientName);

  String r = sendCommand(cmd, 3000);
  return r.indexOf("OK") >= 0;
}

// =====================================================
// MQTT CONNECT
// This connect the mqtt client device to the mqtt broker
// Use this after calling mqtt acquire
// =====================================================
bool AT_Lib::mqttConnect(uint8_t clientId, const char *uri, const char *user, const char *pass, uint16_t keepAlive, bool cleanSession, uint32_t timeout)
{
  // Build full connect command with username and password directly
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "AT+CMQTTCONNECT=%d,\"%s\",%u,%d,\"%s\",\"%s\"", clientId, uri, keepAlive, cleanSession ? 1 : 0, user, pass);

  // Send command to modem
  String r = sendCommand(cmd, timeout);

  // Parse result
  return parseMqttResult(r, "CMQTTCONNECT");
}

// =====================================================
// MQTT SUBSCRIBE
// This subscribe the mqtt client to the server
// call this once
// =====================================================
bool AT_Lib::mqttSubscribe(uint8_t clientId, const char *topic, uint8_t qos, mqtt_rx_callback_t cb, uint32_t timeout)
{
  if (!topic || strlen(topic) == 0)
  {
    _debugSerial.println("[MQTT] Empty topic rejected");
    return false;
  }

  _mqttCallback = cb;

  // 1. Set the topic
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "AT+CMQTTSUBTOPIC=%d,%u,%u", clientId, strlen(topic), qos);
  _modemSerial.println(cmd);

  // Wait for '>' prompt
  if (!waitPrompt('>', timeout))
    return false;

  // Send the topic string
  _modemSerial.print(topic);

  // Wait for OK after topic is set
  String res = readUntilTimeout(timeout);
  if (res.indexOf("OK") < 0)
  {
    _debugSerial.println("[MQTT] Failed to set subscribe topic");
    return false;
  }

  // 2. Subscribe
  snprintf(cmd, sizeof(cmd), "AT+CMQTTSUB=%d", clientId);
  String r2 = sendCommand(cmd, timeout);

  return parseMqttResult(r2, "CMQTTSUB");
}

// =====================================================
// MQTT PUBLISH
// This publish the message to the mqtt server
// call this with passed params it handles the rest
// with empty topic rejection
// =====================================================
bool AT_Lib::mqttPublish(uint8_t clientId, const char *topic, const uint8_t *payload, uint16_t length, uint8_t qos, uint32_t timeout)
{
  if (!topic || strlen(topic) == 0 || strlen(topic) > 128)
  {
    _debugSerial.println("[MQTT] Invalid topic");
    return false;
  }
  if (!payload || length == 0 || length > 1024)
  {
    _debugSerial.println("[MQTT] Invalid payload length");
    return false;
  }

  char cmd[64];

  // 1. Set topic
  snprintf(cmd, sizeof(cmd), "AT+CMQTTTOPIC=%d,%u", clientId, strlen(topic));
  _modemSerial.println(cmd);
  if (!waitPrompt('>', timeout))
    return false;
  _modemSerial.print(topic);

  String res = readUntilTimeout(timeout);
  if (res.indexOf("OK") < 0)
  {
    _debugSerial.println("[MQTT] Failed to set topic");
    return false;
  }

  // 2. Set payload
  snprintf(cmd, sizeof(cmd), "AT+CMQTTPAYLOAD=%d,%u", clientId, length);
  _modemSerial.println(cmd);
  if (!waitPrompt('>', timeout))
    return false;
  _modemSerial.write(payload, length);

  res = readUntilTimeout(timeout);
  if (res.indexOf("OK") < 0)
  {
    _debugSerial.println("[MQTT] Failed to set payload");
    return false;
  }

  // 3. Publish
  snprintf(cmd, sizeof(cmd), "AT+CMQTTPUB=%d,%d,60", clientId, qos);
  res = sendCommand(cmd, timeout);

  return parseMqttResult(res, "CMQTTPUB");
}

// =====================================================
// MQTT UNSUBSCRIBE
// This unsubscribes the client from the server topic.
// =====================================================
bool AT_Lib::mqttUnsubscribe(uint8_t clientId, const char *topic, uint32_t timeout)
{
  char cmd[64];
  snprintf(cmd, sizeof(cmd),
           "AT+CMQTTUNSUB=%d,%u", clientId, strlen(topic));

  _modemSerial.println(cmd);
  if (!waitPrompt('>', timeout))
    return false;
  _modemSerial.print(topic);

  String r = readUntilTimeout(timeout);
  return parseMqttResult(r, "CMQTTUNSUB");
}

// =====================================================
// MQTT DISCONNECT
// This disconnects the mqtt service and should be called
// after unsubcribing the server
// =====================================================
bool AT_Lib::mqttDisconnect(uint8_t clientId, uint32_t timeout)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd),
           "AT+CMQTTDISC=%d,60", clientId);

  String r = sendCommand(cmd, timeout);
  return parseMqttResult(r, "CMQTTDISC");
}

bool AT_Lib::enableSMS()
{
  return sendCommand("AT+CMGF=1", 2000).indexOf("OK") >= 0 &&
         sendCommand("AT+CNMI=2,1,0,0,0", 2000).indexOf("OK") >= 0;
}

// =====================================================
// SMS SENDING (TEXT MODE)
// =====================================================
bool AT_Lib::sendSMS(const char *phoneNumber, const char *message, uint32_t timeout)
{
  if (!phoneNumber || !message || strlen(message) == 0)
  {
    _debugSerial.println("[SMS] Invalid phone number or message");
    return false;
  }

  // 1. Set SMS text mode
  if (sendCommand("AT+CMGF=1", 2000).indexOf("OK") < 0)
  {
    _debugSerial.println("[SMS] Failed to set text mode");
    return false;
  }

  // 2. Set destination number
  char cmd[48];
  snprintf(cmd, sizeof(cmd), "AT+CMGS=\"%s\"", phoneNumber);

  _modemSerial.println(cmd);

  // 3. Wait for '>' prompt
  if (!waitPrompt('>', timeout))
  {
    _debugSerial.println("[SMS] No prompt from modem");
    return false;
  }

  // 4. Send message body
  _modemSerial.print(message);

  // 5. End messge with CTRL+Z
  _modemSerial.write(0x1A);

  // 6. Wait for response
  String r = readUntilTimeout(timeout);

  if (r.indexOf("+CMGS:") >= 0 && r.indexOf("OK") >= 0)
  {
    _debugSerial.println("[SMS] Sent successfuly");
    return true;
  }

  _debugSerial.println("[SMS] Send failed");
  return false;
}

bool AT_Lib::readSMS(uint8_t index, String &outSender, String &outTime, String &outMsg)
{
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "AT+CMGR=%d", index);

  String r = sendCommand(cmd, 5000);

  int hdr = r.indexOf("+CMGR:");
  if (hdr < 0)
    return false;

  // Collect quoted fields
  String fields[4];
  int fieldIndex = 0;
  int pos = hdr;

  while (fieldIndex < 4)
  {
    int start = r.indexOf('"', pos);
    if (start < 0)
      break;

    int end = r.indexOf('"', start + 1);
    if (end < 0)
      break;

    fields[fieldIndex++] = r.substring(start + 1, end);
    pos = end + 1;
  }

  if (fieldIndex < 4)
  {
    _debugSerial.println("[SMS] CMGR header parse failed");
    return false;
  }

  // Assign correct fields
  // fields[0] = REC UNREAD
  // fields[1] = sender
  // fields[2] = alpha
  // fields[3] = timestamp
  outSender = fields[1];
  outTime = fields[3];

  // Message body
  int bodyStart = r.indexOf('\n', hdr);
  if (bodyStart < 0)
    return false;
  bodyStart++;

  int bodyEnd = r.indexOf("\r\nOK", bodyStart);
  if (bodyEnd < 0)
    return false;

  outMsg = r.substring(bodyStart, bodyEnd);
  outMsg.trim();

  return true;
}

bool AT_Lib::deleteSMS(uint8_t index)
{
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "AT+CMGD=%d", index);
  return sendCommand(cmd, 3000).indexOf("OK") >= 0;
}

bool AT_Lib::deleteAllSMS()
{
  // 4 = delete all messages
  return sendCommand("AT+CMGD=1,4", 5000).indexOf("OK") >= 0;
}
