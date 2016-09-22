#include "sccb.h"




#define SCCB_ADDR   			(0x42>>1) 		//OV7725(0x42 for write and 0x43 for read)

uint8_t sccb_Init(void)
{
	i2cInit();
	return 0;
}


bool i2cRead(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);
	bool i2cWrite(uint8_t addr, uint8_t reg, uint8_t data);



//SCCB读一位数据(两相写加两相读)
uint8_t sccb_ReadByte(uint8_t reg,uint8_t *data_ptr)
{
	if(i2cWriteBuffer(SCCB_ADDR,reg,0,NULL) != true)
		return 1;
	if(i2cRead(SCCB_ADDR,reg,1,data_ptr) != true)
		return 1;
	return 0;
}

//SCCB写一位数据（三相写）
uint8_t sccb_WriteByte(uint8_t reg,uint8_t data)
{
	if(i2cWrite(SCCB_ADDR,reg,data)==true)
		return 0;
	else
		return 0;
}
