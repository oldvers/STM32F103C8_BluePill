#ifndef __ICEMKII_MESSAGE_H__
#define __ICEMKII_MESSAGE_H__

#include "types.h"

#define ICEMKII_MSG_HOLDER_SIZE  (24)

typedef struct ICEMKII_MSG_s * ICEMKII_MSG_p;

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
FW_RESULT ICEMKII_MSG_PutByte          (ICEMKII_MSG_p pMsg, U8   aValue);
FW_RESULT ICEMKII_MSG_GetByte          (ICEMKII_MSG_p pMsg, U8 * pValue);
void      ICEMKII_MSG_SetSequenceNumber(ICEMKII_MSG_p pMsg, U16 aValue);
U16       ICEMKII_MSG_GetDataSize      (ICEMKII_MSG_p pMsg);
U16       ICEMKII_MSG_GetPacketSize    (ICEMKII_MSG_p pMsg);

#endif /* __ICEMKII_MESSAGE_H__ */
