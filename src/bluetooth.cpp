#include "bluetooth.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

extern Adafruit_NeoPixel LED_RGB;

BLEServer* pServer = nullptr;
BLECharacteristic* pRxCharacteristic = nullptr;
bool deviceConnected = false;

// Nordic UART UUIDs
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_UUID_RX        "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_UUID_TX        "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// ----------------- Callbacks -----------------
class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        deviceConnected = true;
        Serial.println("[BLE] Client connected");
    }

    void onDisconnect(BLEServer* pServer) override {
        deviceConnected = false;
        Serial.println("[BLE] Client disconnected");
        Serial.println("[BLE] Restarting advertising...");
        BLEDevice::startAdvertising();
        Serial.println("[BLE] Advertising restarted");
    }
};

class RXCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) override {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.empty()) return;

        Serial.print("[BLE] Packet received: ");
        for (size_t i = 0; i < rxValue.size(); i++) {
            Serial.print("0x");
            Serial.print((uint8_t)rxValue[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

        // Use the first byte to update the NeoPixel
        uint8_t hubState = rxValue[0];
        switch(hubState) {
            case 0x00: LED_RGB.setPixelColor(0, LED_RGB.Color(0,0,0)); break;
            case 0x01: LED_RGB.setPixelColor(0, LED_RGB.Color(0,255,0)); break;
            case 0x02: LED_RGB.setPixelColor(0, LED_RGB.Color(0,0,255)); break;
            case 0x03: LED_RGB.setPixelColor(0, LED_RGB.Color(255,0,0)); break;
            default:   LED_RGB.setPixelColor(0, LED_RGB.Color(0,0,0)); break;
        }
        LED_RGB.show();
    }
};

// ----------------- Initialization -----------------
void initBluetooth() {
    Serial.println("[BLE] Initializing BLE device...");

    BLEDevice::init("ESP32_HUB");
    BLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_ADV);

    Serial.println("[BLE] Creating BLE server...");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    Serial.println("[BLE] Creating BLE service...");
    BLEService* pService = pServer->createService(SERVICE_UUID);

    Serial.println("[BLE] Creating RX characteristic...");
    pRxCharacteristic = pService->createCharacteristic(
        CHAR_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE
    );
    pRxCharacteristic->setCallbacks(new RXCallbacks());

    // TX characteristic for future notifications if needed
    BLECharacteristic* pTxCharacteristic = pService->createCharacteristic(
        CHAR_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pTxCharacteristic->addDescriptor(new BLE2902());

    Serial.println("[BLE] Starting service...");
    pService->start();

    Serial.println("[BLE] Setting up advertising...");
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // for Android
    pAdvertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();
    Serial.println("[BLE] ===== Advertising started =====");
}

// ----------------- BLE Handler -----------------
void handleBluetooth() {
    // Empty for now
}
