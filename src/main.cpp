#include <Arduino.h>
#include <Adafruit_Neopixel.h>
#include "bluetooth.h"

Adafruit_NeoPixel LED_RGB(1, 48, NEO_GRBW + NEO_KHZ800);

void setup() {
    Serial.begin(115200);
    delay(1000); // wait for serial monitor
    LED_RGB.begin();
    LED_RGB.setBrightness(70);

    initBluetooth();
    Serial.println("ESP32 HUB Starting...");

    // 2️⃣ Give BLE controller time to stabilize
    delay(100);

    // 3️⃣ Start ESP-NOW second
    initEspNow();

    Serial.println("ESP32 HUB Ready");

}

void loop() {
    handleBluetooth(); // needed for future updates, empty for now
}
