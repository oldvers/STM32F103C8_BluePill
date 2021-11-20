#ifndef __ICEMKII_MESSAGE_H__
#define __ICEMKII_MESSAGE_H__

#include "types.h"

//#define ICEMKII_MESSAGE_HOLDER_SIZE    (24)

//typedef struct EAST_s * EAST_p;
typedef struct ICEMKII_MSG_s * ICEMKII_MSG_p;


//typedef struct ICEMKII_MESSAGE_STRUCT
//{
//  U32             MaxSize;
//  U32             ActSize;
//  U16             Index;
//  U16             RCRC;
//  U16             CCRC;
//  U16             SeqNumber;
//  U16             LastError;
//  FW_BOOLEAN      OK;
//  U8            * Buffer;
//} ICEMKII_MESSAGE;

ICEMKII_MSG_p ICEMKII_MSG_Init
              (
                  U8 * pHolder,
                  U32 aSize,
                  U8 * pBuffer,
                  U32 aBufferSize
              );
FW_RESULT ICEMKII_MSG_SetBuffer
          (
              ICEMKII_MSG_p pMsg,
              U8 * pBuffer,
              U32 aSize
          );
FW_RESULT ICEMKII_MSG_PutByte(ICEMKII_MSG_p pMsg, U8   aValue);
FW_RESULT ICEMKII_MSG_GetByte(ICEMKII_MSG_p pMsg, U8 * pValue);

#endif /* __ICEMKII_MESSAGE_H__ */
