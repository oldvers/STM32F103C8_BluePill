#ifndef __VCP_H__
#define __VCP_H__

#include "types.h"

U32  VCP_Open(void);
void VCP_Close(void);
U32  VCP_Read(U8 * pData, U32 aSize, U32 aTimeout);
U32  VCP_Write(U8 * pData, U32 aSize, U32 aTimeout);

#endif /* __VCP_H__ */
