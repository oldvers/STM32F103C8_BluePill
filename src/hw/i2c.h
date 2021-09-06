#ifndef __I2C_H__
#define __I2C_H__

#include "types.h"

typedef enum I2C_e
{
  I2C_1 = 0,
  I2C_2,
  I2CS_COUNT
} I2C_t;

/* Callback Function Declarations */
typedef FW_BOOLEAN (*I2C_CbComplete_t)(FW_RESULT aResult);

/* Function Declarations */
void I2C_IrqHandler (I2C_t aI2C);
void I2C_IrqError   (I2C_t aI2C);
void I2C_Init       (I2C_t aI2C, I2C_CbComplete_t pCbComplete);
//void I2C_MWr        (I2C_t aI2C, U8 aAdr, U8 * pTx, U8 txSize);
//void I2C_MRd        (I2C_t aI2C, U8 aAdr, U8 * pRx, U8 rxSize);
void I2C_MEx        (I2C_t aI2C, U8 aAdr, U8 * pTx, U8 txSize, U8 * pRx, U8 rxSize);
void I2C_DeInit     (void);

#endif /* __I2C_H__ */
