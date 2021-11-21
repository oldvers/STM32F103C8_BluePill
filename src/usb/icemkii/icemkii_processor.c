#include <string.h>
#include "icemkii_processor.h"
#include "debug.h"

#define ICEMKII_DEBUG

#ifdef ICEMKII_DEBUG
#  define ICEMKII_LOG    DBG
#else
#  define ICEMKII_LOG(...)
#endif

/*----------------------------------------------------------------------------*/

#define CMND_SIGN_OFF             (0x00)
#define CMND_GET_SIGN_ON          (0x01)
#define CMND_SET_PARAMETER        (0x02)
#define CMND_GO                   (0x08)
#define CMND_RESET                (0x0B)
#define CMND_ISP_PACKET           (0x2F)

#define RSP_OK                    (0x80)
#define RSP_SIGN_ON               (0x86)
#define RSP_FAILED                (0xA0)

/*----------------------------------------------------------------------------*/

#define ICEMKII_COMM_ID     (1)    // Communications protocol version
#define ICEMKII_BLDR_FW     (255)  // M_MCU boot-loader FW version
#define ICEMKII_MCU_FW_MIN  (33)   // M_MCU firmware version (minor)
#define ICEMKII_MCU_FW_MAJ  (6)    // M_MCU firmware version (major)
#define ICEMKII_SN          {0x40, 0x15, 0x13, 0x03, 0x85, 0x19}
#define ICEMKII_ID_STR      ("JTAGICEmkII\0")

typedef __packed struct ICEMKII_RSP_SIGN_ON_s
{
  U8 COMM_ID;           // Communications protocol version [BYTE]
  U8 M_MCU_BLDR;        // M_MCU boot-loader FW version    [BYTE]
  U8 M_MCU_FW_MIN;      // M_MCU firmware version (minor)  [BYTE]
  U8 M_MCU_FW_MAJ;      // M_MCU firmware version (major)  [BYTE]
  U8 M_MCU_HW;          // M_MCU hardware version          [BYTE]
  U8 S_MCU_BLDR;        // S_MCU boot-loader FW version    [BYTE]
  U8 S_MCU_FW_MIN;      // S_MCU firmware version (minor)  [BYTE]
  U8 S_MCU_FW_MAJ;      // S_MCU firmware version (major)  [BYTE]
  U8 S_MCU_HW;          // S_MCU hardware version          [BYTE]
  U8 SERIAL_NUMBER[6];  // (USB) EEPROM stored s/n         [6*BYTE], LSB FIRST
  U8 DEVICE_ID_STR[12]; // “JTAGICE mkII\0” Null terminated ASCII string
} ICEMKII_RSP_SIGN_ON_t, * ICEMKII_RSP_SIGN_ON_p;

/*----------------------------------------------------------------------------*/

#define PARAMETER_ID_EMULATOR_MODE           (0x03)
#  define EMULATOR_MODE_DBG_WIRE             (0x00)
#  define EMULATOR_MODE_JTAG_MEGA_AVR        (0x01)
#  define EMULATOR_MODE_UNKNOWN              (0x02)
#  define EMULATOR_MODE_SPI                  (0x03)
#  define EMULATOR_MODE_JTAG_AVR32           (0x04)
#  define EMULATOR_MODE_JTAG_XMEGA           (0x05)
#  define EMULATOR_MODE_PDI_XMEGA            (0x06)

typedef __packed struct ICEMKII_SET_PARAMETER_EMULATOR_MODE_s
{
  U8 mode;
} ICEMKII_SET_PARAMETER_EMULATOR_MODE_t;

typedef __packed struct ICEMKII_REQ_SET_PARAMETER_s
{
  U8 PARAMETER_ID;
  union
  {
    U8                                    PARAMETER_VALUE;
    ICEMKII_SET_PARAMETER_EMULATOR_MODE_t emulator;
  };
} ICEMKII_REQ_SET_PARAMETER_t, * ICEMKII_REQ_SET_PARAMETER_p;

/*----------------------------------------------------------------------------*/

#define RESET_FLAG_LOW_LEVEL     (0x01) // Low level
#define RESET_FLAG_HIGH_LEVEL    (0x02) // High level (reset, then run to main)
#define RESET_FLAG_DBG_WITE_DSBL (0x04) // Reset with debugWire disable

typedef __packed struct ICEMKII_REQ_RESET_s
{
  U8 FLAG;
} ICEMKII_REQ_RESET_t, * ICEMKII_REQ_RESET_p;

/*----------------------------------------------------------------------------*/

typedef __packed struct ICEMKII_MSG_BODY_s
{
  U8 MESSAGE_ID;
  union
  {
    ICEMKII_RSP_SIGN_ON_t       rspSignOn;
    ICEMKII_REQ_SET_PARAMETER_t reqSetParameter;
    ICEMKII_REQ_RESET_t         reqReset;
  };
} ICEMKII_MSG_BODY_t, * ICEMKII_MSG_BODY_p;

/*----------------------------------------------------------------------------*/

void icemkii_SignOn(U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;
  U8 sn[6] = ICEMKII_SN;

  ICEMKII_LOG("ICE Rx: Sign On\r\n");

  rsp->MESSAGE_ID             = RSP_SIGN_ON;
  rsp->rspSignOn.COMM_ID      = ICEMKII_COMM_ID;
  rsp->rspSignOn.M_MCU_BLDR   = ICEMKII_BLDR_FW;
  rsp->rspSignOn.M_MCU_FW_MIN = ICEMKII_MCU_FW_MIN;
  rsp->rspSignOn.M_MCU_FW_MAJ = ICEMKII_MCU_FW_MAJ;
  rsp->rspSignOn.M_MCU_HW     = 0x00;
  rsp->rspSignOn.S_MCU_BLDR   = ICEMKII_BLDR_FW;
  rsp->rspSignOn.S_MCU_FW_MIN = ICEMKII_MCU_FW_MIN;
  rsp->rspSignOn.S_MCU_FW_MAJ = ICEMKII_MCU_FW_MAJ;
  rsp->rspSignOn.S_MCU_HW     = 0x01;
  memcpy(rsp->rspSignOn.SERIAL_NUMBER, sn, 6);
  memcpy(rsp->rspSignOn.DEVICE_ID_STR, ICEMKII_ID_STR, 12);

  *pSize = (sizeof(ICEMKII_RSP_SIGN_ON_t) + 1);
}

/*----------------------------------------------------------------------------*/

void icemkii_SignOff(U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;

  ICEMKII_LOG("ICE Rx: Sign Off\r\n");

  rsp->MESSAGE_ID = RSP_OK;
  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

void icemkii_SetParameter(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p req = (ICEMKII_MSG_BODY_p)pReqBody;
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;

  ICEMKII_LOG("ICE Rx: Set Parameter\r\n");
  rsp->MESSAGE_ID = RSP_FAILED;

  switch (req->reqSetParameter.PARAMETER_ID)
  {
    case PARAMETER_ID_EMULATOR_MODE:
      if (EMULATOR_MODE_SPI == req->reqSetParameter.emulator.mode)
      {
        rsp->MESSAGE_ID = RSP_OK;
      }
      break;
    default:
      break;
  }

  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

void icemkii_IspPacket(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  //ICEMKII_MSG_BODY_p req = (ICEMKII_MSG_BODY_p)pReqBody;
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;

  ICEMKII_LOG("ICE Rx: ISP Packet\r\n");

  rsp->MESSAGE_ID = RSP_FAILED;
  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

void icemkii_Reset(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p req = (ICEMKII_MSG_BODY_p)pReqBody;
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;

  ICEMKII_LOG("ICE Rx: Reset\r\n");
  rsp->MESSAGE_ID = RSP_FAILED;

  switch (req->reqReset.FLAG)
  {
    case RESET_FLAG_LOW_LEVEL:
      rsp->MESSAGE_ID = RSP_OK;
      break;
    default:
      break;
  }

  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

void icemkii_Go(U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;

  ICEMKII_LOG("ICE Rx: Go\r\n");

  rsp->MESSAGE_ID = RSP_OK;
  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

void ICEMKII_Process(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  U32 i = 0;

  ICEMKII_LOG("--> ");
  for (i = 0; i < *pSize; i++)
  {
    ICEMKII_LOG("%02X ", pReqBody[i]);
  }
  ICEMKII_LOG("\r\n");


  switch ( ((ICEMKII_MSG_BODY_p)pReqBody)->MESSAGE_ID )
  {
    case CMND_GET_SIGN_ON:
      icemkii_SignOn(pRspBody, pSize);
      break;
    case CMND_SIGN_OFF:
      icemkii_SignOff(pRspBody, pSize);
      break;
    case CMND_SET_PARAMETER:
      icemkii_SetParameter(pReqBody, pRspBody, pSize);
      break;
    case CMND_ISP_PACKET:
      icemkii_IspPacket(pReqBody, pRspBody, pSize);
      break;
    case CMND_RESET:
      icemkii_Reset(pReqBody, pRspBody, pSize);
      break;
    case CMND_GO:
      icemkii_Go(pRspBody, pSize);
      break;
    default:
      *pSize = 0;
      break;
  }

  if (0 < *pSize)
  {
    ICEMKII_LOG("<-- ");
    for (i = 0; i < *pSize; i++)
    {
      ICEMKII_LOG("%02X ", pRspBody[i]);
    }
    ICEMKII_LOG("\r\n");
  }
}