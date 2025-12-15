#include <Arduino.h>
#include <Adafruit_Neopixel.h>
#include "bluetooth.h"


Adafruit_NeoPixel LED_RGB(1, 48, NEO_GRBW + NEO_KHZ800);

void setup() {
    Serial.begin(115200);
    LED_RGB.begin();
    LED_RGB.setBrightness(70);

    initBluetooth();
}

void loop() {
    handleBluetooth(); // needed for future updates, empty for now
}
