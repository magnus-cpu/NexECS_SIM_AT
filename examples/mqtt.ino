#include <Arduino.h>
#include "AT_Lib.h"
#include "Sim76xx_mqtt_errors.h"

// UARTs
#define MODEM_RX 26
#define MODEM_TX 27

// APN settings
#define APN     "internet"
#define APN_USER ""
#define APN_PASS ""

// Globals
AT_Lib at(Serial1, Serial);

// ----------------------------------------------------
// Basic cellular network setup (PDP context)
// ----------------------------------------------------
bool setupNetwork()
{
    Serial.println("[NET] Setting APN...");

    at.sendCommand("AT+CFUN=1", 2000);
    at.sendCommand("AT+CGDCONT=1,\"IP\",\"" APN "\"", 3000);

    if (APN_USER[0]) {
        at.sendCommand("AT+CGAUTH=1,1,\"" APN_USER "\",\"" APN_PASS "\"", 3000);
    }

    String res = at.sendCommand("AT+CGATT=1", 10000);
    if (res.indexOf("OK") < 0) {
        Serial.println("[NET] Attach failed");
        return false;
    }

    Serial.println("[NET] Network attached");
    return true;
}

// ----------------------------------------------------
// Arduino SETUP
// ----------------------------------------------------
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== ESP32 + SIM7600 MQTT AT Example ===");

    // Start modem UART
    at.begin(115200, MODEM_RX, MODEM_TX);

    if (!at.waitForPBDONE(15000)) return;
    if (!at.modemReady(5000)) return;

    if (!setupNetwork()) return;

    // ---------------- MQTT ----------------
    Serial.println("[MQTT] Starting service...");
    if (!at.mqttStart()) {
        Serial.println("[MQTT] Start failed");
        return;
    }

    Serial.println("[MQTT] Acquiring client...");
    if (!at.mqttAcquire(0, "esp32client")) {
        Serial.println("[MQTT] Acquire failed");
        return;
    }

    Serial.println("[MQTT] Connecting...");
    if (!at.mqttConnect(
            0,
            "tcp://broker.hivemq.com:1883",
            "username",
            "password",
            60,
            true
        )) {
        Serial.println("[MQTT] Connect failed");
        return;
    }

    Serial.println("[MQTT] Subscribing...");
    if (!at.mqttSubscribe(0, "test/topic", 0, onMqttMessage)) {
        Serial.println("[MQTT] Subscribe failed");
        return;
    }

    Serial.println("[MQTT] Publishing...");
    const char* msg = "hello from esp32 + sim7600";
    at.mqttPublish(
        0,
        "test/topic",
        (const uint8_t*)msg,
        strlen(msg),
        0
    );

    Serial.println("[MQTT] Setup complete");
}

// ----------------------------------------------------
// Arduino LOOP
// ----------------------------------------------------
void loop()
{
    // Required to receive MQTT messages
    at.mqttPoll();

    static uint32_t lastPub = 0;
    if (millis() - lastPub > 10000) {
        lastPub = millis();

        const char* msg = "periodic ping";
        at.mqttPublish(
            0,
            "test/topic",
            (const uint8_t*)msg,
            strlen(msg),
            0
        );
    }
}
