#include "stm32f1xx.h"
#include "types.h"
#include "gpio.h"
#include "debug.h"
#include "uniquedevid.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "usb_device.h"
#include "blockqueue.h"

//-----------------------------------------------------------------------------

#define BLOCK_QUEUE_BUFFER_SIZE 128

typedef struct Block_s
{
    U8  length;
    U8  type;
    U8  command;
    U32 parameter;
    U32 value;
} Block_t, * Block_p;

typedef FW_BOOLEAN (* TestFunction_t)(void);

static BlockQueue_p pQueue                               = NULL;
static U8           QueueBuffer[BLOCK_QUEUE_BUFFER_SIZE] = {0};
static U32          gPass                                = 0;
static U32          gFail                                = 0;
static U32          gTotal                               = 0;
static U32          gTested                              = 0;

//--- Mocks -------------------------------------------------------------------

void USB_IRQHandler(void)
{
  //
}

//-----------------------------------------------------------------------------

static void vUpdateTestResult(FW_BOOLEAN aResult)
{
    if (FW_TRUE == aResult)
    {
        gPass++;
    }
    else
    {
        gFail++;
    }
    gTested++;
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN Test_InitSuccess(void)
{
    FW_BOOLEAN result = FW_FALSE;

    LOG("Block Queue Success Initialization Test\r\n");
    pQueue = BlockQueue_Init
             (
                 QueueBuffer,
                 sizeof(QueueBuffer),
                 sizeof(Block_t)
             );
    result = (FW_BOOLEAN)(NULL != pQueue);

    vUpdateTestResult(result);

    return result;
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN Test_InitSizeNotFit(void)
{
    FW_BOOLEAN result = FW_FALSE;

    LOG("Block Queue Size Not Fit Initialization Test\r\n");
    pQueue = BlockQueue_Init
             (
                 QueueBuffer,
                 32 + 12 + 8,
                 12
             );
    result = (FW_BOOLEAN)(NULL == pQueue);

    vUpdateTestResult(result);

    return result;
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN Test_AllocateSuccess(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U8 * pBlock = NULL;
    U32 size = 0;

    LOG("Block Queue Success Allocation Test\r\n");

    status = BlockQueue_Allocate(pQueue, &pBlock, &size);

    result = (FW_BOOLEAN)(FW_SUCCESS == status);
    result &= (FW_BOOLEAN)(NULL != pBlock);
    result &= (FW_BOOLEAN)(0 != size);

    vUpdateTestResult(result);

    return result;
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN Test_AllocateError(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U8 * pBlock = NULL;
    U32 size = 0;

    LOG("Block Queue Error Allocation Test\r\n");

    status = BlockQueue_Allocate(pQueue, &pBlock, &size);

    result = (FW_BOOLEAN)(FW_ERROR == status);
    result &= (FW_BOOLEAN)(NULL != pBlock);
    result &= (FW_BOOLEAN)(0 != size);

    vUpdateTestResult(result);

    return result;
}

//-----------------------------------------------------------------------------

void vLEDTask(void * pvParameters)
{
    //LOG("LED Task Started\r\n");

    while(FW_TRUE)
    {
        //LOG("LED Hi\r\n");
        vTaskDelay(50);
        //LOG("LED Lo\r\n");
        vTaskDelay(50);
    }
    //vTaskDelete(NULL);
}

//-----------------------------------------------------------------------------

static const TestFunction_t gTests[] =
{
    Test_InitSizeNotFit,
    Test_InitSuccess,
    Test_AllocateSuccess,
    Test_AllocateError,
};

void vTemplateTask(void * pvParameters)
{
    FW_BOOLEAN result = FW_FALSE;
    U32        test   = 0;

    LOG("-------------------------------------------\r\n");
    LOG("Block Queue Test Task Started\r\n");

    gTotal = sizeof(gTests) / sizeof(TestFunction_t);

    for (test = 0; test < gTotal; test++)
    {
        LOG("-------------------------------------------\r\n");
        result = gTests[test]();
        if (FW_FALSE == result) break;
    }

    LOG("-------------------------------------------\r\n");
    LOG(" - Total  = %d\r\n", gTotal);
    LOG(" - Tested = %d  Pass = %d  Fail = %d\r\n", gTested, gPass, gFail);
    LOG("-------------------------------------------\r\n");

    vTaskDelay(200);

    while(FW_TRUE)
    {
        //LOG("Template Task Iteration\r\n");
        vTaskDelay(500);
    }
    //vTaskDelete(NULL);
}

//-----------------------------------------------------------------------------

void Prepare_And_Run_Test( void )
{
    LOG("Preparing Template Test To Run...\r\n");

    xTaskCreate
    (
        vLEDTask,
        "LED",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );

    xTaskCreate
    (
        vTemplateTask,
        "Template",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );
}

//-----------------------------------------------------------------------------
