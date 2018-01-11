#include "stm32l1xx_hal.h"

#include "stm32_tm1637.h"

// Configuration.

#define CLK_PORT GPIOB
#define DIO_PORT GPIOB
#define CLK_PIN GPIO_PIN_10
#define DIO_PIN GPIO_PIN_11
#define CLK_PORT_CLK_ENABLE __HAL_RCC_GPIOB_CLK_ENABLE
#define DIO_PORT_CLK_ENABLE __HAL_RCC_GPIOB_CLK_ENABLE


void _tm1637DelayUsec(unsigned int i)
{
    for (; i>0; i--) {
        for (int j = 0; j < 10; ++j) {
            __asm("nop");
        }
    }
}

void _tm1637ClkHigh(void)
{
    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_SET);
}

void _tm1637ClkLow(void)
{
    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_RESET);
}

void _tm1637DioHigh(void)
{
    HAL_GPIO_WritePin(DIO_PORT, DIO_PIN, GPIO_PIN_SET);
}

void _tm1637DioLow(void)
{
    HAL_GPIO_WritePin(DIO_PORT, DIO_PIN, GPIO_PIN_RESET);
}

uint8_t _tm1637DioRead(void)
{
   return HAL_GPIO_ReadPin(DIO_PORT, DIO_PIN);
}

/** Generate Start condition for TM1637
  *  @param  none
  *  @return none
  */ 
void tm1637_start() {

  _tm1637DioLow();
  _tm1637DelayUsec(1);
  _tm1637ClkLow();
  _tm1637DelayUsec(1);
}
  
/** Generate Stop condition for TM1637
  *  @param  none
  *  @return none
  */ 
void tm1637_stop() {

  _tm1637DioLow();
  _tm1637DelayUsec(1);  
  _tm1637ClkHigh();
  _tm1637DelayUsec(1);
  _tm1637DioHigh();
  _tm1637DelayUsec(1);
}

/** Send byte to TM1637
  *  @param  int data
  *  @return none
  */ 
void tm1637_write(int data) {
 
  for (int bit=0; bit<8; bit++) {    
    //The TM1637 expects LSB first
    if (((data >> bit) & 0x01) == 0x01) {
      _tm1637DioHigh();      
    }
    else {    
			_tm1637DioLow();     
    }  
    _tm1637DelayUsec(1);
    _tm1637ClkHigh();
    _tm1637DelayUsec(1);
    _tm1637ClkLow();  
    _tm1637DelayUsec(1);
  }  

  _tm1637DioHigh();
  
  // Prepare DIO to read data
  //_dio.input();
  _tm1637DelayUsec(3);
      
  // dummy Ack
  _tm1637ClkHigh();
  _tm1637DelayUsec(1);
//  _ack = _dio;  
  _tm1637ClkLow();  
  _tm1637DelayUsec(1); 
  
  // Return DIO to output mode
  //_dio.output();  
  _tm1637DelayUsec(3);  

  _tm1637DioHigh(); //idle  
}

/** Read byte from TM1637
  *  @return read byte 
  */ 
char tm1637_read() {
  char keycode = 0;

  // Prepare DIO to read data
  //_dio.input();
  _tm1637DelayUsec(3);
    
  for (int bit=0; bit<8; bit++) {    
   
    //The TM1637 sends bitpattern: S0 S1 S2 K1 K2 1 1 1
    //Data is shifted out by the TM1637 on the falling edge of CLK
    //Observe sufficient delay to allow the Open Drain DIO to rise to H levels
    // Prepare to read next bit, LSB (ie S0) first. 
    // The code below flips bits for easier matching with datasheet
    keycode = keycode << 1;  

    _tm1637ClkHigh();
    _tm1637DelayUsec(1);
    
    // Read next bit
    if (_tm1637DioRead()) { keycode |= 0x01; }        

    _tm1637ClkLow();        
    _tm1637DelayUsec(5); // Delay to allow for slow risetime
  }  
  
  // Return DIO to output mode
  //_dio.output();
  _tm1637DelayUsec(3);  

  // dummy Ack
  _tm1637DioLow(); //Ack   
  _tm1637DelayUsec(1);
  
  _tm1637ClkHigh();
  _tm1637DelayUsec(1);
  _tm1637ClkLow();  
  _tm1637DelayUsec(1); 

  _tm1637DioHigh(); //idle

  return keycode;
}

/** Write command and parameter to TM1637
  *  @param  int cmd Command byte
  *  &Param  int data Parameters for command
  *  @return none
  */
void tm1637_writeCmd(int cmd, int data){
    
  tm1637_start();

  tm1637_write((cmd & TM1637_CMD_MSK) | (data & ~TM1637_CMD_MSK));   
 
  tm1637_stop();          
} 

