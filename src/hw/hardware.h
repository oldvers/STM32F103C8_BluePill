#ifndef __HARDWARE__
#define __HARDWARE__

/* STM32F103C8 Blue Pill Board */

#include "stm32f1xx.h"

#define NRF_RX

/*****************************************************************************/
/*** GPIO ********************************************************************/
/*****************************************************************************/
/*** GPIOA ***/
/* A0 - nRF24 CE (Chip Enable) */
#ifdef NRF_RX
#define NRF24_CE_PORT                   GPIOA
#define NRF24_CE_PIN                    0
#endif

/* A1 */
/* A2 */
/* A3 */
/* A4 */
/* A5 */
/* A6 */
/* A7 */
/* A8 - nRF24 CE (Chip Enable) */
#ifndef NRF_RX
#define NRF24_CE_PORT                   GPIOA
#define NRF24_CE_PIN                    8
#endif

/* A9 - nRF24 CSN (Chip Select Negative) */
#ifndef NRF_RX
#define NRF24_CSN_PORT                  GPIOA
#define NRF24_CSN_PIN                   9
#endif

/* A10 */
/* A11 - USB- */
/* A12 - USB+ */
/* A13 - SWDIO */
/* A14 - SWCLK */
/* A15 */

/*** GPIOB ***/
/* B0 - nRF24 CSN (Chip Select Negative) */
#ifdef NRF_RX
#define NRF24_CSN_PORT                  GPIOB
#define NRF24_CSN_PIN                   0
#endif

/* B1 */
/* B2 - BOOT1 */
/* B3 - SWOUT */
/* B4 */
/* B5 */
/* B6 */
/* B7 */
/* B8 */
/* B9 */
/* B10 */
/* B11 */
/* B12 - nRF24 IRQ */
#define NRF24_IRQ_PORT                  GPIOB
#define NRF24_IRQ_PIN                   12
/* B13 - nRF24 SCK */
#define NRF24_SCK_PORT                  GPIOB
#define NRF24_SCK_PIN                   13
/* B14 - nRF24 MISO */
#define NRF24_MISO_PORT                 GPIOB
#define NRF24_MISO_PIN                  14
/* B15 - nRF24 MOSI */
#define NRF24_MOSI_PORT                 GPIOB
#define NRF24_MOSI_PIN                  15

/*** GPIOC ***/
/* C13 - LED */
/* C14 - OSC32IN */
/* C15 - OSC32OUT */

/*****************************************************************************/
/*** SPI *********************************************************************/
/*****************************************************************************/
#define NRF24_SPI                       SPI2
#define NRF24_SPI_CLK_R                 APB1ENR
#define NRF24_SPI_CLK_E                 RCC_APB1ENR_SPI2EN

/*****************************************************************************/
/*** DMA *********************************************************************/
/*****************************************************************************/
/*** nRF24 ***/
#define NRF24_SPI_DMA                   DMA1
#define NRF24_SPI_DMA_CLK_R             AHBENR
#define NRF24_SPI_DMA_CLK_E             RCC_AHBENR_DMA1EN

#define NRF24_SPI_DMA_RX_CHANNEL        DMA1_Channel4
#define NRF24_SPI_DMA_RX_FTC            DMA_IFCR_CTCIF4
#define NRF24_SPI_DMA_RX_FTE            DMA_IFCR_CTEIF4
#define NRF24_SPI_DMA_RX_FHT            DMA_IFCR_CHTIF4
#define NRF24_SPI_DMA_RX_FGL            DMA_IFCR_CGIF4

#define NRF24_SPI_DMA_TX_CHANNEL        DMA1_Channel5
#define NRF24_SPI_DMA_TX_FTC            DMA_IFCR_CTCIF5
#define NRF24_SPI_DMA_TX_FTE            DMA_IFCR_CTEIF5
#define NRF24_SPI_DMA_TX_FHT            DMA_IFCR_CHTIF5
#define NRF24_SPI_DMA_TX_FGL            DMA_IFCR_CGIF5

/*****************************************************************************/
/*** IRQ Priorities/Numbers **************************************************/
/*****************************************************************************/
/* Interrupt priorities.
 * This is the raw value as per the Cortex-M3 NVIC.
 * Values can be 255 (lowest) to 0 (highest).
 * See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html.
 * See FreeRTOSConfig.h
 *  - configKERNEL_INTERRUPT_PRIORITY      = 255
 *    (Equivalent to 0xF0, or priority 15)
 *  - configMAX_SYSCALL_INTERRUPT_PRIORITY = 191
 *    (Equivalent to 0xB0, or priority 11)
 */
/* nRF24 IRQ - Uses FreeRTOS functions (11, 0xBF, 175) */
#define IRQ_PRIORITY_NRF24              11
/* USB IRQ - Uses FreeRTOS functions   (15, 0xFF, 255) */
#define IRQ_PRIORITY_USB                15
/*      IRQ_PRIORITY_SYSTICK           (15, 0xFF, 255) */
/*      IRQ_PRIORITY_PENDSV            (15, 0xFF, 255) */


#define IRQ_NUMBER_USB                  USB_LP_CAN1_RX0_IRQn
#define IRQ_NUMBER_NRF24                EXTI15_10_IRQn

#endif /* __HARDWARE__ */
