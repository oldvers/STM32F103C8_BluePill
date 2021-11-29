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
#define CMD_READ_FLASH_ISP                  (0x14)
#define CMD_READ_FUSE_ISP                   (0x18)
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
  U8 cmd1;    // Command Byte #1
  U8 cmd2;    // Command Byte #2
  U8 cmd3;    // Command Byte #3
  U8 cmd4;    // Command Byte #4
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
  U8 cmd1;
} REQ_READ_MEMORY_t, * REQ_READ_MEMORY_p;

typedef __packed struct RSP_READ_MEMORY_s
{
  U8 status1;
  U8 data[1024];
} RSP_READ_MEMORY_t, * RSP_READ_MEMORY_p;


/*----------------------------------------------------------------------------*/

typedef __packed struct ISP_REQ_s
{
  U16 Size;
  U8  CommandId;
  union
  {
    REQ_GET_PARAMETER_t   getParameter;
    REQ_SET_PARAMETER_t   setParameter;
    REQ_ENTER_PROG_MODE_t enterProgMode;
    REQ_LEAVE_PROG_MODE_t leaveProgMode;
    REQ_READ_FLSO_t       readFLSO;
    REQ_LOAD_ADDRESS_t    loadAddress;
    REQ_READ_MEMORY_t     readMemory;
  };
} ISP_REQ_t, * ISP_REQ_p;

