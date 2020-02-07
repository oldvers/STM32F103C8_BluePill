#ifndef __ICEMKII_MESSAGE_H__
#define __ICEMKII_MESSAGE_H__

#include "types.h"

typedef struct ICEMKII_MESSAGE_STRUCT
{
  U32             MaxSize;
  U32             ActSize;
  U16             Index;
  U16             RCRC;
  U16             CCRC;
  U16             SeqNumber;
  U16             LastError;
  FW_BOOLEAN      OK;
  U8            * Buffer;
} ICEMKII_MESSAGE;

FW_RESULT ICEMKII_MESSAGE_PutByte(ICEMKII_MESSAGE * pMsg, U8   aValue);
FW_RESULT ICEMKII_MESSAGE_GetByte(ICEMKII_MESSAGE * pMsg, U8 * pValue);
void      ICEMKII_MESSAGE_Init(ICEMKII_MESSAGE * pMsg, U8 *pBuffer, U32 aSize);

#endif /* __ICEMKII_MESSAGE_H__ */
