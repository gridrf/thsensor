#ifndef __LED_BOARD_H__
#define __LED_BOARD_H__

#include <stdint.h>

void LEDInit(void);
uint8_t LEDReadBRT(void);
void LEDSetBRT(uint8_t brt);
uint8_t LEDReadState(void);
void LEDSetState(uint8_t state);
void LEDSetLocate(int idx);
void LEDSetDot(uint8_t idx);
void LEDPutChar(int charInt);
void LEDClear(void);

#endif
