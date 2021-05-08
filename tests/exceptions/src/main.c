#include <stdio.h>

#include "stm32f1xx.h"
#include "types.h"
#include "gpio.h"
#include "uniquedevid.h"
#include "system.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//void vLEDTask(void * pvParameters)
//{
//  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ, 0);
//
//  while(1)
//  {
//    GPIO_Lo(GPIOC, 13);
//    vTaskDelay(500);
//    GPIO_Hi(GPIOC, 13);
//    vTaskDelay(500);
//  }
//  //vTaskDelete(NULL);
//}


/* --- Interrupts priority grouping ----------------------------------------- */

#define IRQ_PRIORITY_GROUP_16_SUB_01    (3)

/* --- Definitions ---------------------------------------------------------- */

#define MAX_TEST_NUMBER                 (5)
#define TEST_MARK_IN_PROGRESS           (0xFACECAFE)
#define TEST_MARK_FINISHED              (0xCAFE0BAD)

#define FAULT_ID_OFFSET                 (16)
#define FAULT_ID_MASK                   (0xFF)
// #define FAULT_ID_NMI                    (0)
// #define FAULT_ID_HARDFAULT              (1)
// #define FAULT_ID_MEMMANAGE              (2)
// #define FAULT_ID_BUSFAULT               (3)
// #define FAULT_ID_USAGEFAULT             (4)
// #define FAULT_ID_DEBUGMON               (5)

typedef enum
{
  R0, R1, R2, R3, R12, LR, PC, PSR
} StackFrameOffsets_e;

__no_init static U32 gTestMark;
__no_init static U32 gTestNumber;

/* --- Functions ------------------------------------------------------------ */

void IRQ_SetPriorityGrouping(void)
{
  U32 grouping = 0, priority = 0;

  NVIC_SetPriorityGrouping(IRQ_PRIORITY_GROUP_16_SUB_01);

  /* Enable Usage-/Bus-/MPU faults */
  SCB->SHCSR |= ( SCB_SHCSR_USGFAULTENA_Msk |
                  SCB_SHCSR_BUSFAULTENA_Msk |
                  SCB_SHCSR_MEMFAULTENA_Msk );

  /* Enable divizion by zero and usage faults */
  SCB->CCR |= ( SCB_CCR_DIV_0_TRP_Msk | SCB_CCR_UNALIGN_TRP_Msk );

  /* Set the interrupt priorities */
  grouping = NVIC_GetPriorityGrouping();
  priority = NVIC_EncodePriority
             (
               grouping,
               0,
               0
             );
  printf("IRQ: Faults Priority = 0x%02X\r\n", priority);

  NVIC_SetPriority(MemoryManagement_IRQn, priority);
  NVIC_SetPriority(BusFault_IRQn, priority);
  NVIC_SetPriority(UsageFault_IRQn, priority);
}

/* -------------------------------------------------------------------------- */

static void Delay(void)
{
  U32 delay = 2000000;

  while (delay) { delay--; }
}

/* -------------------------------------------------------------------------- */

void IRQ_Exception_Handler(U32 pStackFrame[], U32 LRValue)
{
  S32 FaultID = ((SCB->ICSR & FAULT_ID_MASK) - FAULT_ID_OFFSET);

  printf("- Fault ID = ");
  switch (FaultID)
  {
    case NonMaskableInt_IRQn:
      printf("NMI\r\n");
      break;

    case HardFault_IRQn:
      printf("Hard\r\n");
      break;

    case MemoryManagement_IRQn:
      printf("Memory Management\r\n");
      break;

    case BusFault_IRQn:
      printf("Bus\r\n");
      break;

    case UsageFault_IRQn:
      printf("Usage\r\n");
      break;

    case DebugMonitor_IRQn:
      printf("Debug Monitor\r\n");
      break;

    default:
      printf("Unknown!\r\n");
      break;
  }
  printf("  LR       = 0x%08X\r\n", LRValue);
  printf("  SHCSR    = 0x%08X\r\n", SCB->SHCSR);
  printf("  CFSR     = 0x%08X\r\n", SCB->CFSR);
  printf("  HFSR     = 0x%08X\r\n", SCB->HFSR);
  printf("  MMFAR    = 0x%08X\r\n", SCB->MMFAR);
  printf("  BFAR     = 0x%08X\r\n", SCB->BFAR);
  printf("- Stack Frame\r\n");
  printf("  R0       = 0x%08X\r\n", pStackFrame[R0]);
  printf("  R1       = 0x%08X\r\n", pStackFrame[R1]);
  printf("  R2       = 0x%08X\r\n", pStackFrame[R2]);
  printf("  R3       = 0x%08X\r\n", pStackFrame[R3]);
  printf("  R12      = 0x%08X\r\n", pStackFrame[R12]);
  printf("  LR [R14] = 0x%08X - Return address\r\n", pStackFrame[LR]);
  printf("  PC [R15] = 0x%08X - Program counter\r\n", pStackFrame[PC]);
  printf("  PSR      = 0x%08X\r\n", pStackFrame[PSR]);

  //while(TRUE) {};

  if (gTestNumber == FaultID)
  {
    printf("- PASS!\r\n");
  }
  else
  {
    printf("- FAIL!\r\n");
  }

  Delay();

  gTestNumber++;
  NVIC_SystemReset();
}

/* -------------------------------------------------------------------------- */

int main(void)
{
  U32 testValueA = 0, testValueB = 0;

  /* Check for the start of the test */
  if (TEST_MARK_IN_PROGRESS != gTestMark)
  {
    gTestMark = TEST_MARK_IN_PROGRESS;
    gTestNumber = 0;
  }

  if (MAX_TEST_NUMBER <= gTestNumber)
  {
    printf("--- All tests finished! ---------------------\r\n");
    gTestMark = TEST_MARK_FINISHED;
    while(TRUE) {};
  }
  else
  {
    printf("--- Test Number = %d --------------------------\r\n", gTestNumber);

    printf("STM32F103C8 Started!\r\n");
    printf("ID0         = 0x%04X\r\n", UDID_0);
    printf("ID1         = 0x%04X\r\n", UDID_1);
    printf("ID2         = 0x%08X\r\n", UDID_2);
    printf("ID2         = 0x%08X\r\n", UDID_3);
    printf("Memory Size = %d kB\r\n", FLASH_SIZE);
    printf("CPU clock   = %d Hz\r\n", CPUClock);
    printf("AHB clock   = %d Hz\r\n", AHBClock);
    printf("APB1 clock  = %d Hz\r\n", APB1Clock);
    printf("APB2 clock  = %d Hz\r\n", APB2Clock);

    switch (gTestNumber)
    {
      case 0: //FAULT_ID_NMI:
        /* Generate the non maskable exception */
        SCB->ICSR |= SCB_ICSR_NMIPENDSET_Msk;
        break;

      case 1: //FAULT_ID_HARDFAULT:
        break;

      case 2: //FAULT_ID_MEMMANAGE:
        break;

      case 3: //FAULT_ID_BUSFAULT:
        /* Generate the bus exception */
        testValueA = (U32)*((U32 *)0x07F00000);
        break;

      case 4: //FAULT_ID_USAGEFAULT:
        /* Generate usage fault - division by zero */
        testValueA = 13;
        testValueB = 0;
        testValueA /= testValueB;
        break;

      case 5: //FAULT_ID_DEBUGMON:
        break;

      default:
        break;
    }

    Delay();

    gTestNumber++;
    NVIC_SystemReset();
  }
  //else
  //{
  //  printf("All tests finished!\r\n");
  //  gTestMark = TEST_MARK_FINISHED;
  //  while(TRUE) {};
  //}

  //xTaskCreate(vLEDTask,"LEDTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

  //vTaskStartScheduler();

  while(TRUE) {};
}
