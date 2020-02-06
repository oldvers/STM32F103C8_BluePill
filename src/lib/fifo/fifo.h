#ifndef __FIFO_H__
#define __FIFO_H__

#include "types.h"

/* FIFO Description Structure */
typedef struct FIFO_s
{
  S32  I;            /* Input position */
  S32  O;            /* Output position */
  U32  S;            /* Size of FIFO */
  U8 * B;            /* FIFO buffer */
} FIFO_t, * FIFO_p;

void       FIFO_Init     (FIFO_p pFIFO, U8 * pBuffer, U32 aSize);
FW_RESULT  FIFO_Put      (FIFO_p pFIFO, U8 * pByte);
FW_RESULT  FIFO_Get      (FIFO_p pFIFO, U8 * pByte);
U32        FIFO_Free     (FIFO_p pFIFO);
U32        FIFO_Capacity (FIFO_p pFIFO);
U32        FIFO_Size     (FIFO_p pFIFO);

#endif /* __FIFO_H__ */
