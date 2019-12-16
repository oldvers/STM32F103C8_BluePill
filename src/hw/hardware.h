#ifndef __HARDWARE__
#define __HARDWARE__

/* STM32F103C8 Blue Pill Board */

#include "stm32f1xx.h"

//#define NRF_RX

/*****************************************************************************/
/*** GPIO ********************************************************************/
/*****************************************************************************/
/*** GPIOA ***/
/* A0 */
/* A1 */
/* A2 */
/* A3 */
/* A4 */
/* A5 */
/* A6 */
/* A7 */
/* A8 */
/* A9 */
/* A10 */
/* A11 - USB- */
/* A12 - USB+ */
/* A13 - SWDIO */
/* A14 - SWCLK */
/* A15 - */

/*** GPIOB ***/
/* B0 */
/* B1 */
/* B2 - BOOT1 */
/* B3 - SWOUT */
/* B4 */
/* B5 */
/* B6*/
/* B7 */
/* B8 */
/* B9 */
#define WS2812_PORT                     GPIOB
#define WS2812_PIN                      9

/* B10 */
/* B11 */
/* B12 */
/* B13 */
/* B14 */
/* B15 */

/*** GPIOC ***/
/* C13 - */
/* C14 - OSC32IN */
/* C15 - OSC32OUT */

/*****************************************************************************/
/*** SPI *********************************************************************/
/*****************************************************************************/
//#define NRF24_SPI                       SPI2
//#define NRF24_SPI_CLK_R                 APB1ENR
//#define NRF24_SPI_CLK_E                 RCC_APB1ENR_SPI2EN

/*****************************************************************************/
/*** DMA *********************************************************************/
/*****************************************************************************/
/*** WS2812 ***/
#define WS2812_TIM_DMA                  DMA1
#define WS2812_TIM_DMA_CLK_R            AHBENR
#define WS2812_TIM_DMA_CLK_E            RCC_AHBENR_DMA1EN

#define WS2812_TIM_DMA_CHANNEL          DMA1_Channel7
#define WS2812_TIM_DMA_FTC              DMA_IFCR_CTCIF7
#define WS2812_TIM_DMA_FTE              DMA_IFCR_CTEIF7
#define WS2812_TIM_DMA_FHT              DMA_IFCR_CHTIF7
#define WS2812_TIM_DMA_FGL              DMA_IFCR_CGIF7

/*****************************************************************************/
/*** TIM *********************************************************************/
/*****************************************************************************/
/*** WS2812 ***/
#define WS2812_TIM                      TIM4
#define WS2812_TIM_CCR                  CCR4
#define WS2812_TIM_DBGMCU               DBGMCU_CR_DBG_TIM4_STOP
#define WS2812_TIM_CLK_R                APB1ENR
#define WS2812_TIM_CLK_E                RCC_APB1ENR_TIM4EN

/*****************************************************************************/
/*** I2C1 ********************************************************************/
/*****************************************************************************/
//#define SENS_I2C                        I2C1

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
/* WS2812 IRQ - No FreeRTOS functions    ( 0, 0x0F,  15) */
#define IRQ_NUMBER_WS2812_TIM_DMA        DMA1_Channel7_IRQn
#define IRQ_PRIORITY_WS2812_TIM_DMA      0
//#define IRQ_PRIORITY_SENS_I2C_ERROR      1
///* nRF24 IRQ - Uses FreeRTOS functions (11, 0xBF, 175) */
//#define IRQ_PRIORITY_NRF24              11
///* USB IRQ - Uses FreeRTOS functions   (15, 0xFF, 255) */
//#define IRQ_PRIORITY_USB                15
/*      IRQ_PRIORITY_SYSTICK           (15, 0xFF, 255) */
/*      IRQ_PRIORITY_PENDSV            (15, 0xFF, 255) */

//#define IRQ_NUMBER_USB                  USB_LP_CAN1_RX0_IRQn
//#define IRQ_NUMBER_NRF24                EXTI15_10_IRQn
//#define IRQ_NUMBER_SENS_I2C_EVT         I2C1_EV_IRQn 
//#define IRQ_NUMBER_SENS_I2C_ERR         I2C1_ER_IRQn 

#endif /* __HARDWARE__ */
