#include "fifo.h"

/****************************************************************************************/
// Type Definitions
/****************************************************************************************/

/* Very simple queue
 * These are FIFO queues which discard the new data when full.
 *
 * https://stackoverflow.com/questions/215557/how-do-i-implement-a-circular-list-ring-buffer-in-c
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

/****************************************************************************************/
//*** Event Queue functions ***
/****************************************************************************************/

/****************************************************************************************/
// Event Queue Initialization
/****************************************************************************************/

void FIFO_Init(FIFO_p pFIFO, U8 * pBuffer, U32 aSize)
{
  /* Initialize FIFO */
  pFIFO->I = 0;
  pFIFO->O = 0;
  pFIFO->S = aSize;
  pFIFO->B = pBuffer;
  for(U8 i = 0; i < aSize; i++) pFIFO->B[i] = 0;
}

/****************************************************************************************/
// Put Event To Queue
/****************************************************************************************/

U32 FIFO_Put(FIFO_p pFIFO, U8 * pByte)
{
  if (pFIFO->I == ((pFIFO->O - 1 + pFIFO->S) % pFIFO->S))
  {
    return FIFO_IS_FULL;
  }

  pFIFO->B[pFIFO->I] = *pByte;

  pFIFO->I = (pFIFO->I + 1) % pFIFO->S;

  return FIFO_SUCCESS;
}

/****************************************************************************************/
//  Get Event from Queue
/****************************************************************************************/

U32 FIFO_Get(FIFO_p pFIFO, U8 * pByte)
{
  if (pFIFO->I == pFIFO->O)
  {
    return FIFO_IS_EMPTY;
  }

  *pByte = pFIFO->B[pFIFO->O];

  pFIFO->O = (pFIFO->O + 1) % pFIFO->S;

  return FIFO_SUCCESS;
}

/****************************************************************************************/
// Free space in Queue
/****************************************************************************************/

U32 FIFO_Free(FIFO_p pFIFO)
{
  return (pFIFO->O - pFIFO->I - 1 + pFIFO->S) % pFIFO->S;
}

/****************************************************************************************/
// Max count of items in Queue
/****************************************************************************************/

U32 FIFO_Capacity(FIFO_p pFIFO)
{
  return (pFIFO->S - 1);
}

/****************************************************************************************/
// Current count of items in Queue
/****************************************************************************************/

U32 FIFO_Size(FIFO_p pFIFO)
{
//Reset values after ED
LastSavedEventAddress = 0;
LastSendedEventAddress = 0;
}*/

U32 FIFO_Free(FIFO_p pFIFO)
{
  return (pFIFO->S + 1) - (pFIFO->I - pFIFO->O);
}

U32 FIFO_Size(FIFO_p pFIFO)
{
  return (pFIFO->S - 1);
}

//unsigned int fifo_len(fifo_t * fifo)
//{
//    // two usigned will not make this negative 
//    // even if in or out overflow as long as in is ahead of out
//    return fifo->in - fifo->out;
//}

//int fifo_is_empty(fifo_t * fifo)
//{
//     return fifo->in == fifo->out;
//  if (pFIFO->I == pFIFO->O)
//  {
//    return FIFO_IS_EMPTY;
//  }
//}

//int fifo_is_full(fifo_t * fifo)
//{
//     return fifo_len(fifo) > fifo->mask;
//   if (pFIFO->I == ((pFIFO->O - 1 + pFIFO->S) % pFIFO->S))
//  {
//    return FIFO_IS_FULL;
//  }
//}

//unsigned int fifo_free(fifo_t * fifo)
//{
//     return fifo_unused(fifo);
//}
