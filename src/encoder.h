#pragma once

void encoderInit();
void encoderUpdate();

// These are flags your hub can read
extern volatile int encoderDelta;   // +1 / -1 per step
extern volatile bool encoderPressed;
