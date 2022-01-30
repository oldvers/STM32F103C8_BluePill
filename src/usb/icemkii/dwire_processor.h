#ifndef __DWIRE_PROCESSOR_H__
#define __DWIRE_PROCESSOR_H__

#include "types.h"

void       DWire_Init          (void);
FW_BOOLEAN DWire_Sync          (void);
U16        DWire_ReadSignature (void);
U16        DWire_ReadPc        (void);
FW_BOOLEAN DWire_Disable       (void);
FW_BOOLEAN DWire_ReadRegs      (U16 first, U8 * pBuffer, U16 count);

#endif /* __DWIRE_PROCESSOR_H__ */
