#include "rs485-board.h"
#include "board.h"

Gpio_t uart_pd;
Gpio_t uart_rx;

#define MODBUS_FRAME_SIZE 8

#define FIFO_TX_SIZE 16
#define FIFO_RX_SIZE 16

uint8_t TxBuffer[FIFO_TX_SIZE];
uint8_t RxBuffer[FIFO_RX_SIZE];

uint8_t RS485Buffer[FIFO_RX_SIZE];
uint8_t RS485Size = 0;

extern UART_HandleTypeDef UartHandle;
static TimerEvent_t rs485Timer;

static uint8_t RS485_ADDR = 0x23;

void onRS485Message(void);
static void Send_Error(uint8_t *data, uint8_t error);

uint16_t ReadReg(uint8_t *buf)
{
	return (buf[1] & 0xFF) | (buf[0] << 8);
}

uint8_t *WriteReg(uint8_t *buf, uint16_t val)
{
	buf[0] = val >> 8;
	buf[1] = val & 0xFF;
	return buf+2;
}


uint16_t ModRTU_CRC(uint8_t *buf, int len)
{
	uint32_t pos, i;
	uint16_t crc = 0xFFFF;
	
	for (pos = 0; pos < len; pos++) {
		crc ^= (uint16_t)buf[pos];         // XOR byte into least sig. byte of crc
		
		for (i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			}
			else                            // Else LSB is not set
			crc >>= 1;                    // Just shift right
		}
	}
	// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
	return crc;
}

uint16_t DecodeInt16(uint8_t *buf)
{
	return (buf[0] & 0xFF) | (buf[1] << 8);
}

uint8_t *EncodeInt16(uint8_t *data,uint16_t val)
{
	data[0] = val & 0xFF;
	data[1] = val >> 8;
	return data+2;
}


static void RS485DataIrq(UartNotifyId_t id)
{
    uint8_t data;
	  if(id == UART_NOTIFY_RX){
			if( UartGetChar( &Uart1, &data ) == 0 )
      {
				 RS485Buffer[RS485Size++] = data;
				 if(RS485Size >= MODBUS_FRAME_SIZE){
				    //read a frame
						TimerStart( &rs485Timer );
				 }
			}
	  }
}

static void OnRS485ReadTimerEvent( void )
{
    TimerStop( &rs485Timer );
		FifoFlush(&Uart1.FifoRx);
	  onRS485Message();
    RS485Size = 0;
}
/*
void GpioWakeupIrqHandler(void)
{
	  HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
	  RS485InitPort();
	
	  HAL_UART_Receive(&UartHandle, RxBuffer, 8, 100);
	  onRS485Message();
}*/

