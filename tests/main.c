#include <stdio.h>

#include "stm32f1xx.h"
#include "debug.h"
#include "uniquedevid.h"
#include "FreeRTOS.h"
#include "task.h"

//-----------------------------------------------------------------------------

typedef FW_BOOLEAN (* TestFunction_t)(void);

static U32    gPass             = 0;
static U32    gFail             = 0;
static U32    gTested           = 0;
static U32    gTotal            = 0;

extern void vTestHelpTaskFunction( void * pvParameters );
extern U32 TestsGetCount(void);
extern const TestFunction_t gTests[];

//-----------------------------------------------------------------------------

static void vLogTestResult(FW_BOOLEAN result)
{
    if (FW_TRUE == result)
    {
        LOG("-----> PASS\r\n");
        gPass++;
    }
    else
    {
        LOG("-----> FAIL\r\n");
        gFail++;
    }
    gTested++;
}

//-----------------------------------------------------------------------------

void vTestMainTask(void * pvParameters)
{
    FW_BOOLEAN result = FW_FALSE;
    U32        test   = 0;

    LOG("-------------------------------------------\r\n");
    LOG("Test Main Task Started\r\n");

    gTotal = TestsGetCount();

    for (test = 0; test < gTotal; test++)
    {
        LOG("-------------------------------------------\r\n");
        result = gTests[test]();
        vLogTestResult(result);
        if (FW_FALSE == result) break;
    }

    LOG("-------------------------------------------\r\n");
    LOG(" - Total  = %d\r\n", gTotal);
    LOG(" - Tested = %d  Pass = %d  Fail = %d\r\n", gTested, gPass, gFail);
    LOG("-------------------------------------------\r\n");

    while(FW_TRUE)
    {
        vTaskDelay(500);
    }
}

//-----------------------------------------------------------------------------

int main(void)
{
    LOG("STM32F103C8 Started!\r\n");
    LOG("ID0 = 0x%04X\r\n", UDID_0);
    LOG("ID1 = 0x%04X\r\n", UDID_1);
    LOG("ID2 = 0x%08X\r\n", UDID_2);
    LOG("ID2 = 0x%08X\r\n", UDID_3);
    LOG("Memory Size = %d kB\r\n", FLASH_SIZE);
    LOG("SysClock = %d Hz\r\n", SystemCoreClock);

    LOG("Preparing Test Tasks To Run...\r\n");

    xTaskCreate
    (
        vTestMainTask,
        "TestMainTask",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );

    xTaskCreate
    (
        vTestHelpTaskFunction,
        "TestHelpTask",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );

    vTaskStartScheduler();

    while(FW_TRUE) {};
}

//-----------------------------------------------------------------------------
