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
///** @brief Returns free space in the FIFO
// *  @param pFIFO - Pointer to the FIFO context
// *  @return Free space in the FIFO
// */
//
//U32 FIFO_Free(FIFO_p pFIFO)
//{
//  return (pFIFO->O - pFIFO->I - 1 + pFIFO->S) % pFIFO->S;
//}

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
    U8 *          Produced;          /* Allocated Item */
    U8 *          Consumed;          /* Item that should be released */
    /* OS specific fields */
    QueueHandle_t osQueue;           /* OS Queue handle */
    TickType_t    osTimeout;         /* OS queue receive timeout */
} BlockQueue_t;

//-----------------------------------------------------------------------------
/** @brief Wrapper for RTOS queue create function
 *  @param[in] aCapacity - Count of the Items in the Queue
 *  @param[in] aItemSize - The size of the Item
 *  @return Pointer to the created Queue or NULL
 */

static QueueHandle_t osal_QueueCreate(U32 aCapacity, U32 aItemSize)
{
    UBaseType_t available = 0;
    QueueHandle_t result = NULL;

    result = xQueueCreate(aCapacity, (unsigned portBASE_TYPE)aItemSize);

    available = uxQueueSpacesAvailable(result);
    QUEUE_LOG("  OS Queue Free  = %d\r\n", available);

    return result;
}

//-----------------------------------------------------------------------------
/** @brief Wrapper for RTOS queue reset function
 *  @param[in] pQueue - Pointer to the Block Queue
 *  @return None
 */

static void osal_QueueReset(BlockQueue_p pQueue)
{
    UBaseType_t available = 0;

    available = uxQueueSpacesAvailable(pQueue->osQueue);
    QUEUE_LOG("  OS Queue Free  = %d\r\n", available);

    QUEUE_LOG("  OS Queue Reset\r\n");

    xQueueReset(pQueue->osQueue);
    available = uxQueueSpacesAvailable(pQueue->osQueue);

    QUEUE_LOG("  OS Queue Free  = %d\r\n", available);
}

//-----------------------------------------------------------------------------
/** @brief Wrapper for RTOS queue put function
 *  @param[in] pQueue - Pointer to the Block Queue
 *  @param[in] pBlock - Pointer to the allocated block
 *  @param[in] aBlockSize - Size of the Block
 *  @return FW_TRUE if no error
 */

static FW_BOOLEAN osal_QueuePut(BlockQueue_p pQueue, U8* pBlock, U32 aBlockSize)
{
    BaseType_t status = pdFAIL;
    BlockItem_t item = {0};
    UBaseType_t available = 0;

    available = uxQueueSpacesAvailable(pQueue->osQueue);

    QUEUE_LOG("  OS Queue Free  = %d\r\n", available);

    /* Fill the item fields */
    item.Data = pBlock;
    item.Size = aBlockSize;

    /* Put the item to the queue */
    status = xQueueSendToBack(pQueue->osQueue, (void *)&item, (TickType_t)0);

    available = uxQueueSpacesAvailable(pQueue->osQueue);

    QUEUE_LOG("  OS Queue Free  = %d\r\n", available);

    if (pdPASS != status)
    {
        return FW_FALSE;
    }

    return FW_TRUE;
}

//-----------------------------------------------------------------------------
/** @brief Wrapper for RTOS queue get function
 *  @param[in] pQueue - Pointer to the Block Queue
 *  @param[out] ppBlock - Pointer to the block
 *  @param[out] pSize - Size of the Block
 *  @return FW_TRUE if no error, FW_FALSE - in case of timeout or error
 */

static FW_BOOLEAN osal_QueueGet(BlockQueue_p pQueue, U8** ppBlock, U32 * pSize)
{
    BaseType_t status = pdFAIL;
    BlockItem_t item = {0};
    UBaseType_t available = 0;

    available = uxQueueSpacesAvailable(pQueue->osQueue);
    QUEUE_LOG("  OS Queue Free  = %d\r\n", available);

    /* Get the item from the queue */
    status = xQueueReceive
             (
                 pQueue->osQueue,
                 (void *)&item,
                 (TickType_t)pQueue->osTimeout
             );

    available = uxQueueSpacesAvailable(pQueue->osQueue);
    QUEUE_LOG("  OS Queue Free  = %d\r\n", available);

    if (pdTRUE != status)
    {
        *ppBlock = NULL;
        *pSize = 0;
        return FW_FALSE;
    }

    /* Fill the item fields */
    *ppBlock = item.Data;
    *pSize = item.Size;

    return FW_TRUE;
}

//-----------------------------------------------------------------------------
/** @brief Initializes the Block Queue
 *  @param[in] pBuffer - Memory container for the Queue
 *  @param[in] aBufferSize - Memory container size
 *  @param[in] aBlockSize - Size of the queue Item
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
    pQueue->osTimeout = portMAX_DELAY;

    QUEUE_LOG("  pQueue         = %08X\r\n", pQueue);
    QUEUE_LOG("  osQueue        = %08X\r\n", pQueue->osQueue);
    QUEUE_LOG("  Block Count    = %d\r\n", blockCount);
    QUEUE_LOG("  Used Size      = %d\r\n",
                   ((blockAddress - queueAddress) + blockCount * aBlockSize));

    /* Initialize the Block Queue */
    pQueue->I = 0;
    pQueue->O = 0;
    pQueue->Capacity = blockCount;
    pQueue->BlocksBuffer = (U8 *)blockAddress;
    pQueue->BlockSize = aBlockSize;
    pQueue->Produced = NULL;
    pQueue->Consumed = NULL;

    /* Clear the Block buffer */
    for(i = 0; i < blockCount * aBlockSize; i++)
    {
        ((U8 *)blockAddress)[i] = 0;
    }

    return pQueue;
}

//-----------------------------------------------------------------------------
/** @brief Resets the queue
 *  @param[in] pQueue - Pointer to the Block Queue
 *  @return None
 *  @note Should be called carefully due to it isn't thread safe
 */

void BlockQueue_Reset(BlockQueue_p pQueue)
{
    QUEUE_LOG("- BlockQueue_Reset -\r\n");
    QUEUE_LOG("--- State\r\n");
    QUEUE_LOG("  I Position     = %d\r\n", pQueue->I);
    QUEUE_LOG("  O Position     = %d\r\n", pQueue->O);
    QUEUE_LOG("  Produced       = %08X\r\n", pQueue->Produced);
    QUEUE_LOG("  Consumed       = %08X\r\n", pQueue->Consumed);
    QUEUE_LOG("--- Internals\r\n");

    /* Reset the RTOS queue */
    osal_QueueReset(pQueue);

    /* Reset the Block Queue */
    pQueue->I = 0;
    pQueue->O = 0;
    pQueue->Produced = NULL;
    pQueue->Consumed = NULL;

    QUEUE_LOG("  I Position     = %d\r\n", pQueue->I);
    QUEUE_LOG("  O Position     = %d\r\n", pQueue->O);
    QUEUE_LOG("  Produced       = %08X\r\n", pQueue->Produced);
    QUEUE_LOG("  Consumed       = %08X\r\n", pQueue->Consumed);
}

//-----------------------------------------------------------------------------
/** @brief Returns the capacity of Block Queue
 *  @param pQueue - Pointer to the Block Queue
 *  @return Capacity of the queue
 */

U32 BlockQueue_GetCapacity(BlockQueue_p pQueue)
{
    return (pQueue->Capacity - 1);
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
/** @brief Allocates the block from the Queue
 *  @param[in]  pQueue - Pointer to the used Queue
 *  @param[out] ppBlock - Pointer to the allocated Block
 *  @param[out] pSize - Size of the allocated block
 *  @return FW_SUCCESS - if no error happens
 *          FW_FULL - if no space available for allocation the new block
 *          FW_ERROR - if block is allocated but not enqueued yet
 */

FW_RESULT BlockQueue_Allocate(BlockQueue_p pQueue, U8** ppBlock, U32 * pSize)
{
    U8 * block = NULL;

    QUEUE_LOG("- BlockQueue_Allocate -\r\n");
    QUEUE_LOG("--- State\r\n");
    QUEUE_LOG("  I Position     = %d\r\n", pQueue->I);
    QUEUE_LOG("  Capacity       = %d\r\n", BlockQueue_GetCapacity(pQueue));
    QUEUE_LOG("  Produced       = %08X\r\n", pQueue->Produced);
    QUEUE_LOG("--- Internals\r\n");

    /* No space for allocation */
    if (pQueue->I == ((pQueue->O - 1 + pQueue->Capacity) % pQueue->Capacity))
    {
        *ppBlock = NULL;
        *pSize = 0;

        QUEUE_LOG("  No Space\r\n");
        QUEUE_LOG("  Block          = %08X\r\n", *ppBlock);
        QUEUE_LOG("  Block Size     = %d\r\n", *pSize);

        return FW_FULL;
    }

    /* Previously allocated block should be enqueued
       before allocation the next block */
    if (NULL != pQueue->Produced)
    {
        *ppBlock = pQueue->Produced;
        *pSize = pQueue->BlockSize;

        QUEUE_LOG("  Not Enqueued\r\n");
        QUEUE_LOG("  Block          = %08X\r\n", *ppBlock);
        QUEUE_LOG("  Block Size     = %d\r\n", *pSize);

        return FW_ERROR;
    }

    /* Allocate the block */
    block = pQueue->BlocksBuffer + pQueue->I * pQueue->BlockSize;
    pQueue->Produced = block;
    *ppBlock = block;
    *pSize = pQueue->BlockSize;

    /* Move the input position to the next block */
    pQueue->I = (pQueue->I + 1) % pQueue->Capacity;

    QUEUE_LOG("  I Position     = %d\r\n", pQueue->I);
    QUEUE_LOG("  Capacity       = %d\r\n", BlockQueue_GetCapacity(pQueue));
    QUEUE_LOG("  Produced       = %08X\r\n", pQueue->Produced);
    QUEUE_LOG("  Block          = %08X\r\n", *ppBlock);
    QUEUE_LOG("  Block Size     = %d\r\n", *pSize);

    return FW_SUCCESS;
}

//-----------------------------------------------------------------------------
/** @brief Enqueues the allocated block with specified size
 *  @param[in] pQueue - Pointer to the Block Queue
 *  @param[in] aSize - Size of the block payload
 *  @return FW_SUCCESS - in no errors
 *          FW_ERROR - in case of block is not allocated or RTOS error
 */

FW_RESULT BlockQueue_Enqueue(BlockQueue_p pQueue, U32 aSize)
{
    FW_BOOLEAN status = FW_FALSE;

    QUEUE_LOG("- BlockQueue_Enqueue -\r\n");
    QUEUE_LOG("--- Input\r\n");
    QUEUE_LOG("  Size          = %d\r\n", aSize);
    QUEUE_LOG("--- Internals\r\n");
    QUEUE_LOG("  Produced       = %08X\r\n", pQueue->Produced);

    /* The block should be previously allocated before it can be enqueued */
    if (NULL == pQueue->Produced)
    {
        QUEUE_LOG("  Not Allocated\r\n");
        return FW_ERROR;
    }

    /* Block boundaries must be checked outside this function to avoid
       memory leaks. But still it's just the double-check */
    if (aSize > pQueue->BlockSize)
    {
        QUEUE_LOG("  Wrong Block Size!\r\n");
        return FW_ERROR;
    }

    /* Enqueue the block */
    status = osal_QueuePut(pQueue, pQueue->Produced, aSize);
    if (FW_FALSE == status)
    {
        QUEUE_LOG("  RTOS Queue Put error!\r\n");
        return FW_ERROR;
    }

    /* Indicate that the next block can be allocated */
    pQueue->Produced = NULL;

    QUEUE_LOG("  Produced       = %08X\r\n", pQueue->Produced);

    return FW_SUCCESS;
}

//-----------------------------------------------------------------------------
/** @brief Enqueues the block
 *  @param[in] pQueue - Pointer to the Block Queue
 *  @param[out] ppBlock - Pointer to the block
 *  @param[out] pSize - Size of the Block
 *  @return FW_SUCCESS if no error,
 *          FW_ERROR - in case of block is not released or block size is wrong
 *          FW_TIMEOUT - in case of timeout of waiting block
 */

FW_RESULT BlockQueue_Dequeue(BlockQueue_p pQueue, U8** ppBlock, U32 * pSize)
{
    FW_BOOLEAN status = FW_FALSE;

    QUEUE_LOG("- BlockQueue_Dequeue -\r\n");
    QUEUE_LOG("--- State\r\n");
    QUEUE_LOG("  O Position     = %d\r\n", pQueue->O);
    QUEUE_LOG("  Capacity       = %d\r\n", BlockQueue_GetCapacity(pQueue));
    QUEUE_LOG("  Consumed       = %08X\r\n", pQueue->Consumed);
    QUEUE_LOG("--- Internals\r\n");

    /* The block should be previously released before it can be dequeued */
    if (NULL != pQueue->Consumed)
    {
        QUEUE_LOG("  Not Released\r\n");
        return FW_ERROR;
    }

    /* Dequeue the block */
    status = osal_QueueGet(pQueue, ppBlock, pSize);
    if (FW_FALSE == status)
    {
        QUEUE_LOG("  RTOS Queue Get timeout or error!\r\n");
        return FW_TIMEOUT;
    }

    /* Sanity check */
    if (*pSize > pQueue->BlockSize)
    {
        QUEUE_LOG("  Wrong Block Size!\r\n");
        return FW_ERROR;
    }

    QUEUE_LOG("  Size           = %d\r\n", *pSize);

    /* Indicate that the consumed block should be released */
    pQueue->Consumed = *ppBlock;

    QUEUE_LOG("  Consumed       = %08X\r\n", pQueue->Consumed);

    return FW_SUCCESS;
}

//-----------------------------------------------------------------------------
/** @brief Releases the block to the Queue
 *  @param[in]  pQueue - Pointer to the used Queue
 *  @return FW_SUCCESS - if block has been released successfuly
 *          FW_ERROR - if block has been already released
 *          FW_EMPTY - if there is no block to release
 */

FW_RESULT BlockQueue_Release(BlockQueue_p pQueue)
{
    QUEUE_LOG("- BlockQueue_Release -\r\n");
    QUEUE_LOG("--- State\r\n");
    QUEUE_LOG("  O Position     = %d\r\n", pQueue->O);
    QUEUE_LOG("  Capacity       = %d\r\n", BlockQueue_GetCapacity(pQueue));
    QUEUE_LOG("  Consumed       = %08X\r\n", pQueue->Consumed);
    QUEUE_LOG("--- Internals\r\n");

    /* Previously dequeued block has been already released */
    if (NULL == pQueue->Consumed)
    {
        QUEUE_LOG("  Nothing to release\r\n");

        return FW_ERROR;
    }

    /* The queue is already empty so there is no block can be released.
       It should be impossible case */
    if (pQueue->I == pQueue->O)
    {
        pQueue->Consumed = NULL;

        QUEUE_LOG("  Empty\r\n");
        QUEUE_LOG("  Consumed       = %08X\r\n", pQueue->Consumed);

        return FW_EMPTY;
    }

    /* Release the block */
    pQueue->Consumed = NULL;

    /* Move the output position to the next block */
    pQueue->O = (pQueue->O + 1) % pQueue->Capacity;

    QUEUE_LOG("  O Position     = %d\r\n", pQueue->O);
    QUEUE_LOG("  Capacity       = %d\r\n", BlockQueue_GetCapacity(pQueue));
    QUEUE_LOG("  Consumed       = %08X\r\n", pQueue->Consumed);

    return FW_SUCCESS;
}