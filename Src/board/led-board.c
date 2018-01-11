#include "led-board.h"
#include "stm32_tm1637.h"

static uint8_t LED_BRT = TM1637_BRT0;
static TM1637_t tm1637;

void LEDInit(void)
{
		tm1637Init(&tm1637);
	  tm1637_clear();
    tm1637_setBrightness(&tm1637, LED_BRT);
	  tm1637_cls(&tm1637, true);
}

uint8_t LEDReadBRT(void)
{
	 return LED_BRT;
}

void LEDSetBRT(uint8_t brt)
{
	 LED_BRT = brt;
   tm1637_setBrightness(&tm1637, LED_BRT);
}

uint8_t LEDReadState(void)
{
	 return tm1637.display;
}

void LEDSetState(uint8_t state)
{
	 tm1637_setDisplay(&tm1637, state);
}

void LEDSetLocate(int idx)
{
     tm1637_locate(&tm1637, idx);
}

void LEDSetDot(uint8_t idx)
{
	   Icon dots[] = { DP1, DP2, DP3, DP4, DP5, DP6 };
     tm1637_setIcon(&tm1637, dots[idx]);
}

void LEDClear(void)
{
	  tm1637_cls(&tm1637, true);
}

void LEDPutChar(int charInt)
{
	  tm1637_putc(&tm1637,  charInt);
}