void RS485Init()
{		
		Uart1.IrqNotify = RS485DataIrq;
	  FifoInit( &Uart1.FifoRx, RxBuffer, FIFO_RX_SIZE );
	  FifoInit( &Uart1.FifoTx, TxBuffer, FIFO_TX_SIZE );
    GpioInit( &uart_pd, UART_PD, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	  RS485InitPort();
	
	  RS485_ADDR = *(__IO uint32_t*)0x08080000;
	  if(RS485_ADDR == 0x0){
		   RS485_ADDR = 0x23;
		}
		
	  TimerInit(&rs485Timer, OnRS485ReadTimerEvent);
	  TimerSetValue(&rs485Timer, 50);
}

void RS485InitPort()
{	
	/*
	  UartInit( &Uart1, UART_1, UART_TX, UART_RX);
    UartHandle.Instance = USART1;
    UartHandle.Init.BaudRate = 9600;
    UartHandle.Init.Mode = UART_MODE_TX_RX;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits = UART_STOPBITS_1;
    UartHandle.Init.Parity = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if( HAL_UART_Init( &UartHandle ) != HAL_OK )
    {
        assert_param( FAIL );
    }
	*/
	  UartInit( &Uart1, UART_1, UART_TX, UART_RX);
    UartConfig( &Uart1, RX_TX, 9600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
}

void RS485DeInit()
{
/*
	  UartDeInit(&Uart1);
    GpioSetInterrupt( &Uart1.Rx, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, GpioWakeupIrqHandler);
	*/
}

static void RS485_Send(uint8_t *data, uint8_t len)
{
	 GpioWrite(&uart_pd, 1);
	 HAL_UART_Transmit(&UartHandle, data, len, 100);
	 //UartPutBuffer(&Uart1, data, len);
	 DelayMs(100);
	 GpioWrite(&uart_pd, 0);
}

static void Send_Error(uint8_t *data, uint8_t error)
{
	uint16_t crc;
	
	data[1] += 0x80;
	data[2] = error;
	
	crc = ModRTU_CRC(data, 3);
	
	EncodeInt16(data+3, crc);
	
	RS485_Send(data, 5);
}

HAL_StatusTypeDef writeEEPROMByte(uint32_t address, uint8_t data)
 {
    HAL_StatusTypeDef  status;
    address = address + 0x08080000;
    HAL_FLASHEx_DATAEEPROM_Unlock();  //Unprotect the EEPROM to allow writing
    status = HAL_FLASHEx_DATAEEPROM_Program(TYPEPROGRAMDATA_BYTE, address, data);
    HAL_FLASHEx_DATAEEPROM_Lock();  // Reprotect the EEPROM
    return status;
}

void onRS485Message()
{
	 uint8_t *data = RS485Buffer;
	 if(RS485_ADDR == data[0]){  //rs485 addr
		 //crc
		 uint16_t crc_org = DecodeInt16(data+6);
		 uint16_t crc = ModRTU_CRC(data, 6);
		 if(crc == crc_org){
	       uint8_t *enc = RS485Buffer+2;
			   uint8_t command = data[1];
			   uint16_t startReg = ReadReg(data+2);
			   uint16_t regValue = ReadReg(data+4);
			   			 		   
			   if(command == 0x3){
					  uint16_t readVal = 0;
					  
					  if(startReg > 4){
				       Send_Error(data, 0x2);
					     return;
				    }
				  
				    if(regValue < 1 || (regValue+startReg) > 5){
					     Send_Error(data, 0x3);
					     return;
				    }
						
					  //SHT3XReadData();
					 
						*enc++ = regValue * 2;
					  while(regValue--){
						    readVal = 0x0;
							  switch(startReg){
									case 1:
									{
										 readVal = LEDReadState();
										 break;
									}
									case 2:
									{
										 readVal = LEDReadBRT();
										 break;
									}
									case 3:
									{
										 readVal = SHT3XReadTemperature( ) * 100;
										 break;
									}
									case 4:
									{
										 readVal = SHT3XReadHumidity();
										 break;
									}
								}
							
							  startReg++;
							  enc = WriteReg(enc, readVal);
					  }
						
				    crc = ModRTU_CRC(RS485Buffer, enc - RS485Buffer);
				    enc = EncodeInt16(enc, crc);
				    RS485_Send(RS485Buffer, enc - RS485Buffer);			
				 }else if(command == 0x6){
						    //write
					  if(startReg == 0x10){
							  RS485_ADDR = regValue;
							  //eeprom
							  writeEEPROMByte(0, RS485_ADDR);
						}else{
							  if(startReg > 5){
				           Send_Error(data, 0x2);
					         return;
				        }
						}
						
						if(startReg == 0x1){
						   LEDSetState(regValue == 0x1);
						}else if(startReg == 0x2){
						   LEDSetBRT(regValue);
						}
						
						*enc++ = 0x2;
				    enc = WriteReg(enc, regValue);				
				    crc = ModRTU_CRC(RS485Buffer, enc - RS485Buffer);
				    enc = EncodeInt16(enc, crc);
				    RS485_Send(RS485Buffer, enc - RS485Buffer);
				 }else{
						Send_Error(data, 0x1);
				 }
		 }else{
			   Send_Error(data, 0x3);
		 }
	}
}

