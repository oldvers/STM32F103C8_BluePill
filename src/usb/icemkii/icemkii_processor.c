#include <string.h>
#include "icemkii_processor.h"
#include "ispmkii_processor.h"
#include "debug.h"

#define ICEMKII_DEBUG

#ifdef ICEMKII_DEBUG
#  define ICEMKII_LOG    DBG
#else
#  define ICEMKII_LOG(...)
#endif

/*----------------------------------------------------------------------------*/

typedef __packed struct DEVICE_DESCRIPTOR_s
{
  U8  ucReadIO[8];           //LSB = IOloc  0, MSB = IOloc63
  U8  ucReadIOShadow[8];     //LSB = IOloc  0, MSB = IOloc63
  U8  ucWriteIO[8];          //LSB = IOloc  0, MSB = IOloc63
  U8  ucWriteIOShadow[8];    //LSB = IOloc  0, MSB = IOloc63
  U8  ucReadExtIO[52];       //LSB = IOloc  96, MSB = IOloc511
  U8  ucReadIOExtShadow[52]; //LSB = IOloc  96, MSB = IOloc511
  U8  ucWriteExtIO[52];      //LSB = IOloc  96, MSB = IOloc511
  U8  ucWriteIOExtShadow[52];//LSB = IOloc  96, MSB = IOloc511
  U8  ucIDRAddress;          //IDR address
  U8  ucSPMCRAddress; //SPMCR Register address and dW BasePC
  U32 ulBootAddress;  //Device Boot Loader Start Address
  U8  ucRAMPZAddress; //RAMPZ Register address in SRAM I/O space
  U32 uiFlashPageSize;  //Device Flash Page Size, Size = 2 exp ucFlashPageSize
  U8  ucEepromPageSize;  //Device Eeprom Page Size in bytes
  U32 uiUpperExtIOLoc;  //Topmost (last) extended I/O location, 0 if no external I/O
  U32 ulFlashSize;   //Device Flash Size
  U8  ucEepromInst[20];  //Instructions for W/R EEPROM
  U8  ucFlashInst[3];  //Instructions for W/R FLASH
  U8  ucSPHaddr;    // Stack pointer high
  U8  ucSPLaddr;    // Stack pointer low
  U32 uiFlashpages;  // number of pages in flash
  U8  ucDWDRAddress;  // DWDR register address
  U8  ucDWBasePC;    // Base/mask value of the PC
  U8  ucAllowFullPageBitstream; // FALSE on ALL new parts
  U32 uiStartSmallestBootLoaderSection; //
  U8  ucEnablePageProgramming; // For JTAG parts only, default TRUE
  U8  ucCacheType;    // CacheType_Normal 0x00, CacheType_CAN 0x01,
  U32 uiSramStartAddr;   // Start of SRAM
  U8  ucResetType;   // Selects reset type. 0x00
  U8  ucPCMaskExtended;   // For parts with extended PC
  U8  ucPCMaskHigh;  // PC high mask
  U8  ucEindAddress;  // EIND IO address
  U32 uiEECRAddress;    // EECR IO address
} DEVICE_DESCRIPTOR_t;

/*----------------------------------------------------------------------------*/

#define CMND_SIGN_OFF                  (0x00)
#define CMND_GET_SIGN_ON               (0x01)
#define CMND_SET_PARAMETER             (0x02)
#define CMND_GET_PARAMETER             (0x03)
#define CMND_GO                        (0x08)
#define CMND_RESET                     (0x0B)
#define CMND_ISP_PACKET                (0x2F)
#define CMND_SET_DEVICE_DESCRIPTOR     (0x0C)
#define CMND_FORCED_STOP               (0x0A)
#define CMND_WRITE_MEMORY              (0x04)
#define CMND_CLEAR_EVENTS              (0x22)

#define RSP_OK                         (0x80)
#define RSP_PARAMETER                  (0x81)
#define RSP_SIGN_ON                    (0x86)
#define RSP_SPI_DATA                   (0x88)
#define RSP_FAILED                     (0xA0)

/*----------------------------------------------------------------------------*/

#define ICEMKII_COMM_ID     (1)    // Communications protocol version
#define ICEMKII_BLDR_FW     (255)  // M_MCU boot-loader FW version
#define ICEMKII_MCU_FW_MIN  (0x33) // M_MCU firmware version (minor)
#define ICEMKII_MCU_FW_MAJ  (0x06) // M_MCU firmware version (major)
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

