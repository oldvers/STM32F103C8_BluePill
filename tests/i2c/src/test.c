#include <stdio.h>

#include "types.h"
#include "debug.h"
#include "board.h"
#include "gpio.h"
#include "i2c.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

typedef FW_BOOLEAN (* TestFunction_t)(void);

/* -------------------------------------------------------------------------- */

#define I2C_BUFFER_SIZE      (32)
#define BH1750_ADDRESS       (0x23)
#define BH1750_CMD_POWER_ON  (0x01)
#define BH1750_CMD_HRES2     (0x21)
#define LPH8616_ADDRESS      (0x3C)

#define EVT_I2C_COMPLETE     (1 << 0)

/* -------------------------------------------------------------------------- */

static EventGroupHandle_t gEventGroup                = NULL;
static U8                 gTxBuffer[I2C_BUFFER_SIZE] = {0};
static U8                 gRxBuffer[I2C_BUFFER_SIZE] = {0};
static FW_RESULT          gResult                    = FW_FAIL;

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN i2c_Complete(FW_RESULT aResult)
{
  BaseType_t xHigherPriorityTaskWoken, xResult;

  xHigherPriorityTaskWoken = pdFALSE;

  xResult = xEventGroupSetBitsFromISR
            (
              gEventGroup,
              EVT_I2C_COMPLETE,
              &xHigherPriorityTaskWoken
            );
  gResult = aResult;

  if( xResult == pdPASS )
  {
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }

  return FW_TRUE;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN i2c_WaitForComplete(void)
{
  FW_BOOLEAN result = FW_TRUE;
  EventBits_t uxBits = 0;

  DBG(" - Wait for completion\r\n");
  uxBits = xEventGroupWaitBits
           (
             gEventGroup,
             EVT_I2C_COMPLETE,
             pdTRUE, //EVT_I2C_COMPLETE should be cleared before returning
             pdFALSE, //Don't wait for both bits, either bit will do
             100
           );

  if (EVT_I2C_COMPLETE == (uxBits & EVT_I2C_COMPLETE))
  {
    if (FW_COMPLETE == gResult)
    {
      DBG("   - SUCCESS\r\n");
    }
    else
    {
      DBG("   - ERROR: Communication fails!\r\n");
      result = FW_FALSE;
    }
  }
  else
  {
    DBG("   - ERROR: Timeout!\r\n");
    result = FW_FALSE;
  }

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_I2C_Init(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("*** The BH1750 sensor must be connected to the I2C bus! ***\r\n");

  DBG("-----------------------------------------------------------\r\n");

  DBG(" - Initialize the I2C peripheral\r\n");
  I2C_Init(I2C_1, i2c_Complete);

  DBG(" - Initialize the I2C bus pins\r\n");
  GPIO_Init(I2C1_SCL_PORT, I2C1_SCL_PIN, GPIO_TYPE_ALT_OD_10MHZ, 1);
  GPIO_Init(I2C1_SDA_PORT, I2C1_SDA_PIN, GPIO_TYPE_ALT_OD_10MHZ, 1);

  DBG(" - Clear all the events\r\n");
  xEventGroupClearBits(gEventGroup,	EVT_I2C_COMPLETE);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_BH1750_Init(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("-----------------------------------------------------------\r\n");

  DBG(" - Clear all the events\r\n");
  xEventGroupClearBits(gEventGroup,	EVT_I2C_COMPLETE);

  DBG(" - Send BH1750 Power On command\r\n");
  gTxBuffer[0] = BH1750_CMD_POWER_ON;
  I2C_MWr(I2C_1, BH1750_ADDRESS, gTxBuffer, 1);

  result = i2c_WaitForComplete();

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_BH1750_Read(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U32 i = 0;
  float value = 0.0;

  for (i = 0; i < 5; i++)
  {
    DBG("-----------------------------------------------------------\r\n");

    DBG(" - Clear all the events\r\n");
    xEventGroupClearBits(gEventGroup,	EVT_I2C_COMPLETE);

    DBG(" - Send BH1750 Power On command\r\n");
    gTxBuffer[0] = BH1750_CMD_POWER_ON;
    I2C_MWr(I2C_1, BH1750_ADDRESS, gTxBuffer, 1);

    result &= i2c_WaitForComplete();
    if (FW_FALSE == result) break;

    DBG(" - Clear all the events\r\n");
    xEventGroupClearBits(gEventGroup,	EVT_I2C_COMPLETE);

    DBG(" - Send BH1750 High Resolution Mode 2 command\r\n");
    gTxBuffer[0] = BH1750_CMD_HRES2;
    I2C_MWr(I2C_1, BH1750_ADDRESS, gTxBuffer, 1);

    result &= i2c_WaitForComplete();
    if (FW_FALSE == result) break;

    vTaskDelay(200);

    DBG(" - Clear all the events\r\n");
    xEventGroupClearBits(gEventGroup,	EVT_I2C_COMPLETE);

    DBG(" - Read BH1750 value\r\n");
    gRxBuffer[0] = 0;
    gRxBuffer[1] = 0;
    I2C_MRd(I2C_1, BH1750_ADDRESS, gRxBuffer, 2);

    result &= i2c_WaitForComplete();
    if (FW_FALSE == result) break;

    value = (U16)(*((U16 *)gRxBuffer)) / 1.2;
    DBG_SetTextColorYellow();
    DBG(" - BH1750 value = %5.2f\r\n", value);
    DBG_SetDefaultColors();
  }

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

  DBG(" - Create an event group\r\n");
  gEventGroup = xEventGroupCreate();
	if (NULL == gEventGroup)
	{
		DBG(" --- ERROR!\r\n");
	}
}

/* --- Helper Task Main Function (mandatory) -------------------------------- */

void vTestHelpTaskFunction(void * pvParameters)
{
  DBG("LED Task Started..\r\n");
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
    Test_I2C_Init,
    Test_BH1750_Init,
    Test_BH1750_Read,
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
