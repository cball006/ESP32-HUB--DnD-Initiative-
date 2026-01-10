#include <Arduino.h>
#include "encoder.h"

#define ENC_A   5 // GPIO pins
#define ENC_B   4 // GPIO pins
#define ENC_SW  6 // GPIO pins

volatile int encoderDelta = 0;
volatile bool encoderPressed = false;

static int8_t stepAccumulator = 0;

// Encoder state table
static uint8_t lastState = 0;

void encoderInit() {
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  lastState = (digitalRead(ENC_A) << 1) | digitalRead(ENC_B);
}

void encoderUpdate() {
  // ----- ROTATION -----
  uint8_t state = (digitalRead(ENC_A) << 1) | digitalRead(ENC_B);

  int8_t movement = 0;

  switch ((lastState << 2) | state) {
    case 0b0001:
    case 0b0111:
    case 0b1110:
    case 0b1000:
      movement = +1;
      break;

    case 0b0010:
    case 0b0100:
    case 0b1101:
    case 0b1011:
      movement = -1;
      break;

    default:
      movement = 0; // bounce / invalid
  }

  lastState = state;
  stepAccumulator += movement;

if (stepAccumulator >= 4) {
  encoderDelta++;
  stepAccumulator = 0;
} 
else if (stepAccumulator <= -4) {
  encoderDelta--;
  stepAccumulator = 0;
}


  // ----- Button -----
  static unsigned long lastButtonTime = 0;
  if (digitalRead(ENC_SW) == LOW) {
    if (millis() - lastButtonTime > 200) {
      encoderPressed = true;
      lastButtonTime = millis();
    }
  }
}

