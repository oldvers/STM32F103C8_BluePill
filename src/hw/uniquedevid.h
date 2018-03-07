#ifndef __UNIQUEDEVID_H__
#define __UNIQUEDEVID_H__

#include "types.h"

#define FLASH_SIZE   (*((U16*)0x1FFFF7E0))

#define UDID_0       (*((U16*)(0x1FFFF7E8 + 0U)))
#define UDID_1       (*((U16*)(0x1FFFF7E8 + 2U)))
#define UDID_2       (*((U32*)(0x1FFFF7E8 + 4U)))
#define UDID_3       (*((U32*)(0x1FFFF7E8 + 8U)))

#endif /* __UNIQUEDEVID_H__ */
