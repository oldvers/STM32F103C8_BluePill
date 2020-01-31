#ifndef __ICEMKII_MESSAGE_H__
#define __ICEMKII_MESSAGE_H__

#include "types.h"

#define ICEMKII_MSG_SUCCESS            ( 0x00000000 )
#define ICEMKII_MSG_COMPLETE           ( 0xADE00000 )
#define ICEMKII_MSG_ERROR              ( 0xBAD00000 )

typedef struct ICEMKII_MESSAGE_STRUCT
{
  U32             MaxSize;
  U32             ActSize;
  U16             Index;
  U16             CRC16;
  U16             SeqNumber;
  U8              OK;
  U8            * Buffer;
} ICEMKII_MESSAGE;

U32 ICEMKII_MESSAGE_PutByte(ICEMKII_MESSAGE * pMsg, U8   aValue);
U32 ICEMKII_MESSAGE_GetByte(ICEMKII_MESSAGE * pMsg, U8 * pValue);

#endif /* __ICEMKII_MESSAGE_H__ */
