#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include "stm32f1xx.h"

/* --- Critical Area -------------------------------------------------------- */

//-----------------------------------------------------------------------------
/* Interrupt groups and priorities */
/* The highest priority group */
#define IRQ_PRIORITY_GROUP_CRITICAL     (0)
#define IRQ_SAFE_AREA()    unsigned int IrqState;
#define IRQ_DISABLE()      {                               \
                             IrqState = __get_PRIMASK();   \
                             __set_PRIMASK(1);
#define IRQ_RESTORE()        __set_PRIMASK(IrqState);      \
                           }

/* The group for the most of hardware*/
#define IRQ_PRIORITY_GROUP_SYSTEM       (1)
#define IRQ_PRIORITY_UART1              (3)  /*  7 */
#define IRQ_PRIORITY_UART2              (3)  /*  7 */
#define IRQ_PRIORITY_UART3              (3)  /*  7 */

/* Kernel group */
#define IRQ_PRIORITY_GROUP_KERNEL       (2)
/*      IRQ_PRIORITY_SYSCALL            (3)     11 */

/* Background group */
#define IRQ_PRIORITY_GROUP_BACKGROUND   (3) 
/*      IRQ_PRIORITY_SYSTICK            (3)     15 */
/*      IRQ_PRIORITY_PENDSV             (3)     15 */
#define IRQ_PRIORITY_USB                (3)  /* 15 */

//-----------------------------------------------------------------------------
/* Functions */
void IRQ_SetPriorityGrouping(void);
void IRQ_USB_Enable(void);
void IRQ_USB_Disable(void);
void IRQ_USART1_Enable(void);
void IRQ_USART1_Disable(void);
void IRQ_USART2_Enable(void);
void IRQ_USART2_Disable(void);
void IRQ_USART3_Enable(void);
void IRQ_USART3_Disable(void);

/* --- Public Functions ----------------------------------------------------- */

void IRQ_SetPriorityGrouping(void);
void IRQ_USB_Enable(void);
void IRQ_USB_Disable(void);

#endif /* __INTERRUPTS_H__ */
