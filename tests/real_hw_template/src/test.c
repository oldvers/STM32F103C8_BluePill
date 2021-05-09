#include <stdio.h>

#include "types.h"
#include "debug.h"
#include "board.h"
#include "gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef FW_BOOLEAN (* TestFunction_t)(void);

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_SomeCase(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("*** Some Case Test ***\r\n");

  return result;
}

/* --- Test Start Up Function (mandatory, called before RTOS starts) -------- */

void vTestStartUpFunction(void)
{
  DBG_ClearScreen();
  DBG("*** Start Up Test ***\r\n");
}

/* --- Test Prepare Function (mandatory, called before RTOS starts) --------- */

void vTestPrepareFunction (void)
{
  DBG("*** Prepare Test ***\r\n");
}

/* --- Helper Task Main Function (mandatory) -------------------------------- */

void vTestHelpTaskFunction(void * pvParameters)
{
  DBG("LED Task Started..\r\n");
  GPIO_Init(LED_PORT, LED_PIN, GPIO_TYPE_OUT_OD_2MHZ, 0);

  while(1)
  {
    GPIO_Lo(LED_PORT, LED_PIN);
    DBG_SetTextColorGreen();
    printf("LED ON\r\n");
    vTaskDelay(500);
    GPIO_Hi(LED_PORT, LED_PIN);
    DBG_SetTextColorRed();
    printf("LED OFF\r\n");
    vTaskDelay(500);
  }
  //vTaskDelete(NULL);
}

/* --- Test Cases List (mandatory) ------------------------------------------ */

const TestFunction_t gTests[] =
{
    Test_SomeCase,
};

U32 uiTestsGetCount(void)
{
    return (sizeof(gTests) / sizeof(TestFunction_t));
}

/* --- Error callback function (mandatory) ---------------------------------- */

void on_error(S32 parameter)
{
  while (FW_TRUE) {};
}

/* -------------------------------------------------------------------------- */
