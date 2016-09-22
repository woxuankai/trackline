#ifndef SOFTI2C_H
#define SOFTI2C_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

void i2cInit(void);

bool i2cWriteBuffer(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data);

bool i2cWrite(uint8_t addr, uint8_t reg, uint8_t data);

bool i2cRead(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);

#endif //SOFTI2C_H
