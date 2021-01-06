#include "stm32f1xx.h"
#include "types.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "fifo.h"

//-----------------------------------------------------------------------------

typedef FW_BOOLEAN (* TestFunction_t)(void);

static FIFO_p pFIFO          = NULL;
static U8     fifoBuffer[25] = {0};

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

static FW_BOOLEAN vTest_Init(void)
{
    FW_BOOLEAN result = FW_FALSE;

    LOG("*** FIFO Success Initialization Test ***\r\n");

    pFIFO = FIFO_Init(fifoBuffer, sizeof(fifoBuffer));
    result = (FW_BOOLEAN)(NULL != pFIFO);

    LOG("FIFO Capacity = %d\r\n", FIFO_Size(pFIFO));

    return result;
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN vTest_Put8(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    LOG("*** FIFO Put 8 Test ***\r\n");

    result = FW_TRUE;
    for (i = 0; i < 8; i++)
    {
        data = 0x33;
        status = FIFO_Put(pFIFO, &data);
        result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    }
    LOG("--- FIFO Put 8 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));

    return result;
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN vTest_Get5(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    LOG("*** FIFO Get 5 Test ***\r\n");

    result = FW_TRUE;
    for (i = 0; i < 5; i++)
    {
        status = FIFO_Get(pFIFO, &data);
        result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    }
    LOG("--- FIFO Get 5 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(3 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(5 == FIFO_Free(pFIFO));

    return result;
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN vTest_Put10(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    LOG("*** FIFO Put 10 Test ***\r\n");

    result = FW_TRUE;
    for (i = 0; i < 10; i++)
    {
        data = 0x44;
        status = FIFO_Put(pFIFO, &data);
        if (5 > i)
        {
            result &= (FW_BOOLEAN)(FW_SUCCESS == status);
        }
        else
        {
            result &= (FW_BOOLEAN)(FW_FULL == status);
        }
    }
    LOG("--- FIFO Put 10 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));

    return result;
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN vTest_Get3(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    LOG("*** FIFO Get 3 Test ***\r\n");

    result = FW_TRUE;
    for (i = 0; i < 3; i++)
    {
        status = FIFO_Get(pFIFO, &data);
        result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    }
    LOG("--- FIFO Get 3 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(5 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(3 == FIFO_Free(pFIFO));

    return result;
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN vTest_Put13(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    LOG("*** FIFO Put 13 Test ***\r\n");

    result = FW_TRUE;
    for (i = 0; i < 13; i++)
    {
        data = 0x55;
        status = FIFO_Put(pFIFO, &data);
        if (3 > i)
        {
            result &= (FW_BOOLEAN)(FW_SUCCESS == status);
        }
        else
        {
            result &= (FW_BOOLEAN)(FW_FULL == status);
        }
    }
    LOG("--- FIFO Put 13 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));

    return result;
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN vTest_Get20(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    LOG("*** FIFO Get 20 Test ***\r\n");

    result = FW_TRUE;
    for (i = 0; i < 20; i++)
    {
        status = FIFO_Get(pFIFO, &data);
        if (8 > i)
        {
            result &= (FW_BOOLEAN)(FW_SUCCESS == status);
        }
        else
        {
            result &= (FW_BOOLEAN)(FW_EMPTY == status);
        }
    }
    LOG("--- FIFO Get 20 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Free(pFIFO));

    return result;
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN vTest_Get13Again(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    LOG("*** FIFO Get 13 Again Test ***\r\n");

    result = FW_TRUE;
    for (i = 0; i < 13; i++)
    {
        data = 0x66;
        status = FIFO_Put(pFIFO, &data);
        if (8 > i)
        {
            result &= (FW_BOOLEAN)(FW_SUCCESS == status);
        }
        else
        {
            result &= (FW_BOOLEAN)(FW_FULL == status);
        }
    }
    LOG("--- FIFO Put 13 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));

    return result;
}

//-----------------------------------------------------------------------------

const TestFunction_t gTests[] =
{
    vTest_Init,
    vTest_Put8,
    vTest_Get5,
    vTest_Put10,
    vTest_Get3,
    vTest_Put13,
    vTest_Get20,
    vTest_Get13Again,
};

U32 TestsGetCount(void)
{
    return (sizeof(gTests) / sizeof(TestFunction_t));
}

//-----------------------------------------------------------------------------
