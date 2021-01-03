#include "blockqueue.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "debug.h"

/* Very simple queue
 * These are FIFO queues which discard the new data when full.
 *
 * https://stackoverflow.com/questions/215557/
 *                         how-do-i-implement-a-circular-list-ring-buffer-in-c
 *
 * Queue is empty when In == Out.
 * If In != Out, then
 *  - items are placed into In before incrementing In
 *  - items are removed from Out before incrementing Out
 * Queue is full when In == (Out - 1 + MAX_QUEUE_SIZE) % MAX_QUEUE_SIZE;
 *
 * The queue will hold (Size - 1) number of items before the
 * calls to Queue_Put fail.
 */

//-----------------------------------------------------------------------------

#define QUEUE_DEBUG

#ifdef QUEUE_DEBUG
#  define QUEUE_LOG           LOG
#else
#  define QUEUE_LOG(...)
#endif

////-----------------------------------------------------------------------------
///** @brief Puts Byte To The FIFO
// *  @param pFIFO - Pointer to the FIFO context
// *  @param pByte - Pointer to the container for Byte
// *  @return FW_FULL / FW_SUCCESS
// */
//
//FW_RESULT FIFO_Put(FIFO_p pFIFO, U8 * pByte)
//{
////  IRQ_SAFE_AREA();
////  GPIO_Hi(GPIOB, 3);
//
//  if (pFIFO->I == ((pFIFO->O - 1 + pFIFO->S) % pFIFO->S))
//  {
//    return FW_FULL;
//  }
//
////  IRQ_DISABLE();
//
//  pFIFO->B[pFIFO->I] = *pByte;
//
//  pFIFO->I = (pFIFO->I + 1) % pFIFO->S;
//
////  IRQ_RESTORE();
////  GPIO_Lo(GPIOB, 3);
//  return FW_SUCCESS;
//}
//
////-----------------------------------------------------------------------------
///** @brief Gets Byte from the FIFO
// *  @param pFIFO - Pointer to the FIFO context
// *  @param pByte - Pointer to the container for Byte
// *  @return FW_EMPTY / FW_SUCCESS
// */
//
//FW_RESULT FIFO_Get(FIFO_p pFIFO, U8 * pByte)
//{
////  IRQ_SAFE_AREA();
////  GPIO_Hi(GPIOB, 3);
//
//  if (pFIFO->I == pFIFO->O)
//  {
//    return FW_EMPTY;
//  }
//
////  IRQ_DISABLE();
//
//  *pByte = pFIFO->B[pFIFO->O];
//
//  pFIFO->O = (pFIFO->O + 1) % pFIFO->S;
//
////  IRQ_RESTORE();
////  GPIO_Lo(GPIOB, 3);
//
//  return FW_SUCCESS;
//}
//
////-----------------------------------------------------------------------------
///** @brief Returns free space in the FIFO
// *  @param pFIFO - Pointer to the FIFO context
// *  @return Free space in the FIFO
// */
//
//U32 FIFO_Free(FIFO_p pFIFO)
//{
//  return (pFIFO->O - pFIFO->I - 1 + pFIFO->S) % pFIFO->S;
//}
//
////-----------------------------------------------------------------------------
///** @brief Returns size of the FIFO
// *  @param pFIFO - Pointer to the FIFO context
// *  @return Size of the FIFO
// */
//
//U32 FIFO_Size(FIFO_p pFIFO)
//{
//  return (pFIFO->S - 1);
//}
//
////-----------------------------------------------------------------------------
///** @brief Returns count of Bytes in the FIFO
// *  @param pFIFO - Pointer to the FIFO context
// *  @return Count of Bytes in the FIFO
// */
//
//U32 FIFO_Count(FIFO_p pFIFO)
//{
//  return (pFIFO->I - pFIFO->O + pFIFO->S) % pFIFO->S;
//}
//
////-----------------------------------------------------------------------------
///** @brief Initializes the FIFO
// *  @param pFIFO - Pointer to the FIFO context
// *  @param pBuffer - Pointer to the FIFO buffer
// *  @param aSize - Size of the FIFO buffer
// *  @return None
// */
//
//void FIFO_Init(FIFO_p pFIFO, U8 * pBuffer, U32 aSize)
//{
//  pFIFO->I = 0;
//  pFIFO->O = 0;
//  pFIFO->S = aSize;
//  pFIFO->B = pBuffer;
//  for(U32 i = 0; i < aSize; i++) pFIFO->B[i] = 0;
//}
//
////-----------------------------------------------------------------------------
///** @brief Clear the FIFO
// *  @param pFIFO - Pointer to the FIFO context
// *  @return None
// */
//
//void FIFO_Clear(FIFO_p pFIFO)
//{
////  IRQ_SAFE_AREA();
//
////  IRQ_DISABLE();
//
//  pFIFO->I = 0;
//  pFIFO->O = 0;
//  for(U32 i = 0; i < pFIFO->S; i++) pFIFO->B[i] = 0;
//
////  IRQ_RESTORE();
//}

//static QueueHandle_t xPolledQueue;
//const unsigned portBASE_TYPE uxQueueSize = 10;

/* Create the queue used by the producer and consumer. */
//xPolledQueue = xQueueCreate( uxQueueSize, ( unsigned portBASE_TYPE ) sizeof( unsigned short ) );
//producer, the other a
//consumer.

typedef struct BlockItem_s
{
    U32  Size;    /* Size of the Block */
    U8 * Data;    /* Pointer to the Block payload */
} BlockItem_t;

typedef struct BlockQueue_s
{
    S32           I;                 /* Input position in the Queue */
    S32           O;                 /* Output position in the Queue */
    U32           Capacity;          /* Max count of Items in the Queue */
    U32           BlockSize;         /* Size of the Item */
    U8 *          BlocksBuffer;      /* Pointer to the Queue Buffer */
    U8 *          CurrentProducer;   /* Allocated Item */
    U8 *          CurrentConsumer;   /* Item that should be released */
    QueueHandle_t osQueue;
} BlockQueue_t;

//-----------------------------------------------------------------------------
/** @brief Wrapper for RTOS queue create function
 *  @param aCapacity - Count of the Items in the Queue
 *  @param aItemSize - The size of the Item
 *  @return Pointer to the created Queue or NULL
 */

static QueueHandle_t osal_QueueCreate(U32 aCapacity, U32 aItemSize)
{
    QueueHandle_t result = NULL;

    result = xQueueCreate(aCapacity, (unsigned portBASE_TYPE)aItemSize);

    return result;
}

//-----------------------------------------------------------------------------
/** @brief Initializes the Block Queue
 *  @param pBuffer - Memory container for the Queue
 *  @param aBufferSize - Memory container size
 *  @param aBlockSize - Size of the queue Item
 *  @return Pointer to the Block Queue structure or NULL in case of error
 */

BlockQueue_p BlockQueue_Init(U8 * pBuffer, U32 aBufferSize, U32 aBlockSize)
{
    BlockQueue_p pQueue       = NULL;
    U32          queueAddress = 0;
    U32          blockAddress = 0;
    U32          blockCount   = 0;
    U32          i            = 0;

    /* Calculate the block size with alignment equal to 4 bytes */
    aBlockSize = (aBlockSize + 3) / 4 * 4;

    /* Calculate the queue structure address, aligned to 4 bytes */
    queueAddress = ((U32)pBuffer + 3) & 0xFFFFFFFC;

    /* Calculate the first block address, aligned to 4 bytes */
    blockAddress = queueAddress + ((sizeof(BlockQueue_t) + 3) / 4 * 4);

    QUEUE_LOG("- BlockQueue_Init -\r\n");
    QUEUE_LOG("--- Inputs\r\n");
    QUEUE_LOG("  Buffer Address = %08X\r\n", pBuffer);
    QUEUE_LOG("  Buffer Size    = %d\r\n", aBufferSize);
    QUEUE_LOG("  Block Size     = %d\r\n", aBlockSize);
    QUEUE_LOG("--- Internals\r\n");
    QUEUE_LOG("  Queue Address  = %08X\r\n", queueAddress);
    QUEUE_LOG("  Block Address  = %08X\r\n", blockAddress);
    QUEUE_LOG("  Block Size     = %d\r\n", aBlockSize);
    QUEUE_LOG("  Queue Str Size = %d\r\n", sizeof(BlockQueue_t));

    /* At least two elements should be fit into the buffer */
    if ( (blockAddress + 2 * aBlockSize) > ((U32)pBuffer + aBufferSize) )
    {
        return NULL;
    }

    /* Allocate the Queue structure */
    pQueue = (BlockQueue_p)queueAddress;

    /* Calculate the capacity of the queue */
    blockCount = ((U32)pBuffer + aBufferSize - blockAddress) / aBlockSize;

    /* Initialize the RTOS queue */
    pQueue->osQueue = osal_QueueCreate(blockCount - 1, sizeof(BlockItem_t));
    if (NULL == pQueue->osQueue)
    {
        return NULL;
    }

    QUEUE_LOG("  pQueue         = %08X\r\n", pQueue);
    QUEUE_LOG("  osQueue        = %08X\r\n", pQueue->osQueue);
    QUEUE_LOG("  Block Count    = %d\r\n", blockCount);

    /* Initialize the Block Queue */
    pQueue->I = 0;
    pQueue->O = 0;
    pQueue->Capacity = blockCount;
    pQueue->BlocksBuffer = (U8 *)blockAddress;
    pQueue->BlockSize = aBlockSize;
    pQueue->CurrentProducer = NULL;
    pQueue->CurrentConsumer = NULL;

    /* Clear the Block buffer */
    for(i = 0; i < blockCount * aBlockSize; i++)
    {
        ((U8 *)blockAddress)[i] = 0;
    }

    return pQueue;
}

//-----------------------------------------------------------------------------
/** @brief
 *  @param
 *  @return
 */

void BlockQueue_Clear(BlockQueue_p pQueue)
{
    //
}

//-----------------------------------------------------------------------------
/** @brief
 *  @param
 *  @return
 */

U32 BlockQueue_GetCapacity(BlockQueue_p pQueue)
{
    return 0;
}

//-----------------------------------------------------------------------------
/** @brief
 *  @param
 *  @return
 */

U32 BlockQueue_GetCountOfAllocated(BlockQueue_p pQueue)
{
    return 0;
}

//-----------------------------------------------------------------------------
/** @brief
 *  @param
 *  @return
 */

U32 BlockQueue_GetCountOfFree(BlockQueue_p pQueue)
{
    return 0;
}

//-----------------------------------------------------------------------------
/** @brief
 *  @param
 *  @return
 */

FW_RESULT BlockQueue_Allocate(BlockQueue_p pQueue, U8 * pBlock, U32 * pSize)
{
    return FW_ERROR;
}

//-----------------------------------------------------------------------------
/** @brief
 *  @param
 *  @return
 */

FW_RESULT BlockQueue_Enqueue(BlockQueue_p pQueue)
{
    return FW_ERROR;
}

//-----------------------------------------------------------------------------
/** @brief
 *  @param
 *  @return
 */

FW_RESULT BlockQueue_Dequeue(BlockQueue_p pQueue, U8 * pBlock, U32 * pSize)
{
    return FW_ERROR;
}

//-----------------------------------------------------------------------------
/** @brief
 *  @param
 *  @return
 */

FW_RESULT BlockQueue_Free(BlockQueue_p pQueue)
{
    return FW_ERROR;
}