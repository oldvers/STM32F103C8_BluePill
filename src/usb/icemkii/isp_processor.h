#ifndef __ISP_PROCESSOR_H__
#define __ISP_PROCESSOR_H__

#include "types.h"

typedef enum
{
  ISP_FLASH = 0,
  ISP_EEPROM,
} ISP_MEMORY_t;

typedef struct ISP_PARAMETERS_s
{
  U32  buildVersion;
  U8   hwVersion;
  U8   swVersionMajor;
  U8   swVersionMinor;
  U8   vTarget;
  U8   vRef;
  U8   oscPrescale;
  U8   oscCMatch;
  U8   sckDuration;
  U8   topCardDetect;
  U8   status;
  U8   resetPolarity;
  U8   controllerInit;
  U8   oscCMatchLo;
  U8   oscCMatchHi;
  U8   cmdTimeout;
  U8   stabDelay;
  U8   cmdDelay;
  U8   synchLoops;
  U8   byteDelay;
  U8   pollValue[2];
  U8   pollIndex;
  U8   cmd[4];
  U8   preDelay;
  U8   postDelay;
  union
  {
    U8  byte[4];
    U16 word[2];
    U32 value;
  } address;
  U8 pollMethod;
  U8 eraseDelay;
  U8 mode;
} ISP_PARAMETERS_t;

extern ISP_PARAMETERS_t gIspParams;

FW_RESULT  ISP_EnterProgMode  (void);
void       ISP_LeaveProgmode  (void);
U8         ISP_ReadFLSO       (void);
FW_RESULT  ISP_ReadMemory     (U8 * pBuffer, U32 size, ISP_MEMORY_t memory);
FW_RESULT  ISP_ChipErase      (void);
FW_RESULT  ISP_ProgramMemory  (U8 * pBuffer, U32 size, ISP_MEMORY_t memory);
void       ISP_ProgramFLSO    (void);
void       ISP_ResetTarget    (void);

#endif /* __ISP_PROCESSOR_H__ */
