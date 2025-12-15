#include "bluetooth.h"
#include <NimBLEDevice.h>
#include <Adafruit_NeoPixel.h>

extern Adafruit_NeoPixel LED_RGB;

NimBLEServer* pServer;
NimBLECharacteristic* pTxCharacteristic;
NimBLECharacteristic* pRxCharacteristic;
bool deviceConnected = false;

// Nordic UART UUIDs
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) override {
        deviceConnected = true;
        Serial.println("BLE client connected");
    }

    void onDisconnect(NimBLEServer* pServer) override {
        deviceConnected = false;
        Serial.println("BLE client disconnected");

        // 🔑 Restart advertising after disconnect
        NimBLEDevice::startAdvertising();
        Serial.println("Advertising restarted");
    }
};

class RXCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::vector<uint8_t> rxValue = pCharacteristic->getValue();
        if (rxValue.empty()) return;

        uint8_t hubState = rxValue[0];
        Serial.print("Hub LED state: 0x");
        Serial.println(hubState, HEX);

        switch (hubState) {
            case 0x00: LED_RGB.setPixelColor(0, LED_RGB.Color(0,0,0)); break;
            case 0x01: LED_RGB.setPixelColor(0, LED_RGB.Color(0,255,0)); break;
            case 0x02: LED_RGB.setPixelColor(0, LED_RGB.Color(0,0,255)); break;
            case 0x03: LED_RGB.setPixelColor(0, LED_RGB.Color(255,0,0)); break;
            default:   LED_RGB.setPixelColor(0, LED_RGB.Color(0,0,0)); break;
        }
        LED_RGB.show();
    }
};

void initBluetooth() {
    Serial.println("Starting BLE...");

    NimBLEDevice::init("ESP32_HUB");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); // Strong signal

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

    pService->start();

    // 🔑 CORRECT advertising setup
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);     // REQUIRED
    pAdvertising->setMinPreferred(0x06);     // Android compatibility
    pAdvertising->setMinPreferred(0x12);

    NimBLEDevice::startAdvertising();

    Serial.println("BLE Advertising started");
}

void handleBluetooth() {
    // Nothing needed yet
}
