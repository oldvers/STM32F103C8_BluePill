#ifndef __EAST_H__
#define __EAST_H__

#include "types.h"

#define EAST_START_MARKER              (0x24)
#define EAST_STOP_MARKER               (0x42)

typedef void (*EAST_Completed)(void);

typedef __packed struct _EAST_STATE
{
  U16             MaxSize;
  U16             ActSize;
  U16             Index;
  U16             CS;
  U8              OK;
  U8            * Buffer;
  EAST_Completed  OnComplete;
} EAST_STATE;

void EAST_PutByte(EAST_STATE * pState, U8 aValue);
U32 EAST_GetByte(EAST_STATE * pState, U8 * pValue);

#endif /* __EAST_H__ */
