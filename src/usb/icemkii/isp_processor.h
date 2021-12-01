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
  /* Global Parameters */
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
  U8   data;
  U8   resetPolarity;
  U8   controllerInit;
  U8   oscCMatchLo;
  U8   oscCMatchHi;
  /* Enter Programming Mode parameters */
  U8   timeout;     // Command time-out (in ms)
  U8   stabDelay;   // Delay (in ms) used for pin stabilization
  U8   cmdExeDelay; // Delay (in ms) in connection with the command execution
  U8   synchLoops;  // Number of synchronization loops
  U8   byteDelay;   // Delay (in ms) between each byte in the command
  U8   pollValue[2]; // Poll value: 0x53 for AVR, 0x69 for AT89xx
  U8   pollIndex;   // Start addr, rx byte: 0 = no polling, 3 = AVR, 4 = AT89xx
  U8   cmd[4];      // Command Byte # 1 to be transmitted
  //U8   cmd2;        // Command Byte # 2 to be transmitted
  //U8   cmd3;        // Command Byte # 3 to be transmitted
  //U8   cmd4;        // Command Byte # 4 to be transmitted
  /* Exit Programming Mode parameters */
  U8   preDelay;    // Pre-delay (in ms)
  U8   postDelay;   // Post-delay (in ms)
  /* Read Fuse/Lock/Signature/OscCalibration value parameters */
  //U8   retAddr;     // Return address
  //U8   cmd1;        // Command Byte #1
  //U8   cmd2;        // Command Byte #2
  //U8   cmd3;        // Command Byte #3
  //U8   cmd4;        // Command Byte #4
  /* Load Address parameters */
  union
  {
    U8  byte[4];
    U16 word[2];
    U32 value;
  } address;
  /* Read Memory Parameters */
  //U16 length;
  /* Chip Erase */
  U8 pollMethod;
  U8 eraseDelay;
  /* Program Flash */
  U8 mode;
} ISP_PARAMETERS_t;

extern ISP_PARAMETERS_t gIspParams;

FW_RESULT ISP_EnterProgMode(void);
void      ISP_LeaveProgmode(void);
U8        ISP_ReadFLSO     (void);
FW_RESULT ISP_ReadMemory   (U8 * pBuffer, U32 size, ISP_MEMORY_t memory);
FW_RESULT ISP_ChipErase    (void);
FW_RESULT ISP_ProgramMemory(U8 * pBuffer, U32 size, ISP_MEMORY_t memory);
void      ISP_ProgramFLSO  (void);
void      ISP_ResetTarget  (void);

#endif /* __ISP_PROCESSOR_H__ */
