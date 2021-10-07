#include <stdio.h>

#include "types.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "block_queue.h"
#include "east_packet.h"

/* -------------------------------------------------------------------------- */

typedef FW_BOOLEAN (* TestFunction_t)(void);

#define BLOCK_QUEUE_BUFFER_SIZE                         (128)

static EAST_p       pEAST                                = NULL;
static U8           eastContainer[16]                    = {0};
static U8           eastBuffer[13]                       = {0};
static BlockQueue_p pQueue                               = NULL;
static U8           QueueBuffer[BLOCK_QUEUE_BUFFER_SIZE] = {0};

/* -------------------------------------------------------------------------- */

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
  FW_RESULT status = FW_ERROR;

  DBG("*** EAST Block Success Initialization Test ***\r\n");

  DBG(" - Initialise the EAST message\r\n");
  pEAST = EAST_Init(eastContainer, sizeof(eastContainer), NULL, 0);
  result = (FW_BOOLEAN)(NULL != pEAST);
  if (FW_FALSE == result) return result;

  status = EAST_SetBuffer(pEAST, eastBuffer, sizeof(eastBuffer));
  result = (FW_BOOLEAN)(FW_SUCCESS == status);


  DBG(" - Initialise the Block Queue\r\n");
  pQueue = BlockQueue_Init
           (
             QueueBuffer,
             sizeof(QueueBuffer),
             sizeof(eastBuffer)
           );
  result = (FW_BOOLEAN)(NULL != pQueue);

  return result;
}

/* -------------------------------------------------------------------------- */

#define EAST_DATA_SIZE                                  (5)
#define EAST_QUEUE_FULL_COUNT                           (5)

static U8 gTestEast[] =
{
  /* Garbage */
  0x33, 0x33, 0x33, 0x33, 0x33,
  /* Start Token */
  0x24,
  /* Length */
  0x05, 0x00,
  /* Data */
  0x01, 0x02, 0x03, 0x04, 0x05,
  /* Stop Token */
  0x42,
  /* Control Sum */
  0x01, 0x00,
  /* Garbage */
  0x33, 0x33, 0x33, 0x33, 0x33,
};