typedef __packed struct ISP_RSP_s
{
  U8  AnswerId;
  union
  {
    RSP_GET_PARAMETER_t   getParameter;
    RSP_SET_PARAMETER_t   setParameter;
    RSP_ENTER_PROG_MODE_t enterProgMode;
    RSP_LEAVE_PROG_MODE_t leaveProgMode;
    RSP_READ_FLSO_t       readFLSO;
    RSP_LOAD_ADDRESS_t    loadAddress;
    RSP_READ_MEMORY_t     readMemory;
  };
} ISP_RSP_t, * ISP_RSP_p;

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_GetParameter(U8 id, ISP_RSP_p pRsp, U32 * pSize)
{
  FW_BOOLEAN result = FW_TRUE;

  pRsp->AnswerId = CMD_GET_PARAMETER;
  *pSize = 3;

  switch (id)
  {
    case PARAM_SCK_DURATION:
      pRsp->getParameter.status = STATUS_CMD_OK;
      pRsp->getParameter.value = gIspParams.sckDuration;
      break;
    default:
      result = FW_FALSE;
      break;
  }

  return result;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_SetParameter(U8 id, U8 v, ISP_RSP_p pRsp, U32 * pSize)
{
  FW_BOOLEAN result = FW_TRUE;

  pRsp->AnswerId = CMD_SET_PARAMETER;
  *pSize = 2;

  switch (id)
  {
    case PARAM_RESET_POLARITY:
      pRsp->setParameter.status = STATUS_CMD_OK;
      break;
    case PARAM_SCK_DURATION:
      pRsp->setParameter.status = STATUS_CMD_OK;
      gIspParams.sckDuration = v;
      break;
    default:
      result = FW_FALSE;
      break;
  }

  return result;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_EnterProgMode
(
  ISP_REQ_p pReq,
  ISP_RSP_p pRsp,
  U32 * pSize
)
{
  FW_RESULT result = FW_SUCCESS;
  REQ_ENTER_PROG_MODE_p req = &((ISP_REQ_p)pReq)->enterProgMode;

  gIspParams.timeout     = req->timeout;
  gIspParams.stabDelay   = req->stabDelay;
  gIspParams.cmdExeDelay = req->cmdExeDelay;
  gIspParams.synchLoops  = req->synchLoops;
  gIspParams.byteDelay   = req->byteDelay;
  gIspParams.pollValue   = req->pollValue;
  gIspParams.pollIndex   = req->pollIndex;
  gIspParams.cmd[0]      = req->cmd1;
  gIspParams.cmd[1]      = req->cmd2;
  gIspParams.cmd[2]      = req->cmd3;
  gIspParams.cmd[3]      = req->cmd4;

  pRsp->AnswerId = pReq->CommandId;

  result = ISP_EnterProgMode();
  switch (result)
  {
    case FW_SUCCESS:
      pRsp->enterProgMode.status = STATUS_CMD_OK;
      break;
    case FW_TIMEOUT:
      pRsp->enterProgMode.status = STATUS_CMD_TOUT;
      break;
    default:
      pRsp->enterProgMode.status = STATUS_CMD_FAILED;
      break;
  }
  *pSize = 2;

  return FW_TRUE;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_LeaveProgMode
(
  ISP_REQ_p pReq,
  ISP_RSP_p pRsp,
  U32 * pSize
)
{
  FW_BOOLEAN result = FW_TRUE;

  ISP_LeaveProgmode();

  pRsp->AnswerId = CMD_LEAVE_PROGMODE_ISP;
  pRsp->enterProgMode.status = STATUS_CMD_OK;
  *pSize = 2;

  return result;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_ReadFLSO
(
  ISP_REQ_p pReq,
  ISP_RSP_p pRsp,
  U32 * pSize
)
{
//  FW_BOOLEAN result = FW_TRUE;
//  /* ATmega48 signature/fuses */
//  U8 sign[3] = {0x1E, 0x92, 0x05};
//  /* Default */
//  //U8 fuse[3] = {0x62, 0xDF, 0xFF};
//  /* DebugWire enabled */
//  U8 fuse[3] = {0x62, 0x9F, 0xFF};

//  pRsp->AnswerId = pReq->CommandId;
//  pRsp->readFLSO.status1 = STATUS_CMD_OK;
//  pRsp->readFLSO.status2 = STATUS_CMD_OK;
//  *pSize = 4;

//  //Read signature
//  if (0x30 == pReq->readFLSO.cmd1)
//  {
//    pRsp->readFLSO.data = sign[pReq->readFLSO.cmd3];
//  }
//  else if ((0x50 == pReq->readFLSO.cmd1) && (0x00 == pReq->readFLSO.cmd2))
//  {
//    pRsp->readFLSO.data = fuse[0];
//  }
//  else if ((0x58 == pReq->readFLSO.cmd1) && (0x08 == pReq->readFLSO.cmd2))
//  {
//    pRsp->readFLSO.data = fuse[1];
//  }
//  else if ((0x50 == pReq->readFLSO.cmd1) && (0x08 == pReq->readFLSO.cmd2))
//  {
//    pRsp->readFLSO.data = fuse[2];
//  }
//  else
//  {
//    pRsp->readFLSO.data = 0xFF;
//  }

//  return result;

//  FW_RESULT result = FW_SUCCESS;
  REQ_READ_FLSO_p req = &((ISP_REQ_p)pReq)->readFLSO;

  gIspParams.pollIndex   = req->RetAddr;
  gIspParams.cmd[0]      = req->cmd1;
  gIspParams.cmd[1]      = req->cmd2;
  gIspParams.cmd[2]      = req->cmd3;
  gIspParams.cmd[3]      = req->cmd4;

  pRsp->AnswerId         = pReq->CommandId;
  pRsp->readFLSO.status1 = STATUS_CMD_OK;
  pRsp->readFLSO.data    = ISP_ReadFLSO();
  pRsp->readFLSO.status2 = STATUS_CMD_OK;

  *pSize = 4;

  return FW_TRUE;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_LoadAddress
(
  ISP_REQ_p pReq,
  ISP_RSP_p pRsp,
  U32 * pSize
)
{
  FW_BOOLEAN result = FW_TRUE;

  //gIspParams.address  = 0;
  gIspParams.address.byte[0] = pReq->loadAddress.value[3];
  gIspParams.address.byte[1] = pReq->loadAddress.value[2];
  gIspParams.address.byte[2] = pReq->loadAddress.value[1];
  gIspParams.address.byte[3] = pReq->loadAddress.value[0];

  pRsp->AnswerId = pReq->CommandId;
  pRsp->loadAddress.status = STATUS_CMD_OK;
  *pSize = 2;

  return result;
}

/*----------------------------------------------------------------------------*/

static FW_BOOLEAN ispmkii_ReadFlash
(
  ISP_REQ_p pReq,
  ISP_RSP_p pRsp,
  U32 * pSize
)
{
  REQ_READ_MEMORY_p req = &((ISP_REQ_p)pReq)->readMemory;
  RSP_READ_MEMORY_p rsp = &((ISP_RSP_p)pRsp)->readMemory;
  FW_RESULT result = FW_SUCCESS;
  U16 length = 0;

  gIspParams.cmd[0] = req->cmd1;
  length  = 0;
  length |= (req->numBytes[1] << 0);
  length |= (req->numBytes[0] << 8);

  result = ISP_ReadMemory(rsp->data, length);

  pRsp->AnswerId = pReq->CommandId;
  switch (result)
  {
    case FW_SUCCESS:
      pRsp->readMemory.status1      = STATUS_CMD_OK;
      pRsp->readMemory.data[length] = STATUS_CMD_OK;
      *pSize                        = (length + 3);
      break;
    default:
      pRsp->enterProgMode.status    = STATUS_CMD_FAILED;
      *pSize                        = 2;
      break;
  }

  return FW_TRUE;
}

/*----------------------------------------------------------------------------*/

FW_BOOLEAN ISPMKII_Process(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  FW_BOOLEAN result = FW_FALSE;
  ISP_REQ_p  req = (ISP_REQ_p)pReqBody;
  ISP_RSP_p  rsp = (ISP_RSP_p)pRspBody;

  switch (req->CommandId)
  {
    case CMD_GET_PARAMETER:
      if (3 == req->Size)
      {
        result = ispmkii_GetParameter(req->getParameter.id, rsp, pSize);
      }
      break;
    case CMD_SET_PARAMETER:
      if (2 == req->Size)
      {
        result = ispmkii_SetParameter
                 (
                   req->setParameter.id,
                   req->setParameter.value,
                   rsp,
                   pSize
                 );
      }
      break;
    case CMD_ENTER_PROGMODE_ISP:
      if (2 == req->Size)
      {
        result = ispmkii_EnterProgMode(req, rsp, pSize);
      }
      break;
    case CMD_LEAVE_PROGMODE_ISP:
      if (2 == req->Size)
      {
        result = ispmkii_LeaveProgMode(req, rsp, pSize);
      }
      break;
    case CMD_READ_FUSE_ISP:
    case CMD_READ_LOCK_ISP:
    case CMD_READ_SIGNATURE_ISP:
    case CMD_READ_OSCCAL_ISP:
      if (4 == req->Size)
      {
        result = ispmkii_ReadFLSO(req, rsp, pSize);
      }
      break;
    case CMD_LOAD_ADDRESS:
      if (2 == req->Size)
      {
        result = ispmkii_LoadAddress(req, rsp, pSize);
      }
      break;
    case CMD_READ_FLASH_ISP:
      result = ispmkii_ReadFlash(req, rsp, pSize);
      break;
  }

  return result;
}