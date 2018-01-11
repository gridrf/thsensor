#include "sht3x-board.h"
#include "board.h"
#include "i2c-board.h"

static uint16_t temperature = 0;
static uint16_t humidity = 0;

void SHT3XReadData(void)
{
	  uint8_t alldata[6] = {0};
    I2cSetAddrSize( &I2c, I2C_ADDR_SIZE_16 );
		if( I2cReadBuffer( &I2c, 0x44 << 1, (0x2C << 8) | 0x06, alldata, 6 ) == SUCCESS )
		{
			   temperature = (alldata[0] << 8) | alldata[1];
			   humidity = (alldata[3]<<8) | alldata[4];
		}
}

float SHT3XReadTemperature(void)
{
		 float cTemp = -45 + (175 * temperature / 65535.0);
	   return cTemp;
}

float SHT3XReadHumidity(void)
{
     float _humidity = 100 * humidity / 65535.0;
	   return _humidity;
}
