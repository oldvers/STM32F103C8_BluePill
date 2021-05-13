#ifndef __GPIO_H__
#define __GPIO_H__

#include "types.h"
#include "system.h"

/* PIN MODE ****************************************/
/* 0b00 = 0 - Input mode (reset state).            */
/* 0b01 = 1 - Output mode, max speed 10 MHz.       */
/* 0b10 = 2 - Output mode, max speed 2 MHz.        */
/* 0b11 = 3 - Output mode, max speed 50 MHz.       */

/* PIN CNF *****************************************/
/* In input mode (MODE[1:0] = 00) ---------------- */
/* 0b00 = 0 - Analog mode                          */
/* 0b01 = 1 - Floating input (reset state)         */
/* 0b10 = 2 - Input with Pull-Down / Pull-Up       */
/* 0b11 = 3 - Reserved                             */
/* In output mode (MODE[1:0] > 00) --------------- */
/* 0b00 = 0 - General purpose output Push-Pull     */
/* 0b01 = 1 - General purpose output Open-Drain    */
/* 0b10 = 2 - Alternate function output Push-Pull  */
/* 0b11 = 3 - Alternate function output Open-Drain */

#define GPIO_TYPE_MASK                     ((U32)0x0F)

#define GPIO_IRQ_EDGE_FALLING              ((U32)0x01)
#define GPIO_IRQ_EDGE_RISING               ((U32)0x02)

#define GPIO_TYPE_IN_ANALOG                ((U32)0x00)
#define GPIO_TYPE_IN_FLOATING              ((U32)0x04)
#define GPIO_TYPE_IN_PUP_PDN               ((U32)0x08)

#define GPIO_TYPE_OUT_PP_10MHZ             ((U32)0x01)
#define GPIO_TYPE_OUT_PP_2MHZ              ((U32)0x02)
#define GPIO_TYPE_OUT_PP_50MHZ             ((U32)0x03)
#define GPIO_TYPE_OUT_OD_10MHZ             ((U32)0x05)
#define GPIO_TYPE_OUT_OD_2MHZ              ((U32)0x06)
#define GPIO_TYPE_OUT_OD_50MHZ             ((U32)0x07)
#define GPIO_TYPE_ALT_PP_10MHZ             ((U32)0x09)
#define GPIO_TYPE_ALT_PP_2MHZ              ((U32)0x0A)
#define GPIO_TYPE_ALT_PP_50MHZ             ((U32)0x0B)
#define GPIO_TYPE_ALT_OD_10MHZ             ((U32)0x0D)
#define GPIO_TYPE_ALT_OD_2MHZ              ((U32)0x0E)
#define GPIO_TYPE_ALT_OD_50MHZ             ((U32)0x0F)

typedef struct
{
  volatile U32 CR[2];
  volatile U32 IDR;
  volatile U32 ODR;
  volatile U32 BSRR;
  volatile U32 BRR;
  volatile U32 LCKR;
} GPIO;

#define GPIO_IrqEnable(port,pin,edgemask) \
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; \
  AFIO->EXTICR[pin / 4] &= ~(GPIO_TYPE_MASK << ((pin % 4) * 4)); \
  AFIO->EXTICR[pin / 4] |= \
    ((((U32)port >> 10) & 0x07) - 2) << ((pin % 4) * 4); \
  SYS_BITBAND_HW(EXTI->IMR,pin) = 1; \
  SYS_BITBAND_HW(EXTI->FTSR,pin) = edgemask; \
  SYS_BITBAND_HW(EXTI->RTSR,pin) = (edgemask >> 1);

#define GPIO_Init(port,pin,mode) \
  RCC->APB2ENR |= \
    (U32)((1 << (((U32)port >> 10) & GPIO_TYPE_MASK)) | \
    (RCC_APB2ENR_AFIOEN * (U8)(mode > 8))); \
  ((GPIO *)port)->CR[pin / 8] &= ~(GPIO_TYPE_MASK << ((pin % 8) * 4)); \
  ((GPIO *)port)->CR[pin / 8] |= (mode << ((pin % 8) * 4));

#define GPIO_Hi(port,pin) \
  (port->BSRR = (1 << pin))

#define GPIO_Lo(port,pin) \
  (port->BSRR = (1 << (pin + 16)))

#define GPIO_In(port,pin) \
  ((port->IDR >> pin) & 1)

void GPIO_Init(GPIO_TypeDef * pPort, U8 aPin, U8 aMode, U8 aValue);

#endif /* __GPIO_H__ */