#define PARAMETER_ID_RUN_AFTER_PROG          (0x38)
#  define RUN_AFTER_PROG_STOP                (0x00)
#  define RUN_AFTER_PROG_ALLOW               (0x01)

#define PARAMETER_ID_OCD_VTARGET             (0x06)

#define PARAMETER_ID_TARGET_SIGNATURE        (0x1D)

#define PARAMETER_ID_TIMERS_UNDER_DBG        (0x09)
#  define TIMERS_UNDER_DBG_STOPPED           (0x00)
#  define TIMERS_UNDER_DBG_RUNNING           (0x01)

#define PARAMETER_ID_PDI_APPL_OFFS           (0x32)  //PDI offset of flash application section  [BYTE] * 4, LSB first  W  Device specific
#define PARAMETER_ID_PDI_BOOT_OFFS           (0x33)  //PDI offset of flash boot section  [BYTE] * 4, LSB first  W  Device specific

#define PARAMETER_ID_UNKNOWN                 (0x29)

/*----------------------------------------------------------------------------*/

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

typedef __packed struct ICEMKII_GET_PARAMETER_OCD_VTARGET_s
{
  U16 voltage;
} ICEMKII_GET_PARAMETER_OCD_VTARGET_t;

typedef __packed struct ICEMKII_GET_PARAMETER_TGT_SIGN_s
{
  U16 value;
} ICEMKII_GET_PARAMETER_TGT_SIGN_t;

typedef __packed struct ICEMKII_REQ_GET_PARAMETER_s
{
  U8 PARAMETER_ID;
} ICEMKII_REQ_GET_PARAMETER_t, * ICEMKII_REQ_GET_PARAMETER_p;

typedef __packed struct ICEMKII_RSP_GET_PARAMETER_s
{
  union
  {
    U8                                    PARAMETER_VALUE;
    ICEMKII_GET_PARAMETER_OCD_VTARGET_t   ocdVtarget;
    ICEMKII_GET_PARAMETER_TGT_SIGN_t      targetSign;
  };
} ICEMKII_RSP_GET_PARAMETER_t, * ICEMKII_RSP_GET_PARAMETER_p;

/*----------------------------------------------------------------------------*/

#define RESET_FLAG_LOW_LEVEL     (0x01) // Low level
#define RESET_FLAG_HIGH_LEVEL    (0x02) // High level (reset, then run to main)
#define RESET_FLAG_DBG_WITE_DSBL (0x04) // Reset with debugWire disable

typedef __packed struct ICEMKII_REQ_RESET_s
{
  U8 FLAG;
} ICEMKII_REQ_RESET_t, * ICEMKII_REQ_RESET_p;

/*----------------------------------------------------------------------------*/

#define FORCED_STOP_MODE_LOW_LEVEL    (0x01)
#define FORCED_STOP_MODE_HIGH_LEVEL   (0x02)

typedef __packed struct ICEMKII_REQ_FORCED_STOP_s
{
  U8 MODE;
} ICEMKII_REQ_FORCED_STOP_t, * ICEMKII_REQ_FORCED_STOP_p;

/*----------------------------------------------------------------------------*/

typedef __packed struct ICEMKII_ISP_s
{
  U8 packet[4];
} ICEMKII_ISP_t, * ICEMKII_ISP_p;

/*----------------------------------------------------------------------------*/

typedef __packed struct ICEMKII_MSG_BODY_s
{
  U8 MESSAGE_ID;
  union
  {
    ICEMKII_RSP_SIGN_ON_t       rspSignOn;
    ICEMKII_REQ_SET_PARAMETER_t reqSetParameter;
    ICEMKII_REQ_RESET_t         reqReset;
    ICEMKII_ISP_t               isp;
    ICEMKII_REQ_GET_PARAMETER_t reqGetParameter;
    ICEMKII_RSP_GET_PARAMETER_t rspGetParameter;
    ICEMKII_REQ_FORCED_STOP_t   forcedStop;
  };
} ICEMKII_MSG_BODY_t, * ICEMKII_MSG_BODY_p;

/*----------------------------------------------------------------------------*/

static void icemkii_SignOn(U8 * pRspBody, U32 * pSize)
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