void tm1637Init(TM1637_t *tm1637)
{
    CLK_PORT_CLK_ENABLE();
    DIO_PORT_CLK_ENABLE();
    GPIO_InitTypeDef g = {0};
    g.Pull = GPIO_PULLUP;
    g.Mode = GPIO_MODE_OUTPUT_OD; // OD = open drain
    g.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    g.Pin = CLK_PIN;
    HAL_GPIO_Init(CLK_PORT, &g);
    g.Pin = DIO_PIN;
    HAL_GPIO_Init(DIO_PORT, &g);

		//init controller  
    tm1637->display = TM1637_DSP_ON;
    tm1637->bright  = TM1637_BRT_DEF;
		tm1637->column  = 0;
    tm1637->columns = ROBOTDYN_NR_DIGITS;
		
    tm1637_writeCmd(TM1637_DSP_CTRL_CMD, tm1637->display | tm1637->bright );                                 // Display control cmd, display on/off, brightness   

    tm1637_writeCmd(TM1637_DATA_SET_CMD, TM1637_DATA_WR | TM1637_ADDR_INC | TM1637_MODE_NORM); // Data set cmd, normal mode, auto incr, write data 
}

/** Clear the screen and locate to 0
 */  
void tm1637_clear()
{
  tm1637_start();  

  tm1637_write(TM1637_ADDR_SET_CMD | 0x00); // Address set cmd, 0      
  for (int cnt=0; cnt<TM1637_DISPLAY_MEM; cnt++) {
    tm1637_write(0x00); // data 
  }

  tm1637_stop();  
}  

/** Set Brightness
  *
  * @param  char brightness (3 significant bits, valid range 0..7 (1/16 .. 14/14 dutycycle)  
  * @return none
  */
void tm1637_setBrightness(TM1637_t *tm1637, char brightness)
{

  tm1637->bright = brightness & TM1637_BRT_MSK; // mask invalid bits
  
  tm1637_writeCmd(TM1637_DSP_CTRL_CMD, tm1637->display | tm1637->bright );  // Display control cmd, display on/off, brightness  
}

/** Set the Display mode On/off
  *
  * @param bool display mode
  */
void tm1637_setDisplay(TM1637_t *tm1637, bool on) 
{
  
  if (on) {
    tm1637->display = TM1637_DSP_ON;
  }
  else {
    tm1637->display = TM1637_DSP_OFF;
  }
  
  tm1637_writeCmd(TM1637_DSP_CTRL_CMD, tm1637->display | tm1637->bright );  // Display control cmd, display on/off, brightness   
}
/** Write Display datablock to TM1637
  *  @param  DisplayData_t data Array of TM1637_DISPLAY_MEM (=16) bytes for displaydata
  *  @param  length number bytes to write (valid range 0..(TM1637_MAX_NR_GRIDS * TM1637_BYTES_PER_GRID) (=16), when starting at address 0)  
  *  @param  int address display memory location to write bytes (default = 0) 
  *  @return none
  */  
void tm1637_writeData(char *data, int length, int address) {

  tm1637_start();

// sanity check
  address &= TM1637_ADDR_MSK;
  if (length < 0) {length = 0;}
  if ((length + address) > TM1637_DISPLAY_MEM) {length = (TM1637_DISPLAY_MEM - address);}
    
//  _write(TM1637_ADDR_SET_CMD | 0x00); // Set Address at 0
  tm1637_write(TM1637_ADDR_SET_CMD | address); // Set Address
  
  for (int idx=0; idx<length; idx++) {    
//    _write(data[idx]); // data 
    tm1637_write(data[address + idx]); // data 
  }
  
  tm1637_stop();  
}


/** Locate cursor to a screen column
  *
  * @param column  The horizontal position from the left, indexed from 0
  */
void tm1637_locate(TM1637_t *tm1637, int column) {
  //sanity check
  if (column < 0) {column = 0;}
  if (column > (tm1637->columns - 1)) {column = tm1637->columns - 1;}  
  
  tm1637->column = column;       
}
    
/** Clear the screen and locate to 0
  * @param bool clrAll Clear Icons also (default = false)
  */ 
void tm1637_cls(TM1637_t *tm1637, bool clrAll) {  

  if (clrAll) {
    //clear local buffer (including Icons)
    for (int idx=0; idx < ROBOTDYN_NR_GRIDS; idx++) {
      tm1637->displaybuffer[idx] = 0x00;  
    }
  }  
  else {
    //clear local buffer (preserving Icons)
    for (int idx=0; idx < ROBOTDYN_NR_GRIDS; idx++) {
      tm1637->displaybuffer[idx] = tm1637->displaybuffer[idx] & MASK_ICON_GRID[idx];  
    }  
  }

  tm1637_writeData(tm1637->displaybuffer, (ROBOTDYN_NR_GRIDS * TM1637_BYTES_PER_GRID), 0);

  tm1637->column = 0;   
}     

/** Set Icon
  *
  * @param Icon icon Enums Icon has Grid position encoded in 8 MSBs, Icon pattern encoded in 16 LSBs
  * @return none
  */
void tm1637_setIcon(TM1637_t *tm1637, Icon icon) 
{
  int addr, icn;

   icn =        icon  & 0xFFFF;
  addr = (icon >> 24) & 0xFF; 
  addr = (addr - 1);
    
  //Save char...and set bits for icon to write
  tm1637->displaybuffer[addr] = tm1637->displaybuffer[addr] | LO(icn);      
//  writeData(_displaybuffer, (ROBOTDYN_NR_GRIDS * TM1637_BYTES_PER_GRID), 0);
  tm1637_writeData(tm1637->displaybuffer, TM1637_BYTES_PER_GRID, addr);  
}

