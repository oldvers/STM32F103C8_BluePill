#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include "hardware.h"

/*      IRQ_PRIORITY_SYSTICK    255 */
/*      IRQ_PRIORITY_PENDSV     255 */
#define IRQ_PRIORITY_USB        255

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

#endif /* __INTERRUPTS_H__ */
