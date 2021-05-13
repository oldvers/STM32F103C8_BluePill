#include <stdio.h>

#include "types.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "fifo.h"

typedef FW_BOOLEAN (* TestFunction_t)(void);

static FIFO_p pFIFO          = NULL;
static U8     fifoBuffer[25] = {0};

/* --- Tests ---------------------------------------------------------------- */

static FW_BOOLEAN Test_SomeCase(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("*** Some Case Test ***\r\n");

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Init(void)
{
    FW_BOOLEAN result = FW_FALSE;

    DBG("*** FIFO Success Initialization Test ***\r\n");

    pFIFO = FIFO_Init(fifoBuffer, sizeof(fifoBuffer));
    result = (FW_BOOLEAN)(NULL != pFIFO);

    DBG("FIFO Capacity = %d\r\n", FIFO_Size(pFIFO));

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Put8(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    DBG("*** FIFO Put 8 Test ***\r\n");

    result = FW_TRUE;
    for (i = 0; i < 8; i++)
    {
        data = 0x33;
        status = FIFO_Put(pFIFO, &data);
        result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    }
    DBG("--- FIFO Put 8 ---\r\n");
    DBG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    DBG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    DBG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Get5(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    DBG("*** FIFO Get 5 Test ***\r\n");

    result = FW_TRUE;
    for (i = 0; i < 5; i++)
    {
        status = FIFO_Get(pFIFO, &data);
        result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    }
    DBG("--- FIFO Get 5 ---\r\n");
    DBG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    DBG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    DBG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(3 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(5 == FIFO_Free(pFIFO));

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Put10(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    DBG("*** FIFO Put 10 Test ***\r\n");

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
    DBG("--- FIFO Put 10 ---\r\n");
    DBG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    DBG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    DBG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Get3(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    DBG("*** FIFO Get 3 Test ***\r\n");

    result = FW_TRUE;
    for (i = 0; i < 3; i++)
    {
        status = FIFO_Get(pFIFO, &data);
        result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    }
    DBG("--- FIFO Get 3 ---\r\n");
    DBG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    DBG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    DBG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(5 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(3 == FIFO_Free(pFIFO));

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Put13(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    DBG("*** FIFO Put 13 Test ***\r\n");

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
    DBG("--- FIFO Put 13 ---\r\n");
    DBG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    DBG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    DBG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Get20(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    DBG("*** FIFO Get 20 Test ***\r\n");

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
    DBG("--- FIFO Get 20 ---\r\n");
    DBG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    DBG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    DBG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Free(pFIFO));

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Get13Again(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    DBG("*** FIFO Get 13 Again Test ***\r\n");

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
    DBG("--- FIFO Put 13 ---\r\n");
    DBG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    DBG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    DBG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));

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
  //DBG("LED Task Started\r\n");

  while (FW_TRUE)
  {
      //DBG("LED Hi\r\n");
      vTaskDelay(50);
      //DBG("LED Lo\r\n");
      vTaskDelay(50);
  }
  //vTaskDelete(NULL);
}

/* --- Test Cases List (mandatory) ------------------------------------------ */

const TestFunction_t gTests[] =
{
   Test_SomeCase,
   Test_Init,
   Test_Put8,
   Test_Get5,
   Test_Put10,
   Test_Get3,
   Test_Put13,
   Test_Get20,
   Test_Get13Again,
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
