#ifndef __ICEMKII_PROCESSOR_H__
#define __ICEMKII_PROCESSOR_H__

#include "types.h"

void       ICEMKII_Process        (U8 * pReqBody, U8 * pRspBody, U32 * pSize);
FW_BOOLEAN ICEMKII_CheckForEvents (U8 * pReqBody, U8 * pRspBody, U32 * pSize);

#endif /* __ICEMKII_PROCESSOR_H__ */