static void icemkii_SignOff(U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;

  ICEMKII_LOG("ICE Rx: Sign Off\r\n");

  rsp->MESSAGE_ID = RSP_OK;
  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

static void icemkii_SetParameter(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p req = (ICEMKII_MSG_BODY_p)pReqBody;
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;

  ICEMKII_LOG("ICE Rx: Set Parameter\r\n");
  rsp->MESSAGE_ID = RSP_OK;

  switch (req->reqSetParameter.PARAMETER_ID)
  {
    case PARAMETER_ID_EMULATOR_MODE:
      if ( (EMULATOR_MODE_SPI      != req->reqSetParameter.emulator.mode) &&
           (EMULATOR_MODE_DBG_WIRE != req->reqSetParameter.emulator.mode) )
      {
        rsp->MESSAGE_ID = RSP_FAILED;
      }
      break;
    case PARAMETER_ID_RUN_AFTER_PROG:
    case PARAMETER_ID_TIMERS_UNDER_DBG:
    case PARAMETER_ID_PDI_APPL_OFFS:
    case PARAMETER_ID_PDI_BOOT_OFFS:
    case PARAMETER_ID_UNKNOWN:
      break;
    default:
      rsp->MESSAGE_ID = RSP_FAILED;
      break;
  }

  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

static void icemkii_GetParameter(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p req = (ICEMKII_MSG_BODY_p)pReqBody;
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;
  U16 sign = 0x9205;

  ICEMKII_LOG("ICE Rx: Get Parameter\r\n");

  rsp->MESSAGE_ID = RSP_PARAMETER;

  switch (req->reqGetParameter.PARAMETER_ID)
  {
    case PARAMETER_ID_OCD_VTARGET:
      rsp->rspGetParameter.ocdVtarget.voltage = 3300;
      *pSize = 3;
      break;
    case PARAMETER_ID_TARGET_SIGNATURE:
      rsp->rspGetParameter.targetSign.value = sign;
      *pSize = 3;
      break;
    default:
      rsp->MESSAGE_ID = RSP_FAILED;
      *pSize = 1;
      break;
  }
}

/*----------------------------------------------------------------------------*/

static void icemkii_IspPacket(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p req = (ICEMKII_MSG_BODY_p)pReqBody;
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;

  ICEMKII_LOG("ICE Rx: ISP Packet\r\n");

  *pSize -= 1;

  if (FW_FALSE == ISPMKII_Process(req->isp.packet, rsp->isp.packet, pSize))
  {
    rsp->MESSAGE_ID = RSP_FAILED;
    *pSize = 1;
  }
  else
  {
    rsp->MESSAGE_ID = RSP_SPI_DATA;
    *pSize += 1;
  }
}

/*----------------------------------------------------------------------------*/

static void icemkii_Reset(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p req = (ICEMKII_MSG_BODY_p)pReqBody;
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;

  ICEMKII_LOG("ICE Rx: Reset\r\n");
  rsp->MESSAGE_ID = RSP_FAILED;

  switch (req->reqReset.FLAG)
  {
    case RESET_FLAG_LOW_LEVEL:
    case RESET_FLAG_HIGH_LEVEL:
      rsp->MESSAGE_ID = RSP_OK;
      break;
    default:
      break;
  }

  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

static void icemkii_Go(U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;

  ICEMKII_LOG("ICE Rx: Go\r\n");

  rsp->MESSAGE_ID = RSP_OK;
  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

static void icemkii_SetDvcDescr(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p req = (ICEMKII_MSG_BODY_p)pReqBody;
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;

  ICEMKII_LOG("ICE Rx: Set Device Descriptor: Rcvd = %d, Fact = %d\r\n",
              (*pSize - 1), sizeof(DEVICE_DESCRIPTOR_t));

  rsp->MESSAGE_ID = RSP_OK;
  *pSize = 1;
}

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
    case CMND_GET_PARAMETER:
      icemkii_GetParameter(pReqBody, pRspBody, pSize);
      break;
    case CMND_SET_DEVICE_DESCRIPTOR:
      icemkii_SetDvcDescr(pReqBody, pRspBody, pSize);
      break;
    case CMND_FORCED_STOP:
    case CMND_WRITE_MEMORY:
    case CMND_CLEAR_EVENTS:
      pRspBody[0] = RSP_OK;
      *pSize = 1;
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