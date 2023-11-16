#include "ispmkii_processor.h"
#include "isp_processor.h"

/*----------------------------------------------------------------------------*/

/***[ STK general command constants ]***/
#define CMD_SET_PARAMETER                   (0x02)
#define CMD_GET_PARAMETER                   (0x03)
#define CMD_LOAD_ADDRESS                    (0x06)

/***[ STK ISP command constants ]***/
#define CMD_ENTER_PROGMODE_ISP              (0x10)
#define CMD_LEAVE_PROGMODE_ISP              (0x11)
#define CMD_CHIP_ERASE_ISP                  (0x12)
#define CMD_PROGRAM_FLASH_ISP               (0x13)
#define CMD_READ_FLASH_ISP                  (0x14)
#define CMD_PROGRAM_EEPROM_ISP              (0x15)
#define CMD_READ_EEPROM_ISP                 (0x16)
#define CMD_PROGRAM_FUSE_ISP                (0x17)
#define CMD_READ_FUSE_ISP                   (0x18)
#define CMD_PROGRAM_LOCK_ISP                (0x19)
#define CMD_READ_LOCK_ISP                   (0x1A)
#define CMD_READ_SIGNATURE_ISP              (0x1B)
#define CMD_READ_OSCCAL_ISP                 (0x1C)

/***[ STK parameter constants ]***/
#define PARAM_SCK_DURATION                  (0x98)
#define PARAM_RESET_POLARITY                (0x9E)

/***[ STK status constants ]***/
#define STATUS_CMD_OK                       (0x00)
/*    Warnings */
#define STATUS_CMD_TOUT                     (0x80)
#define STATUS_RDY_BSY_TOUT                 (0x81)
#define STATUS_SET_PARAM_MISSING            (0x82)
/*    Errors */
#define STATUS_CMD_FAILED                   (0xC0)
#define STATUS_CKSUM_ERROR                  (0xC1)
#define STATUS_CMD_UNKNOWN                  (0xC9)

/*----------------------------------------------------------------------------*/

typedef __packed struct REQ_GET_PARAMETER_s
{
  U8 id;
} REQ_GET_PARAMETER_t, * REQ_GET_PARAMETER_p;

typedef __packed struct RSP_GET_PARAMETER_s
{
  U8 status;
  U8 value;
} RSP_GET_PARAMETER_t, * RSP_GET_PARAMETER_p;

/*----------------------------------------------------------------------------*/

typedef __packed struct REQ_SET_PARAMETER_s
{
  U8 id;
  U8 value;
} REQ_SET_PARAMETER_t, * REQ_SET_PARAMETER_p;

typedef __packed struct RSP_SET_PARAMETER_s
{
  U8 status;
} RSP_SET_PARAMETER_t, * RSP_SET_PARAMETER_p;

/*----------------------------------------------------------------------------*/

typedef __packed struct REQ_ENTER_PROG_MODE_s
{
  U8 timeout;     // Command time-out (in ms)
  U8 stabDelay;   // Delay (in ms) used for pin stabilization
  U8 cmdExeDelay; // Delay (in ms) in connection with the command execution
  U8 synchLoops;  // Number of synchronization loops
  U8 byteDelay;   // Delay (in ms) between each byte in the command
  U8 pollValue;   // Poll value: 0x53 for AVR, 0x69 for AT89xx
  U8 pollIndex;   // Start addr, rx byte: 0 = no polling, 3 = AVR, 4 = AT89xx
  U8 cmd1;        // Command Byte # 1 to be transmitted
  U8 cmd2;        // Command Byte # 2 to be transmitted
  U8 cmd3;        // Command Byte # 3 to be transmitted
  U8 cmd4;        // Command Byte # 4 to be transmitted
} REQ_ENTER_PROG_MODE_t, * REQ_ENTER_PROG_MODE_p;

typedef __packed struct RSP_ENTER_PROG_MODE_s
{
  U8 status;
} RSP_ENTER_PROG_MODE_t, * RSP_ENTER_PROG_MODE_p;

/*----------------------------------------------------------------------------*/