/** Clr Icon
  *
  * @param Icon icon Enums Icon has Grid position encoded in 8 MSBs, Icon pattern encoded in 16 LSBs
  * @return none
  */
void tm1637_clrIcon(TM1637_t *tm1637,Icon icon) 
{
  int addr, icn;

   icn =        icon  & 0xFFFF;
  addr = (icon >> 24) & 0xFF; 
  addr = (addr - 1);
    
  //Save char...and clr bits for icon to write
  tm1637->displaybuffer[addr] = tm1637->displaybuffer[addr] & ~LO(icn);      
//  writeData(_displaybuffer, (ROBOTDYN_NR_GRIDS * TM1637_BYTES_PER_GRID), 0);
  tm1637_writeData(tm1637->displaybuffer, TM1637_BYTES_PER_GRID, addr);    
}


/** Set User Defined Characters (UDC)
  *
  * @param unsigned char udc_idx  The Index of the UDC (0..7)
  * @param int udc_data           The bitpattern for the UDC (8 bits)       
  */
void tm1637_setUDC(TM1637_t *tm1637, unsigned char udc_idx, int udc_data) 
{

  //Sanity check
  if (udc_idx > (ROBOTDYN_NR_UDC-1)) {
    return;
  }
  // Mask out Icon bits?

  tm1637->UDC_7S[udc_idx] = LO(udc_data);
}

/** Write a single character (Stream implementation)
  *
  */
int tm1637_putc(TM1637_t *tm1637, int value) 
{
//The ROBOTDYN mapping between Digit positions (Left to Right) and Grids is:
//  GR3 GR2 GR1 GR6 GR5 GR4
//The memory addresses or column numbers are:
//   2   1   0   5   4   3
//The Grids are reversed for 2 sets of 3 digits: 
    const int col2addr[] = {0, 1, 2, 3, 4, 5};

    int addr;
    bool validChar = false;
    char pattern   = 0x00;
    
    if ((value == '\n') || (value == '\r')) {
      //No character to write
      validChar = false;
      
      //Update Cursor      
      tm1637->column = 0;
    }
    else if ((value == '.') || (value == ',')) {
      //No character to write
      validChar = false;
      pattern = S7_DP; // placeholder for all DPs
      
      // Check to see that DP can be shown for current column
      if (tm1637->column > 0) {
        //Translate between _column and displaybuffer entries
        //Add DP to bitpattern of digit left of current column.
        addr = col2addr [tm1637->column - 1];
      
        //Save icons...and set bits for decimal point to write
        tm1637->displaybuffer[addr] = tm1637->displaybuffer[addr] | pattern;
//        writeData(_displaybuffer, (ROBOTDYN_NR_GRIDS * TM1637_BYTES_PER_GRID));
        tm1637_writeData(tm1637->displaybuffer, TM1637_BYTES_PER_GRID, addr); 
        
        //No Cursor Update
      }
    }
    else if ((value >= 0) && (value < ROBOTDYN_NR_UDC)) {
      //Character to write
      validChar = true;
      pattern = tm1637->UDC_7S[value];
    }  
    
#if (SHOW_ASCII == 1)
    //display all ASCII characters
    else if ((value >= FONT_7S_START) && (value <= FONT_7S_END)) {   
      //Character to write
      validChar = true;
      pattern = FONT_7S[value - FONT_7S_START];
    } // else
#else    
    //display only digits and hex characters      
    else if (value == '-') {
      //Character to write
      validChar = true;
      pattern = C7_MIN;         
    }
    else if ((value >= (int)'0') && (value <= (int) '9')) {   
      //Character to write
      validChar = true;
      pattern = FONT_7S[value - (int) '0'];
    }
    else if ((value >= (int) 'A') && (value <= (int) 'F')) {   
      //Character to write
      validChar = true;
      pattern = FONT_7S[10 + value - (int) 'A'];
    }
    else if ((value >= (int) 'a') && (value <= (int) 'f')) {   
      //Character to write
      validChar = true;
      pattern = FONT_7S[10 + value - (int) 'a'];
    } //else
#endif

    if (validChar) {
      //Character to write
 
      //Translate between _column and displaybuffer entries
      addr = col2addr[tm1637->column];

      //Save icons...and set bits for character to write
      tm1637->displaybuffer[addr] = (tm1637->displaybuffer[addr] & MASK_ICON_GRID[tm1637->column]) | pattern;

//      writeData(_displaybuffer, (ROBOTDYN_NR_GRIDS * TM1637_BYTES_PER_GRID));
      tm1637_writeData(tm1637->displaybuffer, TM1637_BYTES_PER_GRID, addr);        
                                
      //Update Cursor
      tm1637->column++;
      if (tm1637->column > (ROBOTDYN_NR_DIGITS - 1)) {
        tm1637->column = 0;
      }

    } // if validChar           

    return value;
}

