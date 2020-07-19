#ifndef __UART_H__
#define __UART_H__

#include "stm32f1xx.h"
#include "types.h"

typedef enum UART_STOPBITS_e
{
  UART_STOPBITS_1 = 0,
  UART_STOPBITS_2 = USART_CR2_STOP_1,
} UART_STOPBITS_t;

typedef enum UART_PARITY_e
{
  UART_PARITY_NONE = (0),
  UART_PARITY_EVEN = (USART_CR1_PCE),
  UART_PARITY_ODD  = (USART_CR1_PCE | USART_CR1_PS),
} UART_PARITY_t;

typedef enum UART_e
{
  UART1 = 0,
  UART2,
  UART3,
  UARTS_COUNT
} UART_t;

/* Callback Function Declarations */
typedef FW_BOOLEAN (*UART_CbByte)(U8 * pByte);

/* Function Declarations */
void UART_Init
(
  UART_t      aUART,
  U32         aBaudRate,
  UART_CbByte pRxByteCb,
  UART_CbByte pTxByteCb
);
void UART_DeInit      (UART_t aUART);
void UART_TxStart     (UART_t aUART);
void UART_RxStart     (UART_t aUART);
void UART_SetBaudrate (UART_t aUART, U32 aValue);

/* Interrupt Handler Declaration */
void UART_IRQHandler (UART_t aUART);

#endif  /* __UART_H__ */
