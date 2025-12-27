#include <Arduino.h>
#include <Adafruit_Neopixel.h>
#include "bluetooth.h"
#include "encoder.h"

Adafruit_NeoPixel LED_RGB(1, 48, NEO_GRBW + NEO_KHZ800);
int menuIndex = 0;

void setup() {
    Serial.begin(115200);
    delay(1000); // wait for serial monitor
    LED_RGB.begin();


    initBluetooth();
    Serial.println("ESP32 HUB Starting...");

    // 2️⃣ Give BLE controller time to stabilize
    delay(100);

    // 3️⃣ Start ESP-NOW second
    initEspNow();

    Serial.println("ESP32 HUB Ready");

    encoderInit();

    LED_RGB.setPixelColor(0, LED_RGB.Color(0,255,255)); // Cyan for ready
    LED_RGB.show();
    
}

void loop() {
    handleBluetooth(); // needed for future updates, empty for now

    encoderUpdate();

  if (encoderDelta != 0) {
    menuIndex += encoderDelta;
    encoderDelta = 0;

    Serial.print("Menu index: ");
    Serial.println(menuIndex);
  }

  if (encoderPressed) {
    encoderPressed = false;
    Serial.println("Encoder button pressed");
    // send ESP-NOW command here
  }
}

