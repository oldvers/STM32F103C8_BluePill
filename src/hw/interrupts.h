#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include "stm32f1xx.h"

/* --- Critical Area -------------------------------------------------------- */

#define IRQ_SAFE_AREA()    unsigned int IrqState;
#define IRQ_DISABLE()      {                               \
                             IrqState = __get_PRIMASK();   \
                             __set_PRIMASK(1);
#define IRQ_RESTORE()        __set_PRIMASK(IrqState);      \
                           }

/* --- Public Functions ----------------------------------------------------- */
void       IRQ_SetPriorityGrouping (void);
FW_BOOLEAN IRQ_IsInExceptionMode   (void);
void       IRQ_USB_Enable          (void);
void       IRQ_USB_Disable         (void);
void       IRQ_USART1_Enable       (void);
void       IRQ_USART1_Disable      (void);
void       IRQ_USART2_Enable       (void);
void       IRQ_USART2_Disable      (void);
void       IRQ_USART3_Enable       (void);
void       IRQ_USART3_Disable      (void);
void       IRQ_I2C1_Enable         (void);
void       IRQ_I2C1_Disable        (void);
void       IRQ_I2C2_Enable         (void);
void       IRQ_I2C2_Disable        (void);
void       IRQ_SPI1_Enable         (void);
void       IRQ_SPI1_Disable        (void);
void       IRQ_SPI2_Enable         (void);
void       IRQ_SPI2_Disable        (void);

#endif /* __INTERRUPTS_H__ */
