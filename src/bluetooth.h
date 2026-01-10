#ifndef BLUETOOTH_H
#define BLUETOOTH_H
#include <stdint.h>


void initBluetooth();
void initEspNow();
void handleBluetooth();
void sendTurnCommand(int8_t direction);

#endif
