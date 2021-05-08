#include <stdio.h>
#include "types.h"
#include "interrupts.h"

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
/*      IRQ_PRIORITY_MAX_SYSCALL        (11) */
/*      IRQ_PRIORITY_SYSTICK            (15) */
/*      IRQ_PRIORITY_PENDSV             (15) */

/* Subpriority */
#define IRQ_SUB_PRIORITY                (0)

/* --- Definitions ---------------------------------------------------------- */

typedef enum
{
  R0, R1, R2, R3, R12, LR, PC, PSR
} StackFrameOffsets_e;

/* --- Functions ------------------------------------------------------------ */

void IRQ_SetPriorityGrouping(void)
{
  NVIC_SetPriorityGrouping(IRQ_PRIORITY_GROUPS_CONFIG);
}

/* -------------------------------------------------------------------------- */

void IRQ_Exception_Handler(U32 pStackFrame[], U32 LRValue)
{
  IRQ_LOG("Hard Fault\r\n");
  IRQ_LOG("  SHCSR    = 0x%08x\r\n", SCB->SHCSR);
  IRQ_LOG("  CFSR     = 0x%08x\r\n", SCB->CFSR);
  IRQ_LOG("  HFSR     = 0x%08x\r\n", SCB->HFSR);
  IRQ_LOG("  MMFAR    = 0x%08x\r\n", SCB->MMFAR);
  IRQ_LOG("  BFAR     = 0x%08x\r\n", SCB->BFAR);

  IRQ_LOG("  R0       = 0x%08x\r\n", pStackFrame[R0]);
  IRQ_LOG("  R1       = 0x%08x\r\n", pStackFrame[R1]);
  IRQ_LOG("  R2       = 0x%08x\r\n", pStackFrame[R2]);
  IRQ_LOG("  R3       = 0x%08x\r\n", pStackFrame[R3]);
  IRQ_LOG("  R12      = 0x%08x\r\n", pStackFrame[R12]);
  IRQ_LOG("  LR [R14] = 0x%08x - Return address\r\n", pStackFrame[LR]);
  IRQ_LOG("  PC [R15] = 0x%08x - Program counter\r\n", pStackFrame[PC]);
  IRQ_LOG("  PSR      = 0x%08x\r\n", pStackFrame[PSR]);

  while(TRUE) {};
}

/* -------------------------------------------------------------------------- */
