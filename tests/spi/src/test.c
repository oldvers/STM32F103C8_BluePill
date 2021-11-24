#include <stdio.h>
#include <string.h>

#include "types.h"
#include "debug.h"
#include "board.h"
#include "gpio.h"
#include "spi.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

typedef FW_BOOLEAN (* TestFunction_t)(void);

/* -------------------------------------------------------------------------- */

#define SPI_BUFFER_SIZE      (32)
//#define BH1750_ADDRESS       (0x23)
//#define BH1750_CMD_POWER_ON  (0x01)
//#define BH1750_CMD_HRES2     (0x21)
//#define LPH8616_ADDRESS      (0x3C)

#define EVT_SPI_COMPLETE     (1 << 0)

/* -------------------------------------------------------------------------- */

static EventGroupHandle_t gEventGroup                = NULL;
static U8                 gTxBuffer[SPI_BUFFER_SIZE] = {0};
static U8                 gRxBuffer[SPI_BUFFER_SIZE] = {0};
static FW_RESULT          gResult                    = FW_FAIL;

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN spi_Complete(FW_RESULT aResult)
{
  BaseType_t xHigherPriorityTaskWoken, xResult;

  xHigherPriorityTaskWoken = pdFALSE;

  xResult = xEventGroupSetBitsFromISR
            (
              gEventGroup,
              EVT_SPI_COMPLETE,
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

static FW_BOOLEAN spi_WaitForComplete(void)
{
  FW_BOOLEAN result = FW_TRUE;
  EventBits_t uxBits = 0;

  DBG(" - Wait for completion\r\n");
  uxBits = xEventGroupWaitBits
           (
             gEventGroup,
             EVT_SPI_COMPLETE,
             pdTRUE, //EVT_SPI_COMPLETE should be cleared before returning
             pdFALSE, //Don't wait for both bits, either bit will do
             100
           );

  if (EVT_SPI_COMPLETE == (uxBits & EVT_SPI_COMPLETE))
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

static FW_BOOLEAN Test_SPI1_Init(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("*** The SPI1 MISO/MOSI lines must be connected together! ***\r\n");

  DBG("-----------------------------------------------------------\r\n");

  DBG(" - Initialize the SPI1 peripheral\r\n");
  SPI_Init(SPI_1, spi_Complete);

  DBG(" - Initialize the SPI bus pins\r\n");
  GPIO_Init(SPI1_CS_PORT,   SPI1_CS_PIN,   GPIO_TYPE_OUT_PP_2MHZ,  1);
  GPIO_Init(SPI1_SCK_PORT,  SPI1_SCK_PIN,  GPIO_TYPE_ALT_PP_50MHZ, 0);
  GPIO_Init(SPI1_MOSI_PORT, SPI1_MOSI_PIN, GPIO_TYPE_ALT_PP_50MHZ, 0);
  GPIO_Init(SPI1_MISO_PORT, SPI1_MISO_PIN, GPIO_TYPE_ALT_PP_50MHZ, 0);

  DBG(" - Clear all the events\r\n");
  xEventGroupClearBits(gEventGroup,	EVT_SPI_COMPLETE);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_SPI1_Exchange(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("-----------------------------------------------------------\r\n");

  DBG(" - Clear all the events\r\n");
  xEventGroupClearBits(gEventGroup,	EVT_SPI_COMPLETE);

  DBG(" - Send buffer to SPI1...\r\n");

  gTxBuffer[0] = 0x00;
  gTxBuffer[1] = 0x11;
  gTxBuffer[2] = 0x22;
  gTxBuffer[3] = 0x33;
  gTxBuffer[4] = 0x44;
  gTxBuffer[5] = 0x55;
  gTxBuffer[6] = 0x66;
  gTxBuffer[7] = 0x77;
  gTxBuffer[8] = 0x88;
  gTxBuffer[9] = 0x99;
  memset(gRxBuffer, 0, 10);

  GPIO_Lo(SPI1_CS_PORT, SPI1_CS_PIN);
  SPI_MExchange(SPI_1, gTxBuffer, gRxBuffer, 10);

  result = spi_WaitForComplete();
  GPIO_Hi(SPI1_CS_PORT, SPI1_CS_PIN);

  if ( (FW_TRUE == result) && (0 == memcmp(gTxBuffer, gRxBuffer, 10)) )
  {
    DBG(" - Echo received from SPI1\r\n");
  }
  else
  {
    DBG(" - No any data received from SPI1\r\n");
    result &= FW_FALSE;
  }

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_SPI2_Init(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("*** The SPI2 MISO/MOSI lines must be connected together! ***\r\n");

  DBG("-----------------------------------------------------------\r\n");

  DBG(" - Initialize the SPI2 peripheral\r\n");
  SPI_Init(SPI_2, spi_Complete);

  DBG(" - Initialize the SPI bus pins\r\n");
  GPIO_Init(SPI2_CS_PORT,   SPI2_CS_PIN,   GPIO_TYPE_OUT_PP_2MHZ,  1);
  GPIO_Init(SPI2_SCK_PORT,  SPI2_SCK_PIN,  GPIO_TYPE_ALT_PP_50MHZ, 0);
  GPIO_Init(SPI2_MOSI_PORT, SPI2_MOSI_PIN, GPIO_TYPE_ALT_PP_50MHZ, 0);
  GPIO_Init(SPI2_MISO_PORT, SPI2_MISO_PIN, GPIO_TYPE_ALT_PP_50MHZ, 0);

  DBG(" - Clear all the events\r\n");
  xEventGroupClearBits(gEventGroup,	EVT_SPI_COMPLETE);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_SPI2_Exchange(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("-----------------------------------------------------------\r\n");

  DBG(" - Clear all the events\r\n");
  xEventGroupClearBits(gEventGroup,	EVT_SPI_COMPLETE);

  DBG(" - Send buffer to SPI2...\r\n");

  gTxBuffer[0] = 0x99;
  gTxBuffer[1] = 0x88;
  gTxBuffer[2] = 0x77;
  gTxBuffer[3] = 0x66;
  gTxBuffer[4] = 0x55;
  gTxBuffer[5] = 0x44;
  gTxBuffer[6] = 0x33;
  gTxBuffer[7] = 0x22;
  gTxBuffer[8] = 0x11;
  gTxBuffer[9] = 0x00;
  memset(gRxBuffer, 0, 10);

  GPIO_Lo(SPI2_CS_PORT, SPI2_CS_PIN);
  SPI_MExchange(SPI_2, gTxBuffer, gRxBuffer, 10);

  result = spi_WaitForComplete();
  GPIO_Hi(SPI2_CS_PORT, SPI2_CS_PIN);

  if ( (FW_TRUE == result) && (0 == memcmp(gTxBuffer, gRxBuffer, 10)) )
  {
    DBG(" - Echo received from SPI2\r\n");
  }
  else
  {
    DBG(" - No any data received from SPI2\r\n");
    result &= FW_FALSE;
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
    Test_SPI1_Init,
    Test_SPI1_Exchange,
    Test_SPI2_Init,
    Test_SPI2_Exchange,
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
