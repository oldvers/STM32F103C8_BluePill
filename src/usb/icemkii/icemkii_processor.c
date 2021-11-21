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

#define RSP_OK                    (0x80)
#define RSP_SIGN_ON               (0x86)

/*----------------------------------------------------------------------------*/

#define ICEMKII_COMM_ID     (1)    // Communications protocol version
#define ICEMKII_BLDR_FW     (255)  // M_MCU boot-loader FW version
#define ICEMKII_MCU_FW_MIN  (33)   // M_MCU firmware version (minor)
#define ICEMKII_MCU_FW_MAJ  (6)    // M_MCU firmware version (major)
#define ICEMKII_SN          {0x40, 0x15, 0x13, 0x03, 0x85, 0x19}
#define ICEMKII_ID_STR      ("JTAGICEmkII\0")

/*----------------------------------------------------------------------------*/

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

typedef __packed struct ICEMKII_MSG_BODY_s
{
  U8 MESSAGE_ID;
  union
  {
    ICEMKII_RSP_SIGN_ON_t rspSignOn;
  };
} ICEMKII_MSG_BODY_t, * ICEMKII_MSG_BODY_p;

/*----------------------------------------------------------------------------*/

void icemkii_SignOn(U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;
  U8 sn[6] = ICEMKII_SN;
  //U8 sn[6] = {0x07, 0x00, 0x00, 0x00, 0x46, 0x93};

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

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

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