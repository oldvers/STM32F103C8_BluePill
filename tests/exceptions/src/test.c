#include <stdio.h>
#include <string.h>

#include "stm32f1xx.h"
#include "types.h"
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

typedef FW_BOOLEAN (* TestFunction_t)(void);

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
__no_init static U32 gPass;
__no_init static U32 gFail;

/* --- Functions ------------------------------------------------------------ */

static void Delay(void)
{
  U32 delay = 2000000;

  while (delay) { delay--; }
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ExceptionHandler(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U32 testValueA = 0, testValueB = 0;

  DBG_SetTextColorBlue();
  DBG("*** Test Number = %d *************************\r\n", gTestNumber);
  DBG_SetDefaultColors();

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

  return result;
}

/* --- Test Start Up Function (mandatory, called before RTOS starts) -------- */

void vTestStartUpFunction(void)
{
  /* Check for the start of the test */
  if (TEST_MARK_IN_PROGRESS != gTestMark)
  {
    gTestMark = TEST_MARK_IN_PROGRESS;
    gTestNumber = 0;
    gPass = 0;
    gFail = 0;

    /* Specific function call only for this exact test */
    SEGGER_RTT_Reset();

    /* Init the DBG unit */
    DBG_Init();
    DBG_SetDefaultColors();
    DBG_ClearScreen();
    Delay();
  }
}

/* --- Test Prepare Function (mandatory, called before RTOS starts) --------- */

void vTestPrepareFunction (void)
{
  if (MAX_TEST_NUMBER <= gTestNumber)
  {
    DBG_SetTextColorYellow();
    DBG("-----------------------------------------------------------\r\n");

    DBG_SetTextColorBlue();
    DBG("*** All tests finished! *********************\r\n");

    DBG_SetDefaultColors();
    DBG(" - Total  = %d\r\n", MAX_TEST_NUMBER);
    DBG(" - Tested = %d  ", gTestNumber);

    if (0 < gPass)
    {
      DBG_SetTextColorGreen();
    }
    else
    {
      DBG_SetDefaultColors();
    }
    DBG("Pass = %d  ", gPass);

    if (0 < gFail)
    {
      DBG_SetTextColorRed();
    }
    else
    {
      DBG_SetDefaultColors();
    }
    DBG("Fail = %d\r\n", gFail);

    DBG_SetTextColorYellow();
    DBG("-----------------------------------------------------------\r\n");
    DBG_SetDefaultColors();

    gTestMark = TEST_MARK_FINISHED;

    while (FW_TRUE) {};
  }
}

/* --- Helper Task Main Function (mandatory) -------------------------------- */

void vTestHelpTaskFunction(void * pvParameters)
{
  while (FW_TRUE)
  {
      vTaskDelay(500);
  }
  //vTaskDelete(NULL);
}

/* --- Test Cases List (mandatory) ------------------------------------------ */

const TestFunction_t gTests[] =
{
    Test_ExceptionHandler,
};

U32 uiTestsGetCount(void)
{
    return (sizeof(gTests) / sizeof(TestFunction_t));
}

/* --- Error callback function (mandatory) ---------------------------------- */

void on_error(S32 parameter)
{
   S32 FaultID = parameter;

  if (gTestsMap[gTestNumber] == FaultID)
  {
    DBG_SetTextColorGreen();
    DBG(" ----> PASS!\r\n");
    gPass++;
  }
  else
  {
    DBG_SetTextColorRed();
    DBG(" ----> FAIL!\r\n");
    gFail++;
  }

  Delay();

  gTestNumber++;
  NVIC_SystemReset();
}

/* -------------------------------------------------------------------------- */
