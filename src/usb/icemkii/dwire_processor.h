#ifndef __DWIRE_PROCESSOR_H__
#define __DWIRE_PROCESSOR_H__

#include "types.h"

void       DWire_Init          (void);
FW_BOOLEAN DWire_Sync          (void);
U16        DWire_ReadSignature (void);

#endif /* __DWIRE_PROCESSOR_H__ */
