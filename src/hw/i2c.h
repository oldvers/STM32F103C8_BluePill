#ifndef __I2CROUTINES_H
#define __I2CROUTINES_H

#include "stm32f1xx.h"
#include "types.h"

U32 I2C_MasterRead(I2C_TypeDef * pI2C, U8 aAddress, U8 * pBuffer, U32 aSize);
U32 I2C_MasterWrite(I2C_TypeDef * pI2C, U8 aAddress, U8 * pBuffer, U32 aSize);
void I2C_Init(I2C_TypeDef * pI2C);
void I2C_EV_IRQHandler(I2C_TypeDef * pI2C);
void I2C_ER_IRQHandler(I2C_TypeDef * pI2C);

#endif
