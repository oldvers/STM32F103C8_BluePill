#include <stdio.h>

#include "types.h"
#include "debug.h"
#include "system.h"
#include "uniquedevid.h"

#include "FreeRTOS.h"
#include "task.h"

/* -------------------------------------------------------------------------- */

typedef FW_BOOLEAN (* TestFunction_t)(void);

static U32 gPass                  = 0;
static U32 gFail                  = 0;
static U32 gTested                = 0;
static U32 gTotal                 = 0;

extern void  vTestStartUpFunction ( void );
extern void  vTestPrepareFunction ( void );
extern void  vTestHelpTaskFunction( void * pvParameters );
extern U32   uiTestsGetCount      ( void );

extern const TestFunction_t       gTests[];

/* -------------------------------------------------------------------------- */

static void vLogTestResult(FW_BOOLEAN result)
{
  if (FW_TRUE == result)
  {
    DBG_SetTextColorGreen();
    DBG(" ----> PASS\r\n");
    gPass++;
  }
  else
  {
    DBG_SetTextColorRed();
    DBG(" ----> FAIL\r\n");
    gFail++;
  }
  DBG_SetDefaultColors();
  gTested++;
}

/* -------------------------------------------------------------------------- */

void vTestMainTask(void * pvParameters)
{
  FW_BOOLEAN result = FW_FALSE;
  U32        test   = 0;

  DBG("-----------------------------------------------------------\r\n");
  DBG(" --- Test Main Task Started\r\n");

  gTotal = uiTestsGetCount();

  for (test = 0; test < gTotal; test++)
  {
    DBG("-----------------------------------------------------------\r\n");
    result = gTests[test]();
    vLogTestResult(result);
    if (FW_FALSE == result) break;
  }

  DBG("-----------------------------------------------------------\r\n");
  DBG(" - Total  = %d\r\n", gTotal);
  DBG(" - Tested = %d  Pass = %d  Fail = %d\r\n", gTested, gPass, gFail);
  DBG("-----------------------------------------------------------\r\n");

  while (FW_TRUE)
  {
    vTaskDelay(500);
  }
}

/* -------------------------------------------------------------------------- */

int main(void)
{
  DBG_Init();
  DBG_SetDefaultColors();

  vTestStartUpFunction();

  DBG("-----------------------------------------------------------\r\n");
  DBG("STM32F103C8 Started!\r\n");
  DBG("ID0         = 0x%04X\r\n", UDID_0);
  DBG("ID1         = 0x%04X\r\n", UDID_1);
  DBG("ID2         = 0x%08X\r\n", UDID_2);
  DBG("ID2         = 0x%08X\r\n", UDID_3);
  DBG("Memory Size = %d kB\r\n", FLASH_SIZE);
  DBG("CPU clock   = %d Hz\r\n", CPUClock);
  DBG("AHB clock   = %d Hz\r\n", AHBClock);
  DBG("APB1 clock  = %d Hz\r\n", APB1Clock);
  DBG("APB2 clock  = %d Hz\r\n", APB2Clock);

  DBG("Preparing Test Tasks To Run...\r\n");

  vTestPrepareFunction();

  xTaskCreate
  (
    vTestMainTask,
    "TestMainTask",
    configMINIMAL_STACK_SIZE * 2,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );

  xTaskCreate
  (
    vTestHelpTaskFunction,
    "TestHelpTask",
    configMINIMAL_STACK_SIZE * 2,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );

  vTaskStartScheduler();

  while (FW_TRUE) {};
}

/* -------------------------------------------------------------------------- */
