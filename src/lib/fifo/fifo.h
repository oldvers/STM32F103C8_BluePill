#ifndef __FIFO_H__
#define __FIFO_H__

#include "types.h"

/* FIFO Description Structure */
typedef struct FIFO_s * FIFO_p;

FIFO_p     FIFO_Init  (U8 * pBuffer, U32 aSize);
FW_RESULT  FIFO_Put   (FIFO_p pFIFO, U8 * pByte);
FW_RESULT  FIFO_Get   (FIFO_p pFIFO, U8 * pByte);
U32        FIFO_Free  (FIFO_p pFIFO);
U32        FIFO_Count (FIFO_p pFIFO);
U32        FIFO_Size  (FIFO_p pFIFO);
void       FIFO_Clear (FIFO_p pFIFO);

#endif /* __FIFO_H__ */
