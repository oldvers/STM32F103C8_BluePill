#ifndef __BOARD_H__
#define __BOARD_H__

/* UART1: PA9 - Tx, PA10 - Rx, DTR - PB8, RTS - PB6 */
#define UART1_TX_PORT        GPIOA
#define UART1_TX_PIN         9
#define UART1_RX_PORT        GPIOA
#define UART1_RX_PIN         10
#define UART1_DTR_PORT       GPIOB
#define UART1_DTR_PIN        8
#define UART1_RTS_PORT       GPIOB
#define UART1_RTS_PIN        6

/* UART2: PA2 - Tx, PA3 - Rx */
#define UART2_TX_PORT        GPIOA
#define UART2_TX_PIN         2
#define UART2_RX_PORT        GPIOA
#define UART2_RX_PIN         3

#endif  /* __BOARD_H__ */
