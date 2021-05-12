#ifndef __NRF24L01P__
#define __NRF24L01P__

#include "types.h"
#include "board.h"
#include "gpio.h"

/* Callback Function Declarations */
typedef void (*nRF24L01P_CbIrq)(void);

/* CE (Chip Enable) */
#define nRF24L01P_CE_Lo()         GPIO_Lo(NRF24_CE_PORT, NRF24_CE_PIN)
#define nRF24L01P_CE_Hi()         GPIO_Hi(NRF24_CE_PORT, NRF24_CE_PIN)

/* CSN (Chip Select Negative) */
#define nRF24L01P_CSN_Lo()        GPIO_Lo(NRF24_CSN_PORT, NRF24_CSN_PIN)
#define nRF24L01P_CSN_Hi()        GPIO_Hi(NRF24_CSN_PORT, NRF24_CSN_PIN)

void nRF24L01P_Init(nRF24L01P_CbIrq pCbIrq);
void nRF24L01P_Exchange(U8 * txData, U8 * rxData, U32 aSize);
void nRF24L01P_IrqHandler(void);

#endif /* __NRF24L01P__ */
