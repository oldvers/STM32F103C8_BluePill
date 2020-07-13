#include "types.h"
#include "interrupts.h"
#include "usb.h"
#include "uart.h"
#include "debug.h"

//-----------------------------------------------------------------------------
/* Private definitions */

//#define ENABLE_IRQ_DEBUG

#ifdef ENABLE_IRQ_DEBUG
#  define IRQ_LOG(...)       LOG(__VA_ARGS__)
#else
#  define IRQ_LOG(...)
#endif

void Fault(U32 stack[])
{
    enum {r0, r1, r2, r3, r12, lr, pc, psr};

    LOG("Hard Fault\r\n");
    LOG("  SHCSR    = 0x%08x\r\n", SCB->SHCSR);
    LOG("  CFSR     = 0x%08x\r\n", SCB->CFSR);
    LOG("  HFSR     = 0x%08x\r\n", SCB->HFSR);
    LOG("  MMFAR    = 0x%08x\r\n", SCB->MMFAR);
    LOG("  BFAR     = 0x%08x\r\n", SCB->BFAR);

    LOG("  R0       = 0x%08x\r\n", stack[r0]);
    LOG("  R1       = 0x%08x\r\n", stack[r1]);
    LOG("  R2       = 0x%08x\r\n", stack[r2]);
    LOG("  R3       = 0x%08x\r\n", stack[r3]);
    LOG("  R12      = 0x%08x\r\n", stack[r12]);
    LOG("  LR [R14] = 0x%08x - Subroutine call return address\r\n", stack[lr]);
    LOG("  PC [R15] = 0x%08x - Program counter\r\n", stack[pc]);
    LOG("  PSR      = 0x%08x\r\n", stack[psr]);

    while(FW_TRUE) {};
}

//                IMPORT  Fault
//HardFault_Handler\
//                PROC
//                TST     LR, #4
//                ITE     EQ
//                MRSEQ   R0, MSP
//                MRSNE   R0, PSP
//                B       Fault
//                ENDP

//-----------------------------------------------------------------------------
/* NVIC */

void IRQ_SetPriorityGrouping(void)
{
  NVIC_SetPriorityGrouping(IRQ_PRIORITY_GROUP_04_SUB_04);
}

//-----------------------------------------------------------------------------
/* System/Core */

void NMI_Handler(void)
{
  //
}

//void HardFault_Handler(void)
//{
//  /* Go to infinite loop when Hard Fault exception occurs */
//  while (1)
//  {
//    //
//  }
//}

void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  //ShowInfo("-Mem Manage-");
  while (1)
  {
    //
  }
}

void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  //ShowInfo("-Bus Fault-");
  while (1)
  {
    //
  }
}

void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  //ShowInfo("-Usage Fault-");
  while (1)
  {
    //
  }
}

//void SVC_Handler(void)
//{
//  //
//}

void DebugMon_Handler(void)
{
  //
}

//void PendSV_Handler(void)
//{
//  //
//}

//void SysTick_Handler(void)
//{
//  //
//}

//-----------------------------------------------------------------------------
/* I2C */

void I2C1_EV_IRQHandler(void)
{
  //I2C_EV_IRQHandler(I2C1);
}

void I2C1_ER_IRQHandler(void)
{
  //I2C_ER_IRQHandler(I2C1);
}

void I2C2_EV_IRQHandler(void)
{
  //I2C_EV_IRQHandler(I2C2);
}

void I2C2_ER_IRQHandler(void)
{
  //I2C_ER_IRQHandler(I2C2);
}

//-----------------------------------------------------------------------------
/* Timer */

void TIM1_CC_IRQHandler(void)
{
  //DHT21_TIM_IRQHandler();
}

//-----------------------------------------------------------------------------
/* USB */

void IRQ_USB_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUP_04_SUB_04,
                   IRQ_PRIORITY_GROUP_BACKGROUND,
                   IRQ_PRIORITY_USB
                 );
  IRQ_LOG("IRQ: USB Priority = 0x%02X\r\n", priority);
  
  NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
  NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, priority);
  NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);  
}

void IRQ_USB_Disable(void)
{
  NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_IRQHandler();
}

//-----------------------------------------------------------------------------
/* USART */

void IRQ_USART1_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUP_04_SUB_04,
                   IRQ_PRIORITY_GROUP_SYSTEM,
                   IRQ_PRIORITY_UART1
                 );
  IRQ_LOG("IRQ: USART1 Priority = 0x%02X\r\n", priority);
  
  NVIC_ClearPendingIRQ(USART1_IRQn);
  NVIC_SetPriority(USART1_IRQn, priority);
  NVIC_EnableIRQ(USART1_IRQn);
}

void IRQ_USART1_Disable(void)
{
  NVIC_DisableIRQ(USART1_IRQn);
  NVIC_ClearPendingIRQ(USART1_IRQn);
}

void USART1_IRQHandler(void)
{
  UART_IRQHandler(UART1);
}

void IRQ_USART2_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUP_04_SUB_04,
                   IRQ_PRIORITY_GROUP_SYSTEM,
                   IRQ_PRIORITY_UART2
                 );
  IRQ_LOG("IRQ: USART2 Priority = 0x%02X\r\n", priority);
  
  NVIC_ClearPendingIRQ(USART2_IRQn);
  NVIC_SetPriority(USART2_IRQn, priority);
  NVIC_EnableIRQ(USART2_IRQn);
}

void IRQ_USART2_Disable(void)
{
  NVIC_DisableIRQ(USART2_IRQn);
  NVIC_ClearPendingIRQ(USART2_IRQn);
}

void USART2_IRQHandler(void)
{
  UART_IRQHandler(UART2);
}

void IRQ_USART3_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUP_04_SUB_04,
                   IRQ_PRIORITY_GROUP_SYSTEM,
                   IRQ_PRIORITY_UART3
                 );
  IRQ_LOG("IRQ: USART3 Priority = 0x%02X\r\n", priority);
  
  NVIC_ClearPendingIRQ(USART3_IRQn);
  NVIC_SetPriority(USART3_IRQn, priority);
  NVIC_EnableIRQ(USART3_IRQn);
}

void IRQ_USART3_Disable(void)
{
  NVIC_DisableIRQ(USART3_IRQn);
  NVIC_ClearPendingIRQ(USART3_IRQn);
}

void USART3_IRQHandler(void)
{
  UART_IRQHandler(UART3);
}
