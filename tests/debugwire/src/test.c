#include <stdio.h>

#include "types.h"
#include "debug.h"
#include "board.h"
#include "gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "dwire_processor.h"

typedef FW_BOOLEAN (* TestFunction_t)(void);

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Sync(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("*** dWire Test Sync ***\r\n");

  DWire_Init();

  result = DWire_Sync();

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ReadSignature(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U16 sign = 0;

  DBG("*** dWire Test Read Signature ***\r\n");

  sign = DWire_ReadSignature();

  result = (FW_BOOLEAN)(0 != sign);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ReadPc(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U16 sign = 0;

  DBG("*** dWire Test Read PC ***\r\n");

  sign = DWire_ReadPc();

  result = (FW_BOOLEAN)(0 != sign);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ReadRegs(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("*** dWire Test Read Regs ***\r\n");

  result = DWire_ReadRegs(0x1800, NULL, 32);

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
  //DBG("LED Task Started..\r\n");
  GPIO_Init(LED_PORT, LED_PIN, GPIO_TYPE_OUT_OD_2MHZ, 0);

  while(1)
  {
    GPIO_Lo(LED_PORT, LED_PIN);
    //DBG_SetTextColorGreen();
    //printf("LED ON\r\n");
    vTaskDelay(500);
    GPIO_Hi(LED_PORT, LED_PIN);
    //DBG_SetTextColorRed();
    //printf("LED OFF\r\n");
    vTaskDelay(500);
  }
  //vTaskDelete(NULL);
}

/* --- Test Cases List (mandatory) ------------------------------------------ */

const TestFunction_t gTests[] =
{
  Test_Sync,
  Test_ReadSignature,
  Test_ReadPc,
  Test_ReadRegs,
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
