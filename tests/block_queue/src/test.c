#include <stdio.h>

#include "types.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "block_queue.h"

/* -------------------------------------------------------------------------- */

typedef FW_BOOLEAN (* TestFunction_t)(void);

#define BLOCK_QUEUE_BUFFER_SIZE    (128)

typedef struct Block_s
{
    U8  length;
    U8  type;
    U8  command;
    U32 parameter;
    U32 value;
} Block_t, * Block_p;

static BlockQueue_p pQueue                               = NULL;
static U8           QueueBuffer[BLOCK_QUEUE_BUFFER_SIZE] = {0};

/* --- Tests ---------------------------------------------------------------- */

static FW_BOOLEAN Test_SomeCase(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("*** Some Case Test ***\r\n");

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_InitSuccess(void)
{
    FW_BOOLEAN result = FW_FALSE;

    DBG("*** Block Queue Success Initialization Test ***\r\n");
    pQueue = BlockQueue_Init
             (
                 QueueBuffer,
                 sizeof(QueueBuffer),
                 sizeof(Block_t)
             );
    result = (FW_BOOLEAN)(NULL != pQueue);

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_InitSizeNotFit(void)
{
    FW_BOOLEAN result = FW_FALSE;

    DBG("*** Block Queue Size Not Fit Initialization Test ***\r\n");
    pQueue = BlockQueue_Init
             (
                 QueueBuffer,
                 32 + 12 + 8,
                 12
             );
    result = (FW_BOOLEAN)(NULL == pQueue);

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_AllocateSuccess(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U8 * pBlock = NULL;
    U32 size = 0;

    DBG("*** Block Queue Success Allocation Test ***\r\n");

    status = BlockQueue_Allocate(pQueue, &pBlock, &size);

    result = (FW_BOOLEAN)(FW_SUCCESS == status);
    result &= (FW_BOOLEAN)(NULL != pBlock);
    result &= (FW_BOOLEAN)(0 != size);

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_AllocateError(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U8 * pBlock = NULL;
    U32 size = 0;

    DBG("*** Block Queue Error Allocation Test ***\r\n");

    status = BlockQueue_Allocate(pQueue, &pBlock, &size);

    result = (FW_BOOLEAN)(FW_ERROR == status);
    result &= (FW_BOOLEAN)(NULL != pBlock);
    result &= (FW_BOOLEAN)(0 != size);

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_EnqueueSuccess(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;

    DBG("*** Block Queue Success Enqueue Test ***\r\n");

    status = BlockQueue_Enqueue(pQueue, sizeof(Block_t));

    result = (FW_BOOLEAN)(FW_SUCCESS == status);

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_EnqueueError(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;

    DBG("*** Block Queue Error Enqueue Test ***\r\n");

    status = BlockQueue_Enqueue(pQueue, sizeof(Block_t));

    result = (FW_BOOLEAN)(FW_ERROR == status);

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Reset(void)
{
    FW_BOOLEAN result = FW_TRUE;

    DBG("*** Block Queue Reset Test ***\r\n");

    BlockQueue_Reset(pQueue);

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Full(void)
{
    FW_BOOLEAN result = FW_TRUE;
    FW_RESULT status = FW_ERROR;
    U8 * pBlock = NULL;
    U32 size = 0, item = 0, count = 0;

    DBG("*** Block Queue Full Test ***\r\n");

    /* Reset the queue */
    BlockQueue_Reset(pQueue);

    count = BlockQueue_GetCapacity(pQueue) + 1;

    for (item = 0; item < count; item++)
    {
        /* Allocate the block */
        status = BlockQueue_Allocate(pQueue, &pBlock, &size);
        if (item == (count - 1))
        {
            /* There should be no space in the buffer */
            result &= (FW_BOOLEAN)(FW_FULL == status);
            result &= (FW_BOOLEAN)(NULL == pBlock);
            result &= (FW_BOOLEAN)(0 == size);
        }
        else
        {
            result &= (FW_BOOLEAN)(FW_SUCCESS == status);
            result &= (FW_BOOLEAN)(NULL != pBlock);
            result &= (FW_BOOLEAN)(0 != size);
        }
        if (FW_FALSE == result) break;

        /* Enqueue the block */
        status = BlockQueue_Enqueue(pQueue, sizeof(Block_t));
        if (item == (count - 1))
        {
            /* There should be no allocated blocks */
            result &= (FW_BOOLEAN)(FW_ERROR == status);
        }
        else
        {
            result &= (FW_BOOLEAN)(FW_SUCCESS == status);
        }
        if (FW_FALSE == result) break;
    }

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_SuccessDequeue(void)
{
    FW_BOOLEAN result = FW_TRUE;
    FW_RESULT status = FW_ERROR;
    U8 * pEBlock = NULL, * pDBlock = NULL;
    U32 eSize = 0, dSize = 0;

    DBG("*** Block Queue Success Dequeue Test ***\r\n");

    /* Reset the queue */
    BlockQueue_Reset(pQueue);

    /* Allocate the block */
    status = BlockQueue_Allocate(pQueue, &pEBlock, &eSize);
    result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    result &= (FW_BOOLEAN)(NULL != pEBlock);
    result &= (FW_BOOLEAN)(0 != eSize);
    if (FW_FALSE == result) return result;

    /* Enqueue the block */
    status = BlockQueue_Enqueue(pQueue, sizeof(Block_t));
    result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    if (FW_FALSE == result) return result;

    /* Dequeue the block */
    status = BlockQueue_Dequeue(pQueue, &pDBlock, &dSize);
    result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    result &= (FW_BOOLEAN)(pDBlock == pEBlock);
    result &= (FW_BOOLEAN)(dSize == eSize);

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ErrorDequeue(void)
{
    FW_BOOLEAN result = FW_TRUE;
    FW_RESULT status = FW_ERROR;
    U8 * pBlock = NULL;
    U32 size = 0;

    DBG("*** Block Queue Error Dequeue Test ***\r\n");

    /* Dequeue the block */
    status = BlockQueue_Dequeue(pQueue, &pBlock, &size);
    result &= (FW_BOOLEAN)(FW_ERROR == status);
    result &= (FW_BOOLEAN)(NULL == pBlock);
    result &= (FW_BOOLEAN)(0 == size);

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_SuccessRelease(void)
{
    FW_BOOLEAN result = FW_TRUE;
    FW_RESULT status = FW_ERROR;

    DBG("*** Block Queue Success Release Test ***\r\n");

    /* Release the block */
    status = BlockQueue_Release(pQueue);
    result &= (FW_BOOLEAN)(FW_SUCCESS == status);

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ErrorRelease(void)
{
    FW_BOOLEAN result = FW_TRUE;
    FW_RESULT status = FW_ERROR;

    DBG("*** Block Queue Success Release Test ***\r\n");

    /* Release the block */
    status = BlockQueue_Release(pQueue);
    result &= (FW_BOOLEAN)(FW_ERROR == status);

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_AllocatedFree(void)
{
    FW_BOOLEAN result = FW_TRUE;
    FW_RESULT status = FW_ERROR;
    Block_p pEBlock = NULL, pDBlock = NULL;
    U32 size = 0, count = 0, dSize = 0, eSize = 0;

    DBG("*** Block Queue Allocated/Free Test ***\r\n");

    /* Reset the queue */
    BlockQueue_Reset(pQueue);

    /* Get the capacity of the queue */
    size = BlockQueue_GetCapacity(pQueue);
    DBG("*** Block Queue Capacity  = %d\r\n", size);

    /* Allocate the block */
    DBG("*** Block Queue Allocate\r\n");
    status = BlockQueue_Allocate(pQueue, (U8 **)&pEBlock, &eSize);
    result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    result &= (FW_BOOLEAN)(NULL != pEBlock);
    result &= (FW_BOOLEAN)(0 != eSize);
    if (FW_FALSE == result) return result;

    /* Get Allocated/Free block counts */
    count = BlockQueue_GetCountOfAllocated(pQueue);
    DBG("*** Block Queue Allocated = %d\r\n", count);
    result &= (FW_BOOLEAN)(1 == count);
    count = BlockQueue_GetCountOfFree(pQueue);
    DBG("*** Block Queue Free      = %d\r\n", count);
    result &= (FW_BOOLEAN)((size - 1) == count);
    if (FW_FALSE == result) return result;

    /* Fill the block */
    pEBlock->length = sizeof(Block_t);
    pEBlock->type = 0;
    pEBlock->command = 1;
    pEBlock->parameter = 11;
    pEBlock->value = 13;

     /* Enqueue the block */
    DBG("*** Block Queue Enqueue\r\n");
    status = BlockQueue_Enqueue(pQueue, sizeof(Block_t));
    result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    if (FW_FALSE == result) return result;

    /* Get Allocated/Free block counts */
    count = BlockQueue_GetCountOfAllocated(pQueue);
    DBG("*** Block Queue Allocated = %d\r\n", count);
    result &= (FW_BOOLEAN)(1 == count);
    count = BlockQueue_GetCountOfFree(pQueue);
    DBG("*** Block Queue Free      = %d\r\n", count);
    result &= (FW_BOOLEAN)((size - 1) == count);
    if (FW_FALSE == result) return result;

    /* Dequeue the block */
    DBG("*** Block Queue Dequeue\r\n");
    status = BlockQueue_Dequeue(pQueue, (U8 **)&pDBlock, &dSize);
    result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    result &= (FW_BOOLEAN)(pDBlock == pEBlock);
    result &= (FW_BOOLEAN)(dSize == eSize);

    /* Check the block */
    DBG("*** Block Queue Check Block\r\n");
    result &= (FW_BOOLEAN)(sizeof(Block_t) == dSize);
    result &= (FW_BOOLEAN)(sizeof(Block_t) == pDBlock->length);
    result &= (FW_BOOLEAN)(0  == pDBlock->type);
    result &= (FW_BOOLEAN)(1  == pDBlock->command);
    result &= (FW_BOOLEAN)(11 == pDBlock->parameter);
    result &= (FW_BOOLEAN)(13 == pDBlock->value);
    if (FW_FALSE == result) return result;

    /* Get Allocated/Free block counts */
    count = BlockQueue_GetCountOfAllocated(pQueue);
    DBG("*** Block Queue Allocated = %d\r\n", count);
    result &= (FW_BOOLEAN)(1 == count);
    count = BlockQueue_GetCountOfFree(pQueue);
    DBG("*** Block Queue Free      = %d\r\n", count);
    result &= (FW_BOOLEAN)((size - 1) == count);
    if (FW_FALSE == result) return result;

    /* Release the block */
    DBG("*** Block Queue Release\r\n");
    status = BlockQueue_Release(pQueue);
    result &= (FW_BOOLEAN)(FW_SUCCESS == status);

    /* Get Allocated/Free block counts */
    count = BlockQueue_GetCountOfAllocated(pQueue);
    DBG("*** Block Queue Allocated = %d\r\n", count);
    result &= (FW_BOOLEAN)(0 == count);
    count = BlockQueue_GetCountOfFree(pQueue);
    DBG("*** Block Queue Free      = %d\r\n", count);
    result &= (FW_BOOLEAN)(size == count);
    if (FW_FALSE == result) return result;

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
    Test_InitSizeNotFit,
    Test_InitSuccess,
    Test_AllocateSuccess,
    Test_AllocateError,
    Test_EnqueueSuccess,
    Test_EnqueueError,
    Test_Reset,
    Test_Full,
    Test_SuccessDequeue,
    Test_ErrorDequeue,
    Test_SuccessRelease,
    Test_ErrorRelease,
    Test_AllocatedFree,
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
