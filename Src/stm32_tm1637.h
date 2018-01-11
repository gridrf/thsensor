#ifndef __STM32_TM1637_H__
#define __STM32_TM1637_H__

#include <stdint.h>
#include <stdbool.h>
#include "Font_7Seg.h"

#define ROBOTDYN_NR_GRIDS  6
#define ROBOTDYN_NR_DIGITS 6
#define ROBOTDYN_NR_UDC    8

//TM1637 Display data
#define TM1637_MAX_NR_GRIDS    6
#define TM1637_BYTES_PER_GRID  1

//Significant bits Keymatrix data
//#define TM1638_KEY_MSK      0xFF 

//Memory size in bytes for Display and Keymatrix
#define TM1637_DISPLAY_MEM  (TM1637_MAX_NR_GRIDS * TM1637_BYTES_PER_GRID)
#define TM1637_KEY_MEM         2

//Reserved bits for commands
#define TM1637_CMD_MSK      0xC0

//Data setting commands
#define TM1637_DATA_SET_CMD 0x40
#define TM1637_DATA_WR      0x00
#define TM1637_KEY_RD       0x02
#define TM1637_ADDR_INC     0x00
#define TM1637_ADDR_FIXED   0x04
#define TM1637_MODE_NORM    0x00
#define TM1637_MODE_TEST    0x08

//Address setting commands
#define TM1637_ADDR_SET_CMD 0xC0
#define TM1637_ADDR_MSK     0x07 //0..5

//Display control commands
#define TM1637_DSP_CTRL_CMD 0x80
#define TM1637_BRT_MSK      0x07
#define TM1637_BRT0         0x00 //Pulsewidth 1/16
#define TM1637_BRT1         0x01
#define TM1637_BRT2         0x02
#define TM1637_BRT3         0x03
#define TM1637_BRT4         0x04
#define TM1637_BRT5         0x05
#define TM1637_BRT6         0x06
#define TM1637_BRT7         0x07 //Pulsewidth 14/16

#define TM1637_BRT_DEF      TM1637_BRT3

#define TM1637_DSP_OFF      0x00
#define TM1637_DSP_ON       0x08


//Access to 16 Switches
//S0 S1 S2 K1 K2 1 1 1
//K1,K2 = 0 1
#define TM1637_SW1_BIT      0xEF
#define TM1637_SW2_BIT      0x6F
#define TM1637_SW3_BIT      0xAF
#define TM1637_SW4_BIT      0x2F
#define TM1637_SW5_BIT      0xCF
#define TM1637_SW6_BIT      0x4F
#define TM1637_SW7_BIT      0x8F
#define TM1637_SW8_BIT      0x0F

//K1,K2 = 1 0
#define TM1637_SW9_BIT      0xF7
#define TM1637_SW10_BIT     0x77
#define TM1637_SW11_BIT     0xB7
#define TM1637_SW12_BIT     0x37
#define TM1637_SW13_BIT     0xD7
#define TM1637_SW14_BIT     0x57
#define TM1637_SW15_BIT     0x97
#define TM1637_SW16_BIT     0x17

#define TM1637_SW_NONE      0xFF
	
	
typedef enum{
    DP1   = ( 1<<24) | S7_DP1,  /**<  Digit 1 */
    DP2   = ( 2<<24) | S7_DP2,  /**<  Digit 2 */
    DP3   = ( 3<<24) | S7_DP3,  /**<  Digit 3 */
    DP4   = ( 4<<24) | S7_DP4,  /**<  Digit 4 */
    DP5   = ( 5<<24) | S7_DP5,  /**<  Digit 5 */
    DP6   = ( 6<<24) | S7_DP6,  /**<  Digit 6 */   
}Icon;

typedef struct
{
    char display;
    char bright;
	  char column;
    int  columns;
	  char displaybuffer[TM1637_DISPLAY_MEM];
    char UDC_7S[ROBOTDYN_NR_UDC]; 
}TM1637_t;

void tm1637Init(TM1637_t *tm1637);
void tm1637_cls(TM1637_t *tm1637, bool clrAll);
void tm1637_setIcon(TM1637_t *tm1637, Icon icon);
void tm1637_clrIcon(TM1637_t *tm1637, Icon icon);
void tm1637_setUDC(TM1637_t *tm1637, unsigned char udc_idx, int udc_data);
int tm1637_putc(TM1637_t *tm1637, int value);
void tm1637_writeData(char *data, int length, int address);
void tm1637_clear(void);
void tm1637_setBrightness(TM1637_t *tm1637, char brightness);
void tm1637_setDisplay(TM1637_t *tm1637, bool on);
void tm1637_locate(TM1637_t *tm1637, int column);

#endif
