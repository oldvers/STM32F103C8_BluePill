#ifndef __ICEMKII_MESSAGE_H__
#define __ICEMKII_MESSAGE_H__

#include "types.h"

//typedef void (*EAST_Completed)(void);

typedef struct ICEMKII_MESSAGE_STRUCT
{
  U32             MaxSize;
  U32             ActSize;
  U16             Index;
  U16             CRC;
  U16             SeqNumber;
  U8              OK;
  U8            * Buffer;
  //EAST_Completed  OnComplete;
} ICEMKII_MESSAGE;

void ICEMKII_MESSAGE_PutByte(ICEMKII_MESSAGE * pMsg, U8 aValue);
U32 ICEMKII_MESSAGE_GetByte(ICEMKII_MESSAGE * pMsg, U8 * pValue);

#endif /* __ICEMKII_MESSAGE_H__ */
