#ifndef SCCB_H
#define SCCB_H

#include "stm32f4xx_hal.h"
#include "softI2C.h"

uint8_t sccb_Init(void);

uint8_t sccb_WriteByte(uint8_t reg,uint8_t data);
uint8_t sccb_ReadByte(uint8_t reg,uint8_t *data_ptr);


#endif
