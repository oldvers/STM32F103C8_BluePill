#ifndef __EAST_H__
#define __EAST_H__

#include "types.h"

//typedef void (*EAST_Completed)(void);

//typedef __packed struct _EAST_STATE
//{
//  U16             MaxSize;
//  U16             ActSize;
//  U16             Index;
//  U8              CS;
//  U8              OK;
//  U8            * Buffer;
//  EAST_Completed  OnComplete;
//} EAST_STATE;

typedef struct EAST_s * EAST_p;

EAST_p    EAST_Init(U8 * pContainer, U32 aSize, U8 * pBuffer, U32 aBufferSize);
FW_RESULT   EAST_SetBuffer   (EAST_p pEAST, U8 * pBuffer, U32 aSize);
FW_BOOLEAN  EAST_IsCompleted (EAST_p pEAST);
FW_RESULT   EAST_PutByte     (EAST_p pEAST, U8 aValue);
FW_RESULT   EAST_GetByte     (EAST_p pEAST, U8 * pValue);

#endif /* __EAST_H__ */
