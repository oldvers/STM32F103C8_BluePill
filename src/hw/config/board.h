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

/* SPI1/UART2: PA2 - RST/RX/TX, PA5 - SCK, PA6 - MISO, PA7 - MOSI */
#define SPI1_RST_PORT        GPIOA
#define SPI1_RST_PIN         2
#define SPI1_SCK_PORT        GPIOA
#define SPI1_SCK_PIN         5
#define SPI1_MISO_PORT       GPIOA
#define SPI1_MISO_PIN        6
#define SPI1_MOSI_PORT       GPIOA
#define SPI1_MOSI_PIN        7
#define UART2_RTX_PORT       GPIOA
#define UART2_RTX_PIN        2

#endif  /* __BOARD_H__ */
