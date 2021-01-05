#include "stm32f1xx.h"
#include "types.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "east.h"

//-----------------------------------------------------------------------------

typedef FW_BOOLEAN (* TestFunction_t)(void);

static EAST_p pEAST             = NULL;
static U8     eastContainer[25] = {0};
static U8     eastBuffer[100]   = {0};
static U32    gPass             = 0;
static U32    gFail             = 0;
static U32    gTested           = 0;
static U32    gTotal            = 0;

//--- Mocks -------------------------------------------------------------------

void USB_IRQHandler(void)
{
    //
}

//-----------------------------------------------------------------------------

void vTestHelpTaskFunction(void * pvParameters)
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

//static void vTest_Init(void)
//{
//    FW_BOOLEAN result = FW_FALSE;
//    FW_RESULT status = FW_ERROR;
//    U32 i = 0;
//    U8 data = 0;
//
//    LOG("--- FIFO Test ---------------------------\r\n");
//
//    LOG("- FIFO Init\r\n");
//    pFIFO = FIFO_Init(fifoBuffer, sizeof(fifoBuffer));
//    result = (FW_BOOLEAN)(NULL != pFIFO);
//    vLog_Result(result);
//    LOG("FIFO Capacity = %d\r\n", FIFO_Size(pFIFO));
//
//
//    result = FW_TRUE;
//    for (i = 0; i < 8; i++)
//    {
//        data = 0x33;
//        status = FIFO_Put(pFIFO, &data);
//        result &= (FW_BOOLEAN)(FW_SUCCESS == status);
//    }
//    LOG("--- FIFO Put 8 ---\r\n");
//    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
//    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
//    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
//    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
//    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));
//    vLog_Result(result);
//}

static FW_BOOLEAN Test_InitSuccess(void)
{
    FW_BOOLEAN result = FW_FALSE;

    LOG("*** EAST Success Initialization Test ***\r\n");

    pEAST = EAST_Init(eastContainer, sizeof(eastContainer));
    result = (FW_BOOLEAN)(NULL != pEAST);

    return result;
}

//-----------------------------------------------------------------------------

const TestFunction_t gTests[] =
{
    Test_InitSuccess,
};

U32 TestsGetCount(void)
{
    return (sizeof(gTests) / sizeof(TestFunction_t));
}

//-----------------------------------------------------------------------------