static FW_BOOLEAN Test_FillTheQueue(void)
{
  FW_BOOLEAN result = FW_FALSE;
  FW_RESULT status = FW_ERROR;
  U8 * buffer = NULL, * alloc_buffer = NULL;
  U32 size = 0, i = 0, alloc_size = 0;

  do
  {
    DBG("*** EAST Block Success Fill Queue Test ***\r\n");

    DBG(" - Allocate the Block from Queue\r\n");
    status = BlockQueue_Allocate(pQueue, &buffer, &size);
    result = (FW_BOOLEAN)(FW_SUCCESS == status);
    result &= (FW_BOOLEAN)(NULL != buffer);
    result &= (FW_BOOLEAN)(0 != size);
    if (FW_FALSE == result) break;

    /* Setup/Reset the EAST packet */
    status = EAST_SetBuffer(pEAST, buffer, size);
    result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    if (FW_FALSE == result) break;


    for (i = 0; i < (sizeof(QueueBuffer) + 3 * sizeof(gTestEast)); i++)
    {
      /* Fill the EAST packet */
      status = EAST_PutByte(pEAST, gTestEast[i % sizeof(gTestEast)]);
      if (FW_COMPLETE == status)
      {
        result &= (FW_BOOLEAN)(buffer[0] == gTestEast[8]);
        result &= (FW_BOOLEAN)(buffer[2] == gTestEast[10]);
        result &= (FW_BOOLEAN)(buffer[4] == gTestEast[12]);
        if (FW_FALSE == result) break;
        DBG(" - EAST Message Collected Successfuly\r\n");


        if (0 == BlockQueue_GetCountOfFree(pQueue))
        {
          continue;
        }

        DBG(" - Enqueue the block\r\n");
        status = BlockQueue_Enqueue(pQueue, EAST_GetDataSize(pEAST));

        if (FW_SUCCESS == status)
        {
          DBG(" - Allocate the new Block from Queue\r\n");
          status = BlockQueue_Allocate(pQueue, &alloc_buffer, &alloc_size);
          if (FW_SUCCESS == status)
          {
            result &= (FW_BOOLEAN)(NULL != alloc_buffer);
            result &= (FW_BOOLEAN)(0 != alloc_size);

            buffer = alloc_buffer;
            size = alloc_size;

            status = EAST_SetBuffer(pEAST, buffer, size);
            result = (FW_BOOLEAN)(FW_SUCCESS == status);
            if (FW_FALSE == result) break;

            DBG("   - Allocated successfuly\r\n");
          }
          else
          {
            DBG("   - Not allocated!\r\n");
          }
        }
        else
        {
          DBG("   - Not enqueued!\r\n");
        }
      }
    }
  }
  while (0);

  alloc_size = BlockQueue_GetCountOfAllocated(pQueue);
  result &= ((EAST_QUEUE_FULL_COUNT - 1) == alloc_size);

  alloc_size = BlockQueue_GetCountOfFree(pQueue);
  result &= (0 == alloc_size);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ProcessTheQueue(void)
{
  FW_BOOLEAN result = FW_FALSE;
  FW_RESULT status = FW_ERROR;
  U8 * buffer = NULL, * alloc_buffer = NULL;
  U32 size = 0, i = 0, alloc_size = 0;

  DBG("*** EAST Block Success Processing Queue Test ***\r\n");

  /* Dequeue the block */
  DBG(" - Dequeue\r\n");
  status = BlockQueue_Dequeue(pQueue, &buffer, &size);
  result = (FW_BOOLEAN)(FW_SUCCESS == status);
  result &= (FW_BOOLEAN)(NULL != buffer);
  result &= (FW_BOOLEAN)(EAST_DATA_SIZE == size);

  /* Release the block */
  DBG(" - Release\r\n");
  status = BlockQueue_Release(pQueue);
  result &= (FW_BOOLEAN)(FW_SUCCESS == status);

  for (i = 0; i < sizeof(gTestEast); i++)
  {
    /* Fill the EAST packet */
    status = EAST_PutByte(pEAST, gTestEast[i % sizeof(gTestEast)]);
    if (FW_COMPLETE == status)
    {
      result &= (FW_BOOLEAN)(buffer[0] == gTestEast[8]);
      result &= (FW_BOOLEAN)(buffer[2] == gTestEast[10]);
      result &= (FW_BOOLEAN)(buffer[4] == gTestEast[12]);
      if (FW_FALSE == result) break;
      DBG(" - EAST Message Collected Successfuly\r\n");

      DBG(" - Enqueue the block\r\n");
      status = BlockQueue_Enqueue(pQueue, EAST_GetDataSize(pEAST));

      if (FW_SUCCESS == status)
      {
        DBG(" - Allocate the new Block from Queue\r\n");
        status = BlockQueue_Allocate(pQueue, &alloc_buffer, &alloc_size);
        if (FW_SUCCESS == status)
        {
          result &= (FW_BOOLEAN)(NULL != alloc_buffer);
          result &= (FW_BOOLEAN)(0 != alloc_size);

          buffer = alloc_buffer;
          size = alloc_size;

          status = EAST_SetBuffer(pEAST, buffer, size);
          result = (FW_BOOLEAN)(FW_SUCCESS == status);
          if (FW_FALSE == result) break;

          DBG("   - Allocated successfuly\r\n");
        }
        else
        {
          DBG("   - Not allocated!\r\n");
        }
      }
      else
      {
        DBG("   - Not enqueued!\r\n");
      }
    }
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
    Test_InitSuccess,
    Test_FillTheQueue,
    Test_ProcessTheQueue,
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
