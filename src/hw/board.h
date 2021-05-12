#ifndef __BOARD_H__
#define __BOARD_H__

//#define NRF_RX

/* SWD - SWO */
#define SWD_SWO_PORT         GPIOB
#define SWD_SWO_PIN          3

/* USB - Pull Up */
#define USB_PUP_PORT         GPIOB
#define USB_PUP_PIN          2

/* USB - DP/DM */
#define USB_DM_PORT          GPIOA
#define USB_DM_PIN           11
#define USB_DP_PORT          GPIOA
#define USB_DP_PIN           12

/* LED */
#define LED_PORT             GPIOC
#define LED_PIN              13

/* nRF24 CE (Chip Enable) */
#ifdef NRF_RX
#define NRF24_CE_PORT        GPIOA
#define NRF24_CE_PIN         0
#else
#define NRF24_CE_PORT        GPIOA
#define NRF24_CE_PIN         8
#endif

/* nRF24 CSN (Chip Select Negative) */
#ifndef NRF_RX
#define NRF24_CSN_PORT       GPIOA
#define NRF24_CSN_PIN        9
#else
#define NRF24_CSN_PORT       GPIOB
#define NRF24_CSN_PIN        0
#endif

/* Sensors I2C1 SCL */
#define SENS_I2C_SCL_PORT    GPIOB
#define SENS_I2C_SCL_PIN     6

/* Sensors I2C1 SDA */
#define SENS_I2C_SDA_PORT    GPIOB
#define SENS_I2C_SDA_PIN     7

/* nRF24 IRQ */
#define NRF24_IRQ_PORT       GPIOB
#define NRF24_IRQ_PIN        12
/* nRF24 SCK */
#define NRF24_SCK_PORT       GPIOB
#define NRF24_SCK_PIN        13
/* nRF24 MISO */
#define NRF24_MISO_PORT      GPIOB
#define NRF24_MISO_PIN       14
/* nRF24 MOSI */
#define NRF24_MOSI_PORT      GPIOB
#define NRF24_MOSI_PIN       15

/******************************************************************************/
/*** SPI **********************************************************************/
/******************************************************************************/
#define NRF24_SPI                       SPI2
#define NRF24_SPI_CLK_R                 APB1ENR
#define NRF24_SPI_CLK_E                 RCC_APB1ENR_SPI2EN

/******************************************************************************/
/*** DMA **********************************************************************/
/******************************************************************************/
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

/******************************************************************************/
/*** I2C1 *********************************************************************/
/******************************************************************************/
#define SENS_I2C                        I2C1


#endif  /* __BOARD_H__ */
