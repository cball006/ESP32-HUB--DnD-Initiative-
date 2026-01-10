#include "bluetooth.h"
#include <NimBLEDevice.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

extern Adafruit_NeoPixel LED_RGB;

// ================= BLE GLOBALS =================
NimBLEServer* pServer;
NimBLECharacteristic* pTxCharacteristic;
NimBLECharacteristic* pRxCharacteristic;
bool deviceConnected = false;

// ================= TURN COMMAND BLE =================
#define TURN_COMMAND_UUID "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"

NimBLECharacteristic* pTurnCommandCharacteristic;


// ================= ESPNOW CONFIG =================
#define ESPNOW_PACKET_SIZE 6   // [0]=Hub, [1..5]=Players

typedef struct __attribute__((packed)) {
    uint8_t states[ESPNOW_PACKET_SIZE];
} EspNowPacket;


// 🔑 HARD-CODED RECEIVER MACS (slots 2–6)
uint8_t receiverMACs[][6] = {
    {0x3C, 0xDC, 0x75, 0x63, 0x3E, 0x88}, // Player 2
    {0x3C, 0xDC, 0x75, 0x63, 0x47, 0xD8}, // Player 3
    {0x10, 0x20, 0xBA, 0x4A, 0xF0, 0x48}, // Player 4
    {0x10, 0x20, 0xBA, 0x4A, 0xE6, 0x24}, // Player 5
    {0x10, 0x20, 0xBA, 0x4A, 0xEF, 0x74}, // Player 6
};

constexpr uint8_t NUM_RECEIVERS = sizeof(receiverMACs) / 6;

// ================= BLE UUIDS =================
#define SERVICE_UUID  "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_UUID_RX  "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_UUID_TX  "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// ================= BLE CALLBACKS =================
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer*) override {
        deviceConnected = true;
        Serial.println("BLE client connected");
    }

    void onDisconnect(NimBLEServer*) override {
        deviceConnected = false;
        Serial.println("BLE client disconnected");
        NimBLEDevice::startAdvertising();
        Serial.println("Advertising restarted");
    }
};

// ================= ESP-NOW INIT =================
void initEspNow() {
    Serial.println("Initializing ESP-NOW...");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("❌ ESP-NOW init failed");
        return;
    }

    Serial.println("✅ ESP-NOW initialized");

    // Add all receiver peers
    for (int i = 0; i < NUM_RECEIVERS; i++) {
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, receiverMACs[i], 6);
        peerInfo.channel = 0;        // 🔑 REQUIRED with BLE
        peerInfo.encrypt = false;

        esp_err_t err = esp_now_add_peer(&peerInfo);

        if (err == ESP_OK) {
            Serial.printf("✅ ESP-NOW peer %d added\n", i + 1);
        } else if (err == ESP_ERR_ESPNOW_EXIST) {
            Serial.printf("⚠️ ESP-NOW peer %d already exists\n", i + 1);
        } else {
            Serial.printf("❌ Failed to add peer %d: %d\n", i + 1, err);
        }
    }
}

// ================= ESP-NOW SEND =================
void sendEspNowPacket(const uint8_t* data, size_t len) {
    EspNowPacket packet;
    memset(packet.states, 0, ESPNOW_PACKET_SIZE); // fill with 0s
    memcpy(packet.states, data, (len < ESPNOW_PACKET_SIZE ? len : ESPNOW_PACKET_SIZE)); // copy safely

    for (int i = 0; i < NUM_RECEIVERS; i++) {
        esp_err_t result = esp_now_send(receiverMACs[i], (uint8_t*)&packet, sizeof(packet));
        if (result == ESP_OK) {
            Serial.printf("📡 ESP-NOW packet sent to receiver %d\n", i + 1);
        } else {
            Serial.printf("❌ ESP-NOW send failed to receiver %d: %d\n", i + 1, result);
        }
    }
}




// ================= BLE RX CALLBACK =================
class RXCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::vector<uint8_t> rxValue = pCharacteristic->getValue();
        if (rxValue.size() != ESPNOW_PACKET_SIZE) return;

        Serial.print("BLE packet received: ");
        for (uint8_t b : rxValue) {
            Serial.printf("0x%02X ", b);
        }
        Serial.println();

        // HUB LED uses index 0
        switch (rxValue[0]) {
            case 0x00: LED_RGB.setPixelColor(0, LED_RGB.Color(0,0,0)); break;
            case 0x01: LED_RGB.setPixelColor(0, LED_RGB.Color(0,255,0)); break;
            case 0x02: LED_RGB.setPixelColor(0, LED_RGB.Color(0,0,255)); break;
            case 0x03: LED_RGB.setPixelColor(0, LED_RGB.Color(255,0,0)); break;
            default:   LED_RGB.setPixelColor(0, LED_RGB.Color(0,0,0)); break;
        }
        LED_RGB.show();

        // Forward to ESP-NOW players
        sendEspNowPacket(rxValue.data(), rxValue.size());
    }
};

// ================= BLE INIT =================
void initBluetooth() {
    Serial.println("Starting BLE...");

    NimBLEDevice::init("ESP32_HUB");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    pTxCharacteristic = pService->createCharacteristic(
        CHAR_UUID_TX,
        NIMBLE_PROPERTY::NOTIFY
    );

    pRxCharacteristic = pService->createCharacteristic(
        CHAR_UUID_RX,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    pRxCharacteristic->setCallbacks(new RXCallbacks());

    pTurnCommandCharacteristic = pService->createCharacteristic(
    TURN_COMMAND_UUID,
    NIMBLE_PROPERTY::NOTIFY
    );

    pService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);

    NimBLEDevice::startAdvertising();
    Serial.println("BLE Advertising started");

    initEspNow();
}

void handleBluetooth() {
    // Event-driven; nothing needed
}


void sendTurnCommand(int8_t direction) {
    if (!deviceConnected) return;

    // Only allow -1 or +1
    if (direction != -1 && direction != 1) return;

    pTurnCommandCharacteristic->setValue(
        (uint8_t*)&direction,
        1
    );
    pTurnCommandCharacteristic->notify();

    Serial.printf("🔄 Turn command sent: %d\n", direction);
}

