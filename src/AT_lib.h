#ifndef AT_LIB_H
#define AT_LIB_H

#include <Arduino.h>
#include "Sim76xx_mqtt_errors.h"

/* =====================================================
 * MQTT STATE MACHINE
 * ===================================================== */
typedef enum
{
  MQTT_STATE_IDLE = 0,
  MQTT_STATE_STARTED,
  MQTT_STATE_ACQUIRED,
  MQTT_STATE_CONNECTED,
  MQTT_STATE_SUBSCRIBED
} SIM76xx_mqtt_state_t;

/* =====================================================
 * CALLBACK TYPES
 * ===================================================== */
typedef void (*mqtt_rx_callback_t)(const char *topic, const char *payload, uint16_t payloadLen);
typedef void (*sms_rx_callback_t)(const char *sender, const char *timestamp, const char *message);

/* =====================================================
 * AT LIB CLASS
 * ===================================================== */
class AT_Lib
{
public:
  AT_Lib(HardwareSerial &modemSerial, Stream &debugSerial);

  bool begin(unsigned long baud, int8_t rxPin, int8_t txPin);

  /* Basic AT helpers */
  String sendCommand(const char *command, uint32_t timeout = 500);
  String sendFormatted(const char *format, const char *value, uint32_t timeout = 500);

  /* Modem lifecycle */
  bool waitForPBDONE(uint32_t timeout = 15000);
  bool modemReady(uint32_t timeout = 15000);

  /* Polling */
  void mqttPoll(); // only MQTT
  void smsPoll();  // only SMS
  void poll();     // both MQTT + SMS non-blocking

  bool syncTimeOnTimezone(uint32_t timeout = 30000);

  /* Certificate management */
  String listCertificates(uint32_t timeout = 1000);
  bool uploadCertificate(const char *filename, const uint8_t *data, uint32_t length, uint32_t timeout = 5000);
  bool uploadCertificateIfMissing(const char *filename, const uint8_t *data, uint32_t length, uint32_t timeout = 5000);
  bool deleteCertificate(const char *filename);

  /* =================================================
   * MQTT API (SIM7600 AT-based)
   * ================================================= */
  bool mqttStart(uint32_t timeout = 5000);
  bool mqttStop(uint32_t timeout = 3000);
  bool mqttAcquire(uint8_t clientId, const char *clientName);
  bool mqttConnect(uint8_t clientId, const char *uri, const char *user, const char *pass,
                   uint16_t keepAlive = 60, bool cleanSession = true, uint32_t timeout = 10000);
  bool mqttSubscribe(uint8_t clientId, const char *topic, uint8_t qos, mqtt_rx_callback_t cb,
                     uint32_t timeout = 5000);
  bool mqttUnsubscribe(uint8_t clientId, const char *topic, uint32_t timeout = 5000);
  bool mqttPublish(uint8_t clientId, const char *topic, const uint8_t *payload, uint16_t length,
                   uint8_t qos = 0, uint32_t timeout = 5000);
  bool mqttDisconnect(uint8_t clientId, uint32_t timeout = 5000);

  /* SMS API */
  bool enableSMS();
  bool sendSMS(const char *phoneNumber, const char *message, uint32_t timeout = 15000);

  void onMQTTReceived(mqtt_rx_callback_t cb) { _mqttCallback = cb; }
  void onSMSReceived(sms_rx_callback_t cb) { _smsCallback = cb; }

  bool readSMS(uint8_t index, String &outSender, String &outTime, String &outMsg);
  bool deleteSMS(uint8_t index);
  bool deleteAllSMS();

  /* State access */
  SIM76xx_mqtt_state_t mqttState() const { return _mqttState; }

private:
  /* Core serial interfaces */
  HardwareSerial &_modemSerial;
  Stream &_debugSerial;

  /* MQTT RX state machine */
  enum RxState
  {
    RX_IDLE,
    RX_TOPIC,
    RX_PAYLOAD
  };
  RxState rxState = RX_IDLE;
  uint16_t expectedLen = 0;
  uint16_t received = 0;
  static const uint16_t MQTT_TOPIC_MAX = 128;
  static const uint16_t MQTT_PAYLOAD_MAX = 1024;

  char rxTopic[MQTT_TOPIC_MAX];
  char rxPayload[MQTT_PAYLOAD_MAX];

  typedef void (*mqtt_rx_callback_t)(const char *topic,
                                     const char *payload,
                                     uint16_t length);

  /* SMS buffer */
  String smsLineBuffer = "";

  /* Callbacks */
  mqtt_rx_callback_t _mqttCallback = nullptr;
  sms_rx_callback_t _smsCallback = nullptr;

  /* MQTT state */
  SIM76xx_mqtt_state_t _mqttState = MQTT_STATE_IDLE;

  /* Internal helpers */
  String readUntilTimeout(uint32_t timeout);
  bool waitPrompt(char prompt, uint32_t timeout);
  bool rebootModem(uint32_t timeout = 15000);
  bool parseMqttResult(const String &response, const char *prefix, SIM76xx_mqtt_err_t *errOut = nullptr);
};

#endif /* AT_LIB_H */
