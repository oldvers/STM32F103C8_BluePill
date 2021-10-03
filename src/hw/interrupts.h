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

#endif /* __INTERRUPTS_H__ */
