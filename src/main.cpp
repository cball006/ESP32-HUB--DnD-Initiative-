#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "bluetooth.h"

#define LED_PIN 48
Adafruit_NeoPixel LED_RGB(1, LED_PIN, NEO_GRBW + NEO_KHZ800);

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("ESP32 HUB Starting...");

    LED_RGB.begin();
    LED_RGB.setBrightness(70);
    LED_RGB.setPixelColor(0, LED_RGB.Color(0,0,0));
    LED_RGB.show();

    initBluetooth();

}

void loop() {
    handleBluetooth();
}

    