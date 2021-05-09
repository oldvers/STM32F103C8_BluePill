#include <stdio.h>
#include <string.h>

#include "stm32f1xx.h"
#include "types.h"
#include "gpio.h"
#include "uniquedevid.h"
#include "system.h"
#include "debug.h"
#include "SEGGER_RTT.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* --- Interrupts priority grouping ----------------------------------------- */

#define IRQ_PRIORITY_GROUP_16_SUB_01    (3)

/* --- Definitions ---------------------------------------------------------- */

#define MAX_TEST_NUMBER                 (6)
#define TEST_MARK_IN_PROGRESS           (0xFACECAFE)
#define TEST_MARK_FINISHED              (0xCAFE0BAD)

static const S32 gTestsMap[MAX_TEST_NUMBER] =
{
  NonMaskableInt_IRQn,
  HardFault_IRQn,
  MemoryManagement_IRQn,
  BusFault_IRQn,
  UsageFault_IRQn,
  DebugMonitor_IRQn,
};

__no_init static U32 gTestMark;
__no_init static U32 gTestNumber;

/* --- Functions ------------------------------------------------------------ */

static void Delay(void)
{
  U32 delay = 2000000;

  while (delay) { delay--; }
}

/* -------------------------------------------------------------------------- */

void on_error(S32 parameter)
{
   S32 FaultID = parameter;

  if (gTestsMap[gTestNumber] == FaultID)
  {
    DBG_SetTextColorGreen();
    printf("--- PASS!\r\n");
  }
  else
  {
    DBG_SetTextColorRed();
    printf("--- FAIL!\r\n");
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

    /* Specific function call only for this exact test */
    SEGGER_RTT_Reset();

    /* Init the  */
    DBG_Init();
    DBG_SetDefaultColors();
    DBG_ClearScreen();
    Delay();
  }

  if (MAX_TEST_NUMBER <= gTestNumber)
  {
    DBG_SetTextColorBlue();
    printf("*** All tests finished! *********************\r\n");
    gTestMark = TEST_MARK_FINISHED;
    while(TRUE) {};
  }
  else
  {
    DBG_SetTextColorBlue();
    printf("*** Test Number = %d *************************\r\n", gTestNumber);

    DBG_SetDefaultColors();
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

    switch (gTestsMap[gTestNumber])
    {
      case NonMaskableInt_IRQn:
        /* Generate the non maskable exception */
        SCB->ICSR |= SCB_ICSR_NMIPENDSET_Msk;
        break;

      case HardFault_IRQn:
        /* Route bus fault to hard fault */
        SCB->SHCSR &= ~( SCB_SHCSR_BUSFAULTENA_Msk );

        /* Generate the hard fault exception */
        testValueA = 0x33;
        *((U32 *)0x07F00000) = testValueA;
        break;

      case MemoryManagement_IRQn:
        /* Generate the memory management exception */
        SCB->SHCSR |= ( SCB_SHCSR_MEMFAULTPENDED_Msk );
        break;

      case BusFault_IRQn:
        /* Generate the bus exception */
        testValueA = (U32)*((U32 *)0x07F00000);
        break;

      case UsageFault_IRQn:
        /* Generate usage fault - division by zero */
        testValueA = 13;
        testValueB = 0;
        testValueA /= testValueB;
        break;

      case DebugMonitor_IRQn:
        /* Generate the debug monitor exception */
        CoreDebug->DEMCR |= ( CoreDebug_DEMCR_MON_REQ_Msk |
                              CoreDebug_DEMCR_MON_EN_Msk );
        CoreDebug->DEMCR |= ( CoreDebug_DEMCR_MON_PEND_Msk );
        break;

      default:
        break;
    }

    Delay();

    gTestNumber++;
    NVIC_SystemReset();
  }

  while(TRUE) {};
}
