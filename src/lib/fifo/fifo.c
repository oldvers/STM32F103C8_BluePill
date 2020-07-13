#include "fifo.h"
//#include "interrupts.h"
//#include "gpio.h"

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
/** @brief Puts Byte To The FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @param pByte - Pointer to the container for Byte
 *  @return FW_FULL / FW_SUCCESS
 */

FW_RESULT FIFO_Put(FIFO_p pFIFO, U8 * pByte)
{
//  IRQ_SAFE_AREA();
//  GPIO_Hi(GPIOB, 3);

  if (pFIFO->I == ((pFIFO->O - 1 + pFIFO->S) % pFIFO->S))
  {
    return FW_FULL;
  }

//  IRQ_DISABLE();
  
  pFIFO->B[pFIFO->I] = *pByte;

  pFIFO->I = (pFIFO->I + 1) % pFIFO->S;
  
//  IRQ_RESTORE();
//  GPIO_Lo(GPIOB, 3);
  return FW_SUCCESS;
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte from the FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @param pByte - Pointer to the container for Byte
 *  @return FW_EMPTY / FW_SUCCESS
 */

FW_RESULT FIFO_Get(FIFO_p pFIFO, U8 * pByte)
{
//  IRQ_SAFE_AREA();
//  GPIO_Hi(GPIOB, 3);

  if (pFIFO->I == pFIFO->O)
  {
    return FW_EMPTY;
  }

//  IRQ_DISABLE();

  *pByte = pFIFO->B[pFIFO->O];

  pFIFO->O = (pFIFO->O + 1) % pFIFO->S;

//  IRQ_RESTORE();
//  GPIO_Lo(GPIOB, 3);

  return FW_SUCCESS;
}

//-----------------------------------------------------------------------------
/** @brief Returns free space in the FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @return Free space in the FIFO
 */

U32 FIFO_Free(FIFO_p pFIFO)
{
  return (pFIFO->O - pFIFO->I - 1 + pFIFO->S) % pFIFO->S;
}

//-----------------------------------------------------------------------------
/** @brief Returns size of the FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @return Size of the FIFO
 */

U32 FIFO_Size(FIFO_p pFIFO)
{
  return (pFIFO->S - 1);
}

//-----------------------------------------------------------------------------
/** @brief Returns count of Bytes in the FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @return Count of Bytes in the FIFO
 */

U32 FIFO_Count(FIFO_p pFIFO)
{
  return (pFIFO->I - pFIFO->O + pFIFO->S) % pFIFO->S;
}

//-----------------------------------------------------------------------------
/** @brief Initializes the FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @param pBuffer - Pointer to the FIFO buffer
 *  @param aSize - Size of the FIFO buffer
 *  @return None
 */

void FIFO_Init(FIFO_p pFIFO, U8 * pBuffer, U32 aSize)
{
  pFIFO->I = 0;
  pFIFO->O = 0;
  pFIFO->S = aSize;
  pFIFO->B = pBuffer;
  for(U32 i = 0; i < aSize; i++) pFIFO->B[i] = 0;
}

//-----------------------------------------------------------------------------
/** @brief Clear the FIFO
 *  @param pFIFO - Pointer to the FIFO context
 *  @return None
 */

void FIFO_Clear(FIFO_p pFIFO)
{
//  IRQ_SAFE_AREA();

//  IRQ_DISABLE();

  pFIFO->I = 0;
  pFIFO->O = 0;
  for(U32 i = 0; i < pFIFO->S; i++) pFIFO->B[i] = 0;
  
//  IRQ_RESTORE();
}
