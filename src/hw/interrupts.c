#include <stdio.h>
#include "types.h"
#include "interrupts.h"
#include "usb.h"
#include "uart.h"
#include "debug.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"

/* --- Logging -------------------------------------------------------------- */

#define ENABLE_IRQ_DEBUG

#ifdef ENABLE_IRQ_DEBUG
#  define IRQ_LOG(...)       printf(__VA_ARGS__)
#else
#  define IRQ_LOG(...)
#endif

/* -------------------------------------------------------------------------- */
/*  The table below gives the allowed values of the pre-emption priority and
  subpriority according to the Priority Grouping configuration performed by
  NVIC_PriorityGroupConfig function
 ==============================================================================
    PriorityGroup       | Priority | SubPriority  | Description
 ==============================================================================
  NVIC_PriorityGroup_0  |   0      |  0-15        |   0 bits for priority
                        |          |              |   4 bits for subpriority
 ------------------------------------------------------------------------------
  NVIC_PriorityGroup_1  |   0-1    |  0-7         |   1 bits for priority
                        |          |              |   3 bits for subpriority
 ------------------------------------------------------------------------------
  NVIC_PriorityGroup_2  |   0-3    |  0-3         |   2 bits for priority
                        |          |              |   2 bits for subpriority
 ------------------------------------------------------------------------------
  NVIC_PriorityGroup_3  |   0-7    |  0-1         |   3 bits for priority
                        |          |              |   1 bits for subpriority
 ------------------------------------------------------------------------------
  NVIC_PriorityGroup_4  |   0-15   |  0           |   4 bits for priority
                        |          |              |   0 bits for subpriority
 ============================================================================ */

/* Interrupts priority grouping */
#define IRQ_PRIORITY_GROUP_16_SUB_01    (3)
#define IRQ_PRIORITY_GROUP_08_SUB_02    (4)
#define IRQ_PRIORITY_GROUP_04_SUB_04    (5)
#define IRQ_PRIORITY_GROUP_02_SUB_08    (6)
#define IRQ_PRIORITY_GROUP_01_SUB_16    (7)

/* --- Interrupt groups and priorities -------------------------------------- */
/* FreeRTOS doesn't allow subpriorities */
#define IRQ_PRIORITY_GROUPS_CONFIG      (IRQ_PRIORITY_GROUP_16_SUB_01)

/* The preemption priority */
#define IRQ_PRIORITY_FAULT              ( 0)
#define IRQ_PRIORITY_I2C1               ( 1)
#define IRQ_PRIORITY_I2C2               ( 1)
#define IRQ_PRIORITY_I2C3               ( 1)
#define IRQ_PRIORITY_TIM2               ( 1)
#define IRQ_PRIORITY_UART1              ( 3)
#define IRQ_PRIORITY_UART2              ( 3)
#define IRQ_PRIORITY_UART3              ( 3)
#define IRQ_PRIORITY_SPI1               ( 4)
#define IRQ_PRIORITY_SPI2               ( 4)
/*      IRQ_PRIORITY_MAX_SYSCALL        (11) */
#define IRQ_PRIORITY_USB                (15)
/*      IRQ_PRIORITY_SYSTICK            (15) */
/*      IRQ_PRIORITY_PENDSV             (15) */

/* Subpriority */
#define IRQ_SUB_PRIORITY                (0)

/* --- Definitions ---------------------------------------------------------- */

#define FAULT_ID_OFFSET                 (16)
#define FAULT_ID_MASK                   (0xFF)

typedef enum
{
  R0, R1, R2, R3, R12, LR, PC, PSR
} StackFrameOffsets_e;

extern void on_error(S32 parameter);

/* --- Functions ------------------------------------------------------------ */

void IRQ_SetPriorityGrouping(void)
{
  U32 grouping = 0, priority = 0;

  NVIC_SetPriorityGrouping(IRQ_PRIORITY_GROUPS_CONFIG);

  /* Enable Usage-/Bus-/MPU faults */
  SCB->SHCSR |= ( SCB_SHCSR_USGFAULTENA_Msk |
                  SCB_SHCSR_BUSFAULTENA_Msk |
                  SCB_SHCSR_MEMFAULTENA_Msk );

  /* Enable usage faults: */
  SCB->CCR |=
  (
    /* On divizion by zero */
    SCB_CCR_DIV_0_TRP_Msk |
    /* On unaligned memory access */
    0 //SCB_CCR_UNALIGN_TRP_Msk
  );

  /* Set the interrupt priorities */
  grouping = NVIC_GetPriorityGrouping();
  priority = NVIC_EncodePriority
             (
               grouping,
               IRQ_PRIORITY_FAULT,
               IRQ_SUB_PRIORITY
             );

  NVIC_SetPriority(MemoryManagement_IRQn, priority);
  NVIC_SetPriority(BusFault_IRQn, priority);
  NVIC_SetPriority(UsageFault_IRQn, priority);
}

/* -------------------------------------------------------------------------- */

void IRQ_Exception_Handler(U32 pStackFrame[], U32 LRValue)
{
  S32 FaultID = ((SCB->ICSR & FAULT_ID_MASK) - FAULT_ID_OFFSET);

  IRQ_LOG("- Fault ID = ");
  switch (FaultID)
  {
    case NonMaskableInt_IRQn:
      IRQ_LOG("NMI\r\n");
      break;

    case HardFault_IRQn:
      IRQ_LOG("Hard\r\n");
      break;

    case MemoryManagement_IRQn:
      IRQ_LOG("Memory Management\r\n");
      break;

    case BusFault_IRQn:
      IRQ_LOG("Bus\r\n");
      break;

    case UsageFault_IRQn:
      IRQ_LOG("Usage\r\n");
      break;

    case DebugMonitor_IRQn:
      IRQ_LOG("Debug Monitor\r\n");
      break;

    default:
      IRQ_LOG("Unknown!\r\n");
      break;
  }
  IRQ_LOG("  LR       = 0x%08X\r\n", LRValue);
  IRQ_LOG("  SHCSR    = 0x%08X\r\n", SCB->SHCSR);
  IRQ_LOG("  CFSR     = 0x%08X\r\n", SCB->CFSR);
  IRQ_LOG("  HFSR     = 0x%08X\r\n", SCB->HFSR);
  IRQ_LOG("  MMFAR    = 0x%08X\r\n", SCB->MMFAR);
  IRQ_LOG("  BFAR     = 0x%08X\r\n", SCB->BFAR);
  IRQ_LOG("- Stack Frame\r\n");
  IRQ_LOG("  R0       = 0x%08X\r\n", pStackFrame[R0]);
  IRQ_LOG("  R1       = 0x%08X\r\n", pStackFrame[R1]);
  IRQ_LOG("  R2       = 0x%08X\r\n", pStackFrame[R2]);
  IRQ_LOG("  R3       = 0x%08X\r\n", pStackFrame[R3]);
  IRQ_LOG("  R12      = 0x%08X\r\n", pStackFrame[R12]);
  IRQ_LOG("  LR [R14] = 0x%08X - Return address\r\n", pStackFrame[LR]);
  IRQ_LOG("  PC [R15] = 0x%08X - Program counter\r\n", pStackFrame[PC]);
  IRQ_LOG("  PSR      = 0x%08X\r\n", pStackFrame[PSR]);

  on_error(FaultID);
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN IRQ_IsInExceptionMode(void)
{
  U32 ipsr = __get_IPSR();
  return (FW_BOOLEAN)(0 != ipsr);
}

/* --- USB ------------------------------------------------------------------ */

void IRQ_USB_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUPS_CONFIG,
                   IRQ_PRIORITY_USB,
                   IRQ_SUB_PRIORITY
                 );
  IRQ_LOG("IRQ: USB Priority = 0x%02X\r\n", priority);

  NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
  NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, priority);
  NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}

/* -------------------------------------------------------------------------- */

void IRQ_USB_Disable(void)
{
  NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
}

/* -------------------------------------------------------------------------- */

void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_IRQHandler();
}

/* --- USART ---------------------------------------------------------------- */

void IRQ_USART1_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUPS_CONFIG,
                   IRQ_PRIORITY_UART1,
                   IRQ_SUB_PRIORITY
                 );
  IRQ_LOG("IRQ: USART1 Priority = 0x%02X\r\n", priority);

  NVIC_ClearPendingIRQ(USART1_IRQn);
  NVIC_SetPriority(USART1_IRQn, priority);
  NVIC_EnableIRQ(USART1_IRQn);
}

/* -------------------------------------------------------------------------- */

void IRQ_USART1_Disable(void)
{
  NVIC_DisableIRQ(USART1_IRQn);
  NVIC_ClearPendingIRQ(USART1_IRQn);
}

/* -------------------------------------------------------------------------- */

void USART1_IRQHandler(void)
{
  UART_IrqHandler(UART1);
}

/* -------------------------------------------------------------------------- */

void IRQ_USART2_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUPS_CONFIG,
                   IRQ_PRIORITY_UART2,
                   IRQ_SUB_PRIORITY
                 );
  IRQ_LOG("IRQ: USART2 Priority = 0x%02X\r\n", priority);

  NVIC_ClearPendingIRQ(USART2_IRQn);
  NVIC_SetPriority(USART2_IRQn, priority);
  NVIC_EnableIRQ(USART2_IRQn);
}

/* -------------------------------------------------------------------------- */

void IRQ_USART2_Disable(void)
{
  NVIC_DisableIRQ(USART2_IRQn);
  NVIC_ClearPendingIRQ(USART2_IRQn);
}

/* -------------------------------------------------------------------------- */

void USART2_IRQHandler(void)
{
  UART_IrqHandler(UART2);
}

/* -------------------------------------------------------------------------- */

void IRQ_USART3_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUPS_CONFIG,
                   IRQ_PRIORITY_UART3,
                   IRQ_SUB_PRIORITY
                 );
  IRQ_LOG("IRQ: USART3 Priority = 0x%02X\r\n", priority);

  NVIC_ClearPendingIRQ(USART3_IRQn);
  NVIC_SetPriority(USART3_IRQn, priority);
  NVIC_EnableIRQ(USART3_IRQn);
}

/* -------------------------------------------------------------------------- */

void IRQ_USART3_Disable(void)
{
  NVIC_DisableIRQ(USART3_IRQn);
  NVIC_ClearPendingIRQ(USART3_IRQn);
}

/* -------------------------------------------------------------------------- */

void USART3_IRQHandler(void)
{
  UART_IrqHandler(UART3);
}

/* --- I2C1 ----------------------------------------------------------------- */

void IRQ_I2C1_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUPS_CONFIG,
                   IRQ_PRIORITY_I2C1,
                   IRQ_SUB_PRIORITY
                 );
  IRQ_LOG("IRQ: I2C1 Priority = 0x%02X\r\n", priority);

  NVIC_ClearPendingIRQ(I2C1_EV_IRQn);
  NVIC_ClearPendingIRQ(I2C1_ER_IRQn);
  NVIC_SetPriority(I2C1_EV_IRQn, priority);
  NVIC_SetPriority(I2C1_ER_IRQn, priority);
  NVIC_EnableIRQ(I2C1_EV_IRQn);
  NVIC_EnableIRQ(I2C1_ER_IRQn);
}

/* -------------------------------------------------------------------------- */

void IRQ_I2C1_Disable(void)
{
  NVIC_DisableIRQ(I2C1_EV_IRQn);
  NVIC_DisableIRQ(I2C1_ER_IRQn);
  NVIC_ClearPendingIRQ(I2C1_EV_IRQn);
  NVIC_ClearPendingIRQ(I2C1_ER_IRQn);
}

/* -------------------------------------------------------------------------- */

void I2C1_EV_IRQHandler(void)
{
  I2C_IrqHandler(I2C_1);
}
/* -------------------------------------------------------------------------- */

void I2C1_ER_IRQHandler(void)
{
  I2C_IrqError(I2C_1);
}

/* --- I2C2 ----------------------------------------------------------------- */

void IRQ_I2C2_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUPS_CONFIG,
                   IRQ_PRIORITY_I2C2,
                   IRQ_SUB_PRIORITY
                 );
  IRQ_LOG("IRQ: I2C2 Priority = 0x%02X\r\n", priority);

  NVIC_ClearPendingIRQ(I2C2_EV_IRQn);
  NVIC_ClearPendingIRQ(I2C2_ER_IRQn);
  NVIC_SetPriority(I2C2_EV_IRQn, priority);
  NVIC_SetPriority(I2C2_ER_IRQn, priority);
  NVIC_EnableIRQ(I2C2_EV_IRQn);
  NVIC_EnableIRQ(I2C2_ER_IRQn);
}

/* -------------------------------------------------------------------------- */

void IRQ_I2C2_Disable(void)
{
  NVIC_DisableIRQ(I2C2_EV_IRQn);
  NVIC_DisableIRQ(I2C2_ER_IRQn);
  NVIC_ClearPendingIRQ(I2C2_EV_IRQn);
  NVIC_ClearPendingIRQ(I2C2_ER_IRQn);
}

/* -------------------------------------------------------------------------- */

void I2C2_EV_IRQHandler(void)
{
  I2C_IrqHandler(I2C_2);
}
/* -------------------------------------------------------------------------- */

void I2C2_ER_IRQHandler(void)
{
  I2C_IrqError(I2C_2);
}

/* --- SPI1 ----------------------------------------------------------------- */

void IRQ_SPI1_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUPS_CONFIG,
                   IRQ_PRIORITY_SPI1,
                   IRQ_SUB_PRIORITY
                 );
  IRQ_LOG("IRQ: SPI1 Priority = 0x%02X\r\n", priority);

  NVIC_ClearPendingIRQ(DMA1_Channel2_IRQn);
  NVIC_SetPriority(DMA1_Channel2_IRQn, priority);
  NVIC_EnableIRQ(DMA1_Channel2_IRQn);
}

/* -------------------------------------------------------------------------- */

void IRQ_SPI1_Disable(void)
{
  NVIC_DisableIRQ(DMA1_Channel2_IRQn);
  NVIC_ClearPendingIRQ(DMA1_Channel2_IRQn);
}

/* -------------------------------------------------------------------------- */

void DMA1_Channel2_IRQHandler(void)
{
  SPI_IrqHandler(SPI_1);
}

/* --- SPI2 ----------------------------------------------------------------- */

void IRQ_SPI2_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUPS_CONFIG,
                   IRQ_PRIORITY_SPI2,
                   IRQ_SUB_PRIORITY
                 );
  IRQ_LOG("IRQ: SPI2 Priority = 0x%02X\r\n", priority);

  NVIC_ClearPendingIRQ(DMA1_Channel4_IRQn);
  NVIC_SetPriority(DMA1_Channel4_IRQn, priority);
  NVIC_EnableIRQ(DMA1_Channel4_IRQn);
}

/* -------------------------------------------------------------------------- */

void IRQ_SPI2_Disable(void)
{
  NVIC_DisableIRQ(DMA1_Channel4_IRQn);
  NVIC_ClearPendingIRQ(DMA1_Channel4_IRQn);
}

/* -------------------------------------------------------------------------- */

void DMA1_Channel4_IRQHandler(void)
{
  SPI_IrqHandler(SPI_2);
}

/* --- TIM2 ----------------------------------------------------------------- */

void IRQ_TIM2_Enable(void)
{
  U32 priority = NVIC_EncodePriority
                 (
                   IRQ_PRIORITY_GROUPS_CONFIG,
                   IRQ_PRIORITY_TIM2,
                   IRQ_SUB_PRIORITY
                 );
  IRQ_LOG("IRQ: TIM2 Priority = 0x%02X\r\n", priority);

  NVIC_ClearPendingIRQ(TIM2_IRQn);
  NVIC_SetPriority(TIM2_IRQn, priority);
  NVIC_EnableIRQ(TIM2_IRQn);
}

/* -------------------------------------------------------------------------- */

void IRQ_TIM2_Disable(void)
{
  NVIC_DisableIRQ(TIM2_IRQn);
  NVIC_ClearPendingIRQ(TIM2_IRQn);
}

/* -------------------------------------------------------------------------- */

void TIM2_IRQHandler(void)
{
  TIM2_IrqHandler();
}

/* -------------------------------------------------------------------------- */
