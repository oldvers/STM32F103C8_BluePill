#include "fifo.h"
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

#define FIFO_DEBUG

#ifdef FIFO_DEBUG
#  define FIFO_LOG    LOG
#else
#  define FIFO_LOG(...)
#endif

//-----------------------------------------------------------------------------

/* FIFO Description Structure */
typedef struct FIFO_s
{
    S32  I;   /* Input position in the FIFO */
    S32  O;   /* Output position in the FIFO */
    U32  S;   /* Size of the FIFO Buffer */
    U8 * B;   /* Pointer to the FIFO Buffer */
} FIFO_t;

//-----------------------------------------------------------------------------
/** @brief Puts Byte To The FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @param pByte - Pointer to the container for Byte
 *  @return FW_FULL / FW_SUCCESS / FW_ERROR
 */

FW_RESULT FIFO_Put(FIFO_p pFIFO, U8 * pByte)
{
    if ((NULL == pFIFO) || (NULL == pByte))
    {
        return FW_ERROR;
    }

    if (pFIFO->I == ((pFIFO->O - 1 + pFIFO->S) % pFIFO->S))
    {
        return FW_FULL;
    }

    pFIFO->B[pFIFO->I] = *pByte;

    pFIFO->I = (pFIFO->I + 1) % pFIFO->S;

    return FW_SUCCESS;
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte from the FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @param pByte - Pointer to the container for Byte
 *  @return FW_EMPTY / FW_SUCCESS / FW_ERROR
 */

FW_RESULT FIFO_Get(FIFO_p pFIFO, U8 * pByte)
{
    if ((NULL == pFIFO) || (NULL == pByte))
    {
        return FW_ERROR;
    }

    if (pFIFO->I == pFIFO->O)
    {
        return FW_EMPTY;
    }

    *pByte = pFIFO->B[pFIFO->O];

    pFIFO->O = (pFIFO->O + 1) % pFIFO->S;

    return FW_SUCCESS;
}

//-----------------------------------------------------------------------------
/** @brief Returns free space in the FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @return Free space in the FIFO
 */

U32 FIFO_Free(FIFO_p pFIFO)
{
    if (NULL != pFIFO)
    {
        return (pFIFO->O - pFIFO->I - 1 + pFIFO->S) % pFIFO->S;
    }
    return 0;
}

//-----------------------------------------------------------------------------
/** @brief Returns size of the FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @return Size of the FIFO
 */

U32 FIFO_Size(FIFO_p pFIFO)
{
    if (NULL != pFIFO)
    {
        return (pFIFO->S - 1);
    }
    return 0;
}

//-----------------------------------------------------------------------------
/** @brief Returns count of Bytes in the FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @return Count of Bytes in the FIFO
 */

U32 FIFO_Count(FIFO_p pFIFO)
{
    if (NULL != pFIFO)
    {
        return (pFIFO->I - pFIFO->O + pFIFO->S) % pFIFO->S;
    }
    return 0;
}

//-----------------------------------------------------------------------------
/** @brief Initializes the FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @param pBuffer - Pointer to the FIFO buffer
 *  @param aSize - Size of the FIFO buffer
 *  @return None
 */

FIFO_p FIFO_Init(U8 * pBuffer, U32 aSize)
{
    FIFO_p result = NULL;
    U32 fifoAddress = 0;
    U32 dataAddress = 0;
    U32 dataLength  = 0;

    FIFO_LOG("- FIFO_Init() -\r\n");
    FIFO_LOG("--- Inputs\r\n");
    FIFO_LOG("  Buffer Address = %08X\r\n", pBuffer);
    FIFO_LOG("  Buffer Size    = %d\r\n", aSize);
    FIFO_LOG("--- Internals\r\n");

    if (NULL == pBuffer)
    {
        FIFO_LOG("  Input parameters error!\r\n");
        return NULL;
    }

    /* Calculate the FIFO structure address, aligned to 4 bytes */
    fifoAddress = ((U32)pBuffer + 3) & 0xFFFFFFFC;

    /* Calculate the first data byte address */
    dataAddress = fifoAddress + sizeof(FIFO_t);

    /* Calculate the FIFO length */
    dataLength = (U32)pBuffer + aSize - dataAddress;

    FIFO_LOG("  FIFO Address   = %08X\r\n", fifoAddress);
    FIFO_LOG("  Data Address   = %08X\r\n", dataAddress);
    FIFO_LOG("  FIFO Length    = %d\r\n", dataLength);
    FIFO_LOG("  FIFO Str Size  = %d\r\n", sizeof(FIFO_t));

    /* At least two elements should be fit into the buffer */
    if ( (dataAddress + 2) > ((U32)pBuffer + aSize) )
    {
        return NULL;
    }

    /* Allocate the FIFO structure */
    result = (FIFO_p)fifoAddress;

    FIFO_LOG("  pFIFO          = %08X\r\n", result);
    FIFO_LOG("  Used Size      = %d\r\n",
                                  ((dataAddress - fifoAddress) + dataLength));

    result->I = 0;
    result->O = 0;
    result->S = dataLength;
    result->B = (U8 *)dataAddress;
    for(U32 i = 0; i < aSize; i++) result->B[i] = 0;

    return result;
}

//-----------------------------------------------------------------------------
/** @brief Clear the FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @return None
 *  @note Should be called carefully due to it isn't thread safe
 */

void FIFO_Clear(FIFO_p pFIFO)
{
    if (NULL == pFIFO) return;

    pFIFO->I = 0;
    pFIFO->O = 0;
    for(U32 i = 0; i < pFIFO->S; i++) pFIFO->B[i] = 0;
}
