#ifndef __BOARD_H__
#define __BOARD_H__

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

/* WiFi: PB7 - En */
#define WIFI_EN_PORT         GPIOB
#define WIFI_EN_PIN          7

/* I2C1: PB6 - SCL, PB7 - SDA */
#define I2C1_SCL_PORT        GPIOB
#define I2C1_SCL_PIN         6
#define I2C1_SDA_PORT        GPIOB
#define I2C1_SDA_PIN         7

/* SPI1: PA4 - CS, PA5 - SCK, PA6 - MISO, PA7 - MOSI */
#define SPI1_CS_PORT         GPIOA
#define SPI1_CS_PIN          4
#define SPI1_SCK_PORT        GPIOA
#define SPI1_SCK_PIN         5
#define SPI1_MISO_PORT       GPIOA
#define SPI1_MISO_PIN        6
#define SPI1_MOSI_PORT       GPIOA
#define SPI1_MOSI_PIN        7

/* SPI2: PB12 - CS, PB13 - SCK, PB14 - MISO, PB15 - MOSI */
#define SPI2_CS_PORT         GPIOB
#define SPI2_CS_PIN          12
#define SPI2_SCK_PORT        GPIOB
#define SPI2_SCK_PIN         13
#define SPI2_MISO_PORT       GPIOB
#define SPI2_MISO_PIN        14
#define SPI2_MOSI_PORT       GPIOB
#define SPI2_MOSI_PIN        15

#endif  /* __BOARD_H__ */