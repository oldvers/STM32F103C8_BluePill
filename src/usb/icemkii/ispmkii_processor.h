#ifndef __ISPMKII_PROCESSOR_H__
#define __ISPMKII_PROCESSOR_H__

#include "types.h"

FW_BOOLEAN ISPMKII_Process(U8 * pReqBody, U8 * pRspBody, U32 * pSize);
void       ISPMKII_Reset(void);

#endif /* __ISPMKII_PROCESSOR_H__ */