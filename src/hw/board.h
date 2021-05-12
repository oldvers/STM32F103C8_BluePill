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

#endif  /* __BOARD_H__ */
