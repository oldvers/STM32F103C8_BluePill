#ifndef __SPI_H__
#define __SPI_H__

#include "types.h"
#include "board.h"
#include "gpio.h"

///* Callback Function Declarations */
//typedef void (*nRF24L01P_CbIrq)(void);

///* CE (Chip Enable) */
//#define nRF24L01P_CE_Lo()         GPIO_Lo(NRF24_CE_PORT, NRF24_CE_PIN)
//#define nRF24L01P_CE_Hi()         GPIO_Hi(NRF24_CE_PORT, NRF24_CE_PIN)

///* CSN (Chip Select Negative) */
//#define nRF24L01P_CSN_Lo()        GPIO_Lo(NRF24_CSN_PORT, NRF24_CSN_PIN)
//#define nRF24L01P_CSN_Hi()        GPIO_Hi(NRF24_CSN_PORT, NRF24_CSN_PIN)

typedef enum SPI_e
{
  SPI_1 = 0,
  SPI_2,
  SPI_COUNT
} SPI_t;

/* Callback Function Declarations */
typedef FW_BOOLEAN (*SPI_CbComplete_t)(FW_RESULT aResult);

/* Function Declarations */
void SPI_IrqHandler (SPI_t aSPI);
//void I2C_IrqError   (I2C_t aI2C);
void SPI_Init       (SPI_t aSPI, SPI_CbComplete_t pCbComplete);
//void I2C_MWr        (I2C_t aI2C, U8 aAdr, U8 * pTx, U8 txSize);
//void I2C_MRd        (I2C_t aI2C, U8 aAdr, U8 * pRx, U8 rxSize);
void SPI_MExchange  (SPI_t aSPI, U8 * pTx, U8 * pRx, U32 aSize);
void SPI_DeInit     (SPI_t aSPI);

//void SPI_Init(nRF24L01P_CbIrq pCbIrq);
//void SPI_Exchange(U8 * txData, U8 * rxData, U32 aSize);
//void nRF24L01P_IrqHandler(void);

FW_BOOLEAN SPI_SetBaudratePrescaler(SPI_t aSPI, U16 value);

#endif /* __SPI_H__ */