typedef __packed struct REQ_LEAVE_PROG_MODE_s
{
  U8 preDelay;  // Pre-delay (in ms)
  U8 postDelay; // Post-delay (in ms)
} REQ_LEAVE_PROG_MODE_t, * REQ_LEAVE_PROG_MODE_p;

typedef __packed struct RSP_LEAVE_PROG_MODE_s
{
  U8 status;
} RSP_LEAVE_PROG_MODE_t, * RSP_LEAVE_PROG_MODE_p;

/*----------------------------------------------------------------------------*/

/* Read Fuse/Lock/Signature/OscCalibration value */
typedef __packed struct REQ_READ_FLSO_s
{
  U8 RetAddr; // Return address
  U8 cmd[4];  // Command Byte #1-#4
} REQ_READ_FLSO_t, * REQ_READ_FLSO_p;

typedef __packed struct RSP_READ_FLSO_s
{
  U8 status1;
  U8 data;
  U8 status2;
} RSP_READ_FLSO_t, * RSP_READ_FLSO_p;

/*----------------------------------------------------------------------------*/

/* Load Address */
typedef __packed struct REQ_LOAD_ADDRESS_s
{
  U8 value[4];
} REQ_LOAD_ADDRESS_t, * REQ_LOAD_ADDRESS_p;

typedef __packed struct RSP_LOAD_ADDRESS_s
{
  U8 status;
} RSP_LOAD_ADDRESS_t, * RSP_LOAD_ADDRESS_p;

/*----------------------------------------------------------------------------*/

/* Read Memory */
typedef __packed struct REQ_READ_MEMORY_s
{
  U8 numBytes[2];
  U8 cmd;
} REQ_READ_MEMORY_t, * REQ_READ_MEMORY_p;

typedef __packed struct RSP_READ_MEMORY_s
{
  U8 status1;
  U8 data[1024];
} RSP_READ_MEMORY_t, * RSP_READ_MEMORY_p;

/*----------------------------------------------------------------------------*/

/* Chip Erase */
typedef __packed struct REQ_CHIP_ERASE_s
{
  U8 eraseDelay;
  U8 pollMethod;
  U8 cmd[4];
} REQ_CHIP_ERASE_t, * REQ_CHIP_ERASE_p;

typedef __packed struct RSP_CHIP_ERASE_s
{
  U8 status;
} RSP_CHIP_ERASE_t, * RSP_CHIP_ERASE_p;

/*----------------------------------------------------------------------------*/

/* Program Flash */
typedef __packed struct REQ_PROGRAM_FLASH_s
{
  U8 numBytes[2]; // Total number of bytes to program, MSB first
  U8 mode;        // Mode byte
  U8 delay;       // Delay, used for different types of programming
  U8 cmd[3];      // (Load Page, Write Program Memory)
                  // (Write Program Memory Page)
                  // (Read Program Memory)
  U8 poll[2];
  U8 data[1024];
} REQ_PROGRAM_FLASH_t, * REQ_PROGRAM_FLASH_p;

typedef __packed struct RSP_PROGRAM_FLASH_s
{
  U8 status;
} RSP_PROGRAM_FLASH_t, * RSP_PROGRAM_FLASH_p;

/*----------------------------------------------------------------------------*/

/* Program Fuse/Lock */
typedef __packed struct REQ_PROGRAM_FLSO_s
{
  U8 cmd[4];
} REQ_PROGRAM_FLSO_t, * REQ_PROGRAM_FLSO_p;

typedef __packed struct RSP_PROGRAM_FLSO_s
{
  U8 status1;
  U8 status2;
} RSP_PROGRAM_FLSO_t, * RSP_PROGRAM_FLSO_p;

/*----------------------------------------------------------------------------*/

typedef __packed struct ISP_REQ_s
{
  U16 size;
  U8  commandId;
  union
  {
    REQ_GET_PARAMETER_t   getParameter;
    REQ_SET_PARAMETER_t   setParameter;
    REQ_ENTER_PROG_MODE_t enterProgMode;
    REQ_LEAVE_PROG_MODE_t leaveProgMode;
    REQ_READ_FLSO_t       readFLSO;
    REQ_LOAD_ADDRESS_t    loadAddress;
    REQ_READ_MEMORY_t     readMemory;
    REQ_CHIP_ERASE_t      chipErase;
    REQ_PROGRAM_FLASH_t   programFlash;
    REQ_PROGRAM_FLSO_t    programFLSO;
  };
} ISP_REQ_t, * ISP_REQ_p;

typedef __packed struct ISP_RSP_s
{
  U8  answerId;
  union
  {
    RSP_GET_PARAMETER_t   getParameter;
    RSP_SET_PARAMETER_t   setParameter;
    RSP_ENTER_PROG_MODE_t enterProgMode;
    RSP_LEAVE_PROG_MODE_t leaveProgMode;
    RSP_READ_FLSO_t       readFLSO;
    RSP_LOAD_ADDRESS_t    loadAddress;
    RSP_READ_MEMORY_t     readMemory;
    RSP_CHIP_ERASE_t      chipErase;
    RSP_PROGRAM_FLASH_t   programFlash;
    RSP_PROGRAM_FLSO_t    programFLSO;
  };
} ISP_RSP_t, * ISP_RSP_p;

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_GetParameter(ISP_REQ_p req, ISP_RSP_p rsp, U32 * size)
{
  FW_BOOLEAN result = FW_TRUE;

  if (3 != req->size) return FW_FALSE;

  rsp->answerId = req->commandId;
  *size = 3;

  switch (req->getParameter.id)
  {
    case PARAM_SCK_DURATION:
      rsp->getParameter.status = STATUS_CMD_OK;
      rsp->getParameter.value = gIspParams.sckDuration;
      break;
    default:
      result = FW_FALSE;
      break;
  }

  return result;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_SetParameter(ISP_REQ_p req, ISP_RSP_p rsp, U32 * size)
{
  FW_BOOLEAN result = FW_TRUE;

  if (2 != req->size) return FW_FALSE;

  rsp->answerId = req->commandId;
  *size = 2;

  switch (req->setParameter.id)
  {
    case PARAM_RESET_POLARITY:
      rsp->setParameter.status = STATUS_CMD_OK;
      break;
    case PARAM_SCK_DURATION:
      rsp->setParameter.status = STATUS_CMD_OK;
      gIspParams.sckDuration = req->setParameter.value;
      break;
    default:
      result = FW_FALSE;
      break;
  }

  return result;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_EnterProgMode(ISP_REQ_p rq, ISP_RSP_p rp, U32 * size)
{
  FW_RESULT result = FW_SUCCESS;

  if (2 != rq->size) return FW_FALSE;

  gIspParams.cmdTimeout   = rq->enterProgMode.timeout;
  gIspParams.stabDelay    = rq->enterProgMode.stabDelay;
  gIspParams.cmdDelay     = rq->enterProgMode.cmdExeDelay;
  gIspParams.synchLoops   = rq->enterProgMode.synchLoops;
  gIspParams.byteDelay    = rq->enterProgMode.byteDelay;
  gIspParams.pollValue[0] = rq->enterProgMode.pollValue;
  gIspParams.pollIndex    = rq->enterProgMode.pollIndex;
  gIspParams.cmd[0]       = rq->enterProgMode.cmd1;
  gIspParams.cmd[1]       = rq->enterProgMode.cmd2;
  gIspParams.cmd[2]       = rq->enterProgMode.cmd3;
  gIspParams.cmd[3]       = rq->enterProgMode.cmd4;

  rp->answerId = rq->commandId;

  result = ISP_EnterProgMode();
  switch (result)
  {
    case FW_SUCCESS:
      rp->enterProgMode.status = STATUS_CMD_OK;
      break;
    case FW_TIMEOUT:
      rp->enterProgMode.status = STATUS_CMD_TOUT;
      break;
    default:
      rp->enterProgMode.status = STATUS_CMD_FAILED;
      break;
  }
  *size = 2;

  return FW_TRUE;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_LeaveProgMode(ISP_REQ_p rq, ISP_RSP_p rp, U32 * size)
{
  if (2 != rq->size) return FW_FALSE;

  ISP_LeaveProgmode();

  rp->answerId = rq->commandId;
  rp->enterProgMode.status = STATUS_CMD_OK;
  *size = 2;

  return FW_TRUE;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_ReadFLSO(ISP_REQ_p req, ISP_RSP_p rsp, U32 * size)
{
  if (4 != req->size) return FW_FALSE;

  gIspParams.pollIndex = req->readFLSO.RetAddr;
  gIspParams.cmd[0]    = req->readFLSO.cmd[0];
  gIspParams.cmd[1]    = req->readFLSO.cmd[1];
  gIspParams.cmd[2]    = req->readFLSO.cmd[2];
  gIspParams.cmd[3]    = req->readFLSO.cmd[3];

  rsp->answerId         = req->commandId;
  rsp->readFLSO.status1 = STATUS_CMD_OK;
  rsp->readFLSO.data    = ISP_ReadFLSO();
  rsp->readFLSO.status2 = STATUS_CMD_OK;
  *size = 4;

  return FW_TRUE;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_LoadAddress(ISP_REQ_p req, ISP_RSP_p rsp, U32 * size)
{
  if (2 != req->size) return FW_FALSE;

  gIspParams.address.byte[0] = req->loadAddress.value[3];
  gIspParams.address.byte[1] = req->loadAddress.value[2];
  gIspParams.address.byte[2] = req->loadAddress.value[1];
  gIspParams.address.byte[3] = req->loadAddress.value[0];

  rsp->answerId           = req->commandId;
  rsp->loadAddress.status = STATUS_CMD_OK;
  *size = 2;

  return FW_TRUE;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_ReadMemory
(
  ISP_REQ_p req,
  ISP_RSP_p rsp,
  U32 * size,
  ISP_MEMORY_t memory
)
{
  FW_RESULT result = FW_SUCCESS;
  U16 length = 0;

  length |= (req->readMemory.numBytes[1] << 0);
  length |= (req->readMemory.numBytes[0] << 8);

  if (length != (req->size - 3)) return FW_FALSE;

  gIspParams.cmd[0] = req->readMemory.cmd;

  result = ISP_ReadMemory(rsp->readMemory.data, length, memory);

  rsp->answerId = req->commandId;
  switch (result)
  {
    case FW_SUCCESS:
      rsp->readMemory.status1      = STATUS_CMD_OK;
      rsp->readMemory.data[length] = STATUS_CMD_OK;
      *size                        = (length + 3);
      break;
    default:
      rsp->enterProgMode.status    = STATUS_CMD_FAILED;
      *size                        = 2;
      break;
  }

  return FW_TRUE;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_ChipErase(ISP_REQ_p req, ISP_RSP_p rsp, U32 * size)
{
  FW_RESULT result = FW_SUCCESS;

  if (2 != req->size) return FW_FALSE;

  gIspParams.eraseDelay = req->chipErase.eraseDelay;
  gIspParams.pollMethod = req->chipErase.pollMethod;
  gIspParams.cmd[0]     = req->chipErase.cmd[0];
  gIspParams.cmd[1]     = req->chipErase.cmd[1];
  gIspParams.cmd[2]     = req->chipErase.cmd[2];
  gIspParams.cmd[3]     = req->chipErase.cmd[3];

  result = ISP_ChipErase();

  rsp->answerId = req->commandId;
  switch (result)
  {
    case FW_SUCCESS:
      rsp->chipErase.status = STATUS_CMD_OK;
      break;
    default:
      rsp->chipErase.status = STATUS_CMD_TOUT;
      break;
  }

  *size = 2;

  return FW_TRUE;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_ProgramMemory
(
  ISP_REQ_p req,
  ISP_RSP_p rsp,
  U32 * size,
  ISP_MEMORY_t memory
)
{
  FW_RESULT result = FW_SUCCESS;
  U16 length = 0;

  if (2 != req->size) return FW_FALSE;

  length |= (req->programFlash.numBytes[1] << 0);
  length |= (req->programFlash.numBytes[0] << 8);

  gIspParams.mode         = req->programFlash.mode;
  gIspParams.cmdDelay     = req->programFlash.delay;
  gIspParams.cmd[0]       = req->programFlash.cmd[0];
  gIspParams.cmd[1]       = req->programFlash.cmd[1];
  gIspParams.cmd[2]       = req->programFlash.cmd[2];
  gIspParams.pollValue[0] = req->programFlash.poll[0];
  gIspParams.pollValue[1] = req->programFlash.poll[1];

  result = ISP_ProgramMemory(req->programFlash.data, length, memory);

  rsp->answerId = req->commandId;
  switch (result)
  {
    case FW_SUCCESS:
      rsp->programFlash.status = STATUS_CMD_OK;
      break;
    default:
      rsp->programFlash.status = STATUS_CMD_TOUT;
      break;
  }
  *size = 2;

  return FW_TRUE;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_ProgramFLSO(ISP_REQ_p req, ISP_RSP_p rsp, U32 * size)
{
  if (3 != req->size) return FW_FALSE;

  gIspParams.cmd[0] = req->programFLSO.cmd[0];
  gIspParams.cmd[1] = req->programFLSO.cmd[1];
  gIspParams.cmd[2] = req->programFLSO.cmd[2];
  gIspParams.cmd[3] = req->programFLSO.cmd[3];

  ISP_ProgramFLSO();

  rsp->answerId            = req->commandId;
  rsp->programFLSO.status1 = STATUS_CMD_OK;
  rsp->programFLSO.status2 = STATUS_CMD_OK;

  *size = 3;

  return FW_TRUE;
}

/*----------------------------------------------------------------------------*/

FW_BOOLEAN ISPMKII_Process(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  FW_BOOLEAN result = FW_FALSE;
  ISP_REQ_p  req = (ISP_REQ_p)pReqBody;
  ISP_RSP_p  rsp = (ISP_RSP_p)pRspBody;

  switch (req->commandId)
  {
    case CMD_GET_PARAMETER:
      result = ispmkii_GetParameter(req, rsp, pSize);
      break;
    case CMD_SET_PARAMETER:
      result = ispmkii_SetParameter(req, rsp, pSize);
      break;
    case CMD_ENTER_PROGMODE_ISP:
      result = ispmkii_EnterProgMode(req, rsp, pSize);
      break;
    case CMD_LEAVE_PROGMODE_ISP:
      result = ispmkii_LeaveProgMode(req, rsp, pSize);
      break;
    case CMD_READ_FUSE_ISP:
    case CMD_READ_LOCK_ISP:
    case CMD_READ_SIGNATURE_ISP:
    case CMD_READ_OSCCAL_ISP:
      result = ispmkii_ReadFLSO(req, rsp, pSize);
      break;
    case CMD_LOAD_ADDRESS:
      result = ispmkii_LoadAddress(req, rsp, pSize);
      break;
    case CMD_READ_FLASH_ISP:
      result = ispmkii_ReadMemory(req, rsp, pSize, ISP_FLASH);
      break;
    case CMD_READ_EEPROM_ISP:
      result = ispmkii_ReadMemory(req, rsp, pSize, ISP_EEPROM);
      break;
    case CMD_CHIP_ERASE_ISP:
      result = ispmkii_ChipErase(req, rsp, pSize);
      break;
    case CMD_PROGRAM_FLASH_ISP:
      result = ispmkii_ProgramMemory(req, rsp, pSize, ISP_FLASH);
      break;
    case CMD_PROGRAM_EEPROM_ISP:
      result = ispmkii_ProgramMemory(req, rsp, pSize, ISP_EEPROM);
      break;
    case CMD_PROGRAM_FUSE_ISP:
    case CMD_PROGRAM_LOCK_ISP:
      result = ispmkii_ProgramFLSO(req, rsp, pSize);
      break;
  }

  return result;
}

/*----------------------------------------------------------------------------*/

void ISPMKII_Reset(void)
{
  ISP_ResetTarget();
}

/*----------------------------------------------------------------------------*/
