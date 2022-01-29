#include <string.h>
#include "icemkii_processor.h"
#include "ispmkii_processor.h"
#include "dwire_processor.h"
#include "debug.h"

#define ICEMKII_DEBUG

#ifdef ICEMKII_DEBUG
#  define ICEMKII_LOG    DBG
#else
#  define ICEMKII_LOG(...)
#endif
//#define ISP_LOG_FUNC_START()      ISP_LOG("--- ISP Start ----------------\r\n")
//#define ISP_LOG_FUNC_END()        ISP_LOG("--- ISP End ------------------\r\n")
//#define ISP_LOG_SUCCESS()         ISP_LOG(" - Success\r\n")
//#define ISP_LOG_ERROR()           ISP_LOG(" - Fail\r\n")

/*----------------------------------------------------------------------------*/

typedef __packed struct DEVICE_DESCRIPTOR_s
{
  U8  ucReadIO[8]; //LSB = IOloc 0, MSB = IOloc63
  U8  ucReadIOShadow[8]; //LSB = IOloc 0, MSB = IOloc63
  U8  ucWriteIO[8]; //LSB = IOloc 0, MSB = IOloc63
  U8  ucWriteIOShadow[8]; //LSB = IOloc 0, MSB = IOloc63
  U8  ucReadExtIO[52]; //LSB = IOloc 96, MSB = IOloc511
  U8  ucReadIOExtShadow[52]; //LSB = IOloc 96, MSB = IOloc511
  U8  ucWriteExtIO[52]; //LSB = IOloc 96, MSB = IOloc511
  U8  ucWriteIOExtShadow[52];//LSB = IOloc 96, MSB = IOloc511
  U8  ucIDRAddress; //IDR address
  U8  ucSPMCRAddress; //SPMCR Register address and dW BasePC
  U32 ulBootAddress; //Device Boot Loader Start Address
  U8  ucRAMPZAddress; //RAMPZ Register address in SRAM I/O space
  U16 uiFlashPageSize; //Device Flash Page Size, Size = 2 exp ucFlashPageSize
  U8  ucEepromPageSize; //Device Eeprom Page Size in bytes
  U16 uiUpperExtIOLoc; //Topmost (last) extended I/O location, 0 if no external I/O
  U32 ulFlashSize; //Device Flash Size
  U8  ucEepromInst[20]; //Instructions for W/R EEPROM
  U8  ucFlashInst[3]; //Instructions for W/R FLASH
  U8  ucSPHaddr; // Stack pointer high
  U8  ucSPLaddr; // Stack pointer low
  U16 uiFlashpages; // number of pages in flash
  U8  ucDWDRAddress; // DWDR register address
  U8  ucDWBasePC; // Base/mask value of the PC
  U8  ucAllowFullPageBitstream; // FALSE on ALL new parts
  U16 uiStartSmallestBootLoaderSection; //
  U8  EnablePageProgramming; // For JTAG parts only, default TRUE
  U8  ucCacheType; // CacheType_Normal 0x00, CacheType_CAN 0x01, CacheType_HEIMDALL 0x02
  U16 uiSramStartAddr; // Start of SRAM
  U8  ucResetType; // Selects reset type. 0x00
  U8  ucPCMaskExtended; // For parts with extended PC
  U8  ucPCMaskHigh; // PC high mask
  U8  ucEindAddress; // EIND IO address
  U16 EECRAddress; // EECR IO address
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
#define CMND_READ_MEMORY               (0x05)
#define CMND_READ_PC                   (0x07)
#define CMND_RESTORE_TARGET            (0x23)
#define CMND_SINGLE_STEP               (0x09)

#define RSP_OK                         (0x80)
#define RSP_PARAMETER                  (0x81)
#define RSP_SIGN_ON                    (0x86)
#define RSP_SPI_DATA                   (0x88)
#define RSP_FAILED                     (0xA0)
#define RSP_MEMORY                     (0x82)
#define RSP_PC                         (0x84)

#define EVT_BREAK                      (0xE0)

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
    U8                                    PARAMETER_VALUE[1];
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
    U8                                    PARAMETER_VALUE[1];
    ICEMKII_GET_PARAMETER_OCD_VTARGET_t   ocdVtarget;
    ICEMKII_GET_PARAMETER_TGT_SIGN_t      targetSign;
  };
} ICEMKII_RSP_GET_PARAMETER_t, * ICEMKII_RSP_GET_PARAMETER_p;

/*----------------------------------------------------------------------------*/

#define RESET_FLAG_LOW_LEVEL     (0x01) // Low level
#define RESET_FLAG_HIGH_LEVEL    (0x02) // High level (reset, then run to main)
#define RESET_FLAG_DBG_WIRE_DSBL (0x04) // Reset with debugWire disable

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

#define BREAK_CAUSE_UNSPECIFIED       (0x00)
#define BREAK_CAUSE_PROGRAM           (0x01)
#define BREAK_CAUSE_DATA_PDSB         (0x02)
#define BREAK_CAUSE_DATA_PDMSB        (0x03)

typedef __packed struct ICEMKII_EVT_BREAK_s
{
  U32 PROGRAM_COUNTER;
  U8  BREAK_CAUSE;
} ICEMKII_EVT_BREAK_t, * ICEMKII_EVT_BREAK_p;

/*----------------------------------------------------------------------------*/

typedef __packed struct ICEMKII_ISP_s
{
  U8 packet[4];
} ICEMKII_ISP_t, * ICEMKII_ISP_p;

/*----------------------------------------------------------------------------*/

#define MEMORY_TYPE_IO_SHADOW                    (0x30)
#define MEMORY_TYPE_SRAM                         (0x20)
#define MEMORY_TYPE_EEPROM                       (0x22)
#define MEMORY_TYPE_EVENT                        (0x60)
#define MEMORY_TYPE_SPM                          (0xA0)
#define MEMORY_TYPE_FLASH_PAGE                   (0xB0)
#define MEMORY_TYPE_EEPROM_PAGE                  (0xB1)
#define MEMORY_TYPE_FUSE_BITS                    (0xB2)
#define MEMORY_TYPE_LOCK_BITS                    (0xB3)
#define MEMORY_TYPE_SIGN_JTAG                    (0xB4)
#define MEMORY_TYPE_OSCCAL_BYTE                  (0xB5)
#define MEMORY_TYPE_CAN                          (0xB6)
#define MEMORY_TYPE_XMEGA_APPLICATION_FLASH      (0xC0)
#define MEMORY_TYPE_XMEGA_BOOT_FLASH             (0xC1)
#define MEMORY_TYPE_XMEGA_USER_SIGNATURE         (0xC5)
#define MEMORY_TYPE_XMEGA_CALIBRATION_SIGNATURE  (0xC6)

typedef __packed struct ICEMKII_REQ_READ_MEMORY_s
{
  U8  MEMORY_TYPE;
  U32 BYTE_COUNT;
  U32 START_ADDRESS;
} ICEMKII_REQ_READ_MEMORY_t, * ICEMKII_REQ_READ_MEMORY_p;

typedef __packed struct ICEMKII_RSP_MEMORY_s
{
  U8 DATA[1];
} ICEMKII_RSP_MEMORY_t, * ICEMKII_RSP_MEMORY_p;

/*----------------------------------------------------------------------------*/

typedef __packed struct ICEMKII_RSP_PC_s
{
  U32 PROGRAM_COUNTER;
} ICEMKII_RSP_PC_t, * ICEMKII_RSP_PC_p;

/*----------------------------------------------------------------------------*/

#define SINGLE_STEP_LOW_LEVEL                    (0x01)
#define SINGLE_STEP_HIGH_LEVEL                   (0x02)

#define SINGLE_STEP_OVER                         (0x00)
#define SINGLE_STEP_INTO                         (0x01)
#define SINGLE_STEP_OUT                          (0x02)

typedef __packed struct ICEMKII_REQ_SINGLE_STEP_s
{
  U8 FLAG;
  U8 STEP_MODE;
} ICEMKII_REQ_SINGLE_STEP_t, * ICEMKII_REQ_SINGLE_STEP_p;

/*----------------------------------------------------------------------------*/

typedef __packed struct ICEMKII_MSG_BODY_s
{
  U8 MESSAGE_ID;
  union
  {
    ICEMKII_REQ_SET_PARAMETER_t reqSetParameter;
    ICEMKII_REQ_RESET_t         reqReset;
    ICEMKII_REQ_GET_PARAMETER_t reqGetParameter;
    ICEMKII_REQ_FORCED_STOP_t   reqForcedStop;
    ICEMKII_REQ_READ_MEMORY_t   reqReadMemory;
    ICEMKII_ISP_t               isp;
    ICEMKII_RSP_SIGN_ON_t       rspSignOn;
    ICEMKII_RSP_GET_PARAMETER_t rspGetParameter;
    ICEMKII_RSP_MEMORY_t        rspMemory;
    ICEMKII_RSP_PC_t            rspPc;
    ICEMKII_EVT_BREAK_t         evtBreak;
  };
} ICEMKII_MSG_BODY_t, * ICEMKII_MSG_BODY_p;

/*----------------------------------------------------------------------------*/

static void icemkii_SignOn
(
  ICEMKII_MSG_BODY_p pReq,
  ICEMKII_MSG_BODY_p pRsp,
  U32 * pSize
)
{
  U8 sn[6] = ICEMKII_SN;

  ICEMKII_LOG("ICE Rx: Sign On\r\n");

  pRsp->MESSAGE_ID             = RSP_SIGN_ON;
  pRsp->rspSignOn.COMM_ID      = ICEMKII_COMM_ID;
  pRsp->rspSignOn.M_MCU_BLDR   = ICEMKII_BLDR_FW;
  pRsp->rspSignOn.M_MCU_FW_MIN = ICEMKII_MCU_FW_MIN;
  pRsp->rspSignOn.M_MCU_FW_MAJ = ICEMKII_MCU_FW_MAJ;
  pRsp->rspSignOn.M_MCU_HW     = 0x00;
  pRsp->rspSignOn.S_MCU_BLDR   = ICEMKII_BLDR_FW;
  pRsp->rspSignOn.S_MCU_FW_MIN = ICEMKII_MCU_FW_MIN;
  pRsp->rspSignOn.S_MCU_FW_MAJ = ICEMKII_MCU_FW_MAJ;
  pRsp->rspSignOn.S_MCU_HW     = 0x01;
  memcpy(pRsp->rspSignOn.SERIAL_NUMBER, sn, 6);
  memcpy(pRsp->rspSignOn.DEVICE_ID_STR, ICEMKII_ID_STR, 12);

  *pSize = (sizeof(ICEMKII_RSP_SIGN_ON_t) + 1);
}

/*----------------------------------------------------------------------------*/

static void icemkii_SignOff
(
  ICEMKII_MSG_BODY_p pReq,
  ICEMKII_MSG_BODY_p pRsp,
  U32 * pSize
)
{
  ICEMKII_LOG("ICE Rx: Sign Off\r\n");

  pRsp->MESSAGE_ID = RSP_OK;
  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

static void icemkii_SetParameter
(
  ICEMKII_MSG_BODY_p pReq,
  ICEMKII_MSG_BODY_p pRsp,
  U32 * pSize
)
{
  ICEMKII_LOG("ICE Rx: Set Parameter\r\n");
  pRsp->MESSAGE_ID = RSP_OK;

  switch (pReq->reqSetParameter.PARAMETER_ID)
  {
    case PARAMETER_ID_EMULATOR_MODE:
      ICEMKII_LOG(" - Emulator mode\r\n");
      if (EMULATOR_MODE_SPI == pReq->reqSetParameter.emulator.mode)
      {
        //
      }
      else if (EMULATOR_MODE_DBG_WIRE == pReq->reqSetParameter.emulator.mode)
      {
        DWire_Init();
        if (FW_FALSE == DWire_Sync())
        {
          pRsp->MESSAGE_ID = RSP_FAILED;
        }
      }
      else
      {
        pRsp->MESSAGE_ID = RSP_FAILED;
      }
      break;
    case PARAMETER_ID_RUN_AFTER_PROG:
      ICEMKII_LOG(" - Run after programming\r\n");
      break;
    case PARAMETER_ID_TIMERS_UNDER_DBG:
      ICEMKII_LOG(" - Timers under debug\r\n");
      break;
    case PARAMETER_ID_PDI_APPL_OFFS:
      ICEMKII_LOG(" - PDI application offset\r\n");
      break;
    case PARAMETER_ID_PDI_BOOT_OFFS:
      ICEMKII_LOG(" - PDI bootloader offset\r\n");
      break;
    case PARAMETER_ID_UNKNOWN:
      ICEMKII_LOG(" - Unknown\r\n");
      break;
    default:
      ICEMKII_LOG(" - Undefined\r\n");
      pRsp->MESSAGE_ID = RSP_FAILED;
      break;
  }

  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

static void icemkii_GetParameter
(
  ICEMKII_MSG_BODY_p pReq,
  ICEMKII_MSG_BODY_p pRsp,
  U32 * pSize
)
{
  U16 sign = 0x9205;

  ICEMKII_LOG("ICE Rx: Get Parameter\r\n");

  pRsp->MESSAGE_ID = RSP_PARAMETER;

  switch (pReq->reqGetParameter.PARAMETER_ID)
  {
    case PARAMETER_ID_OCD_VTARGET:
      ICEMKII_LOG(" - OCD Vtarget\r\n");
      pRsp->rspGetParameter.ocdVtarget.voltage = 3300;
      *pSize = 3;
      break;
    case PARAMETER_ID_TARGET_SIGNATURE:
      ICEMKII_LOG(" - Target signature\r\n");
      pRsp->rspGetParameter.targetSign.value = sign;
      *pSize = 3;
      break;
    default:
      ICEMKII_LOG(" - Undefined\r\n");
      pRsp->MESSAGE_ID = RSP_FAILED;
      *pSize = 1;
      break;
  }
}

/*----------------------------------------------------------------------------*/

static void icemkii_IspPacket
(
  ICEMKII_MSG_BODY_p pReq,
  ICEMKII_MSG_BODY_p pRsp,
  U32 * pSize
)
{
  ICEMKII_LOG("ICE Rx: ISP Packet\r\n");

  *pSize -= 1;

  if (FW_FALSE == ISPMKII_Process(pReq->isp.packet, pRsp->isp.packet, pSize))
  {
    pRsp->MESSAGE_ID = RSP_FAILED;
    *pSize = 1;
  }
  else
  {
    pRsp->MESSAGE_ID = RSP_SPI_DATA;
    *pSize += 1;
  }
}

/*----------------------------------------------------------------------------*/

static void icemkii_Reset
(
  ICEMKII_MSG_BODY_p pReq,
  ICEMKII_MSG_BODY_p pRsp,
  U32 * pSize
)
{
  ICEMKII_LOG("ICE Rx: Reset\r\n");
  pRsp->MESSAGE_ID = RSP_FAILED;

  switch (pReq->reqReset.FLAG)
  {
    case RESET_FLAG_LOW_LEVEL:
    case RESET_FLAG_HIGH_LEVEL:
      pRsp->MESSAGE_ID = RSP_OK;
      break;
    default:
      break;
  }

  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

static void icemkii_Go
(
  ICEMKII_MSG_BODY_p pReq,
  ICEMKII_MSG_BODY_p pRsp,
  U32 * pSize
)
{
  ICEMKII_LOG("ICE Rx: Go\r\n");

  pRsp->MESSAGE_ID = RSP_OK;
  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

static void icemkii_SetDvcDescr
(
  ICEMKII_MSG_BODY_p pReq,
  ICEMKII_MSG_BODY_p pRsp,
  U32 * pSize
)
{
  ICEMKII_LOG("ICE Rx: Set Device Descriptor:\r\n");
  ICEMKII_LOG(" Rcvd  = %d\r\n", (*pSize - 1));
  ICEMKII_LOG(" Fact  = %d\r\n", sizeof(DEVICE_DESCRIPTOR_t));

  DEVICE_DESCRIPTOR_t * dsc = (DEVICE_DESCRIPTOR_t *)(pReq + 1);

  ICEMKII_LOG(" ucReadIO[0]                      = %08X\r\n", dsc->ucReadIO[0]);
  ICEMKII_LOG(" ucReadIOShadow[0]                = %08X\r\n", dsc->ucReadIOShadow[0]);
  ICEMKII_LOG(" ucWriteIO[0]                     = %08X\r\n", dsc->ucWriteIO[0]);
  ICEMKII_LOG(" ucWriteIOShadow[0]               = %08X\r\n", dsc->ucWriteIOShadow[0]);
  ICEMKII_LOG(" ucReadExtIO[0]                   = %08X\r\n", dsc->ucReadExtIO[0]);
  ICEMKII_LOG(" ucReadIOExtShadow[0]             = %08X\r\n", dsc->ucReadIOExtShadow[0]);
  ICEMKII_LOG(" ucWriteExtIO[0]                  = %08X\r\n", dsc->ucWriteExtIO[0]);
  ICEMKII_LOG(" ucWriteIOExtShadow[0]            = %08X\r\n", dsc->ucWriteIOExtShadow[0]);
  ICEMKII_LOG(" ucIDRAddress                     = %08X\r\n", dsc->ucIDRAddress);
  ICEMKII_LOG(" ucSPMCRAddress                   = %08X\r\n", dsc->ucSPMCRAddress);
  ICEMKII_LOG(" ulBootAddress                    = %08X\r\n", dsc->ulBootAddress);
  ICEMKII_LOG(" ucRAMPZAddress                   = %08X\r\n", dsc->ucRAMPZAddress);
  ICEMKII_LOG(" uiFlashPageSize                  = %08X\r\n", dsc->uiFlashPageSize);
  ICEMKII_LOG(" ucEepromPageSize                 = %08X\r\n", dsc->ucEepromPageSize);
  ICEMKII_LOG(" uiUpperExtIOLoc                  = %08X\r\n", dsc->uiUpperExtIOLoc);
  ICEMKII_LOG(" ulFlashSize                      = %08X\r\n", dsc->ulFlashSize);
  ICEMKII_LOG(" ucEepromInst[0]                  = %08X\r\n", dsc->ucEepromInst[0]);
  ICEMKII_LOG(" ucFlashInst[0]                   = %08X\r\n", dsc->ucFlashInst[0]);
  ICEMKII_LOG(" ucSPHaddr                        = %08X\r\n", dsc->ucSPHaddr);
  ICEMKII_LOG(" ucSPLaddr                        = %08X\r\n", dsc->ucSPLaddr);
  ICEMKII_LOG(" uiFlashpages                     = %08X\r\n", dsc->uiFlashpages);
  ICEMKII_LOG(" ucDWDRAddress                    = %08X\r\n", dsc->ucDWDRAddress);
  ICEMKII_LOG(" ucDWBasePC                       = %08X\r\n", dsc->ucDWBasePC);
  ICEMKII_LOG(" ucAllowFullPageBitstream         = %08X\r\n", dsc->ucAllowFullPageBitstream);
  ICEMKII_LOG(" uiStartSmallestBootLoaderSection = %08X\r\n", dsc->uiStartSmallestBootLoaderSection);
  ICEMKII_LOG(" EnablePageProgramming            = %08X\r\n", dsc->EnablePageProgramming);
  ICEMKII_LOG(" ucCacheType                      = %08X\r\n", dsc->ucCacheType);
  ICEMKII_LOG(" uiSramStartAddr                  = %08X\r\n", dsc->uiSramStartAddr);
  ICEMKII_LOG(" ucResetType                      = %08X\r\n", dsc->ucResetType);
  ICEMKII_LOG(" ucPCMaskExtended                 = %08X\r\n", dsc->ucPCMaskExtended);
  ICEMKII_LOG(" ucPCMaskHigh                     = %08X\r\n", dsc->ucPCMaskHigh);
  ICEMKII_LOG(" ucEindAddress                    = %08X\r\n", dsc->ucEindAddress);
  ICEMKII_LOG(" EECRAddress                      = %08X\r\n", dsc->EECRAddress);

  pRsp->MESSAGE_ID = RSP_OK;
  *pSize = 1;
}

/*----------------------------------------------------------------------------*/

static void icemkii_ReadMemory
(
  ICEMKII_MSG_BODY_p pReq,
  ICEMKII_MSG_BODY_p pRsp,
  U32 * pSize
)
{
  ICEMKII_LOG("ICE Rx: Read Memory\r\n");
  pRsp->MESSAGE_ID = RSP_FAILED;

  switch (pReq->reqReadMemory.MEMORY_TYPE)
  {
    case MEMORY_TYPE_IO_SHADOW:
    case MEMORY_TYPE_SRAM:
    case MEMORY_TYPE_EEPROM:
    case MEMORY_TYPE_EVENT:
    case MEMORY_TYPE_SPM:
    case MEMORY_TYPE_FLASH_PAGE:
    case MEMORY_TYPE_EEPROM_PAGE:
    case MEMORY_TYPE_FUSE_BITS:
    case MEMORY_TYPE_LOCK_BITS:
    case MEMORY_TYPE_SIGN_JTAG:
    case MEMORY_TYPE_OSCCAL_BYTE:
    case MEMORY_TYPE_CAN:
    case MEMORY_TYPE_XMEGA_APPLICATION_FLASH:
    case MEMORY_TYPE_XMEGA_BOOT_FLASH:
    case MEMORY_TYPE_XMEGA_USER_SIGNATURE:
    case MEMORY_TYPE_XMEGA_CALIBRATION_SIGNATURE:
      pRsp->MESSAGE_ID = RSP_MEMORY;
      memset(pRsp->rspMemory.DATA, 0, pReq->reqReadMemory.BYTE_COUNT);
      *pSize = (1 + pReq->reqReadMemory.BYTE_COUNT);
      break;
    default:
      *pSize = 1;
      break;
  }
}

/*----------------------------------------------------------------------------*/

static void icemkii_ReadPc
(
  ICEMKII_MSG_BODY_p pReq,
  ICEMKII_MSG_BODY_p pRsp,
  U32 * pSize
)
{
  ICEMKII_LOG("ICE Rx: Read PC\r\n");

  pRsp->MESSAGE_ID = RSP_PC;
  pRsp->rspPc.PROGRAM_COUNTER = 0;
  *pSize = 5;
}

/*----------------------------------------------------------------------------*/

void ICEMKII_Process(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  ICEMKII_MSG_BODY_p req = (ICEMKII_MSG_BODY_p)pReqBody;
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;
  U32 i = 0;

  ICEMKII_LOG("---------------------------------------------------------\r\n");
  ICEMKII_LOG("--> ");
  for (i = 0; i < *pSize; i++)
  {
    ICEMKII_LOG("%02X ", pReqBody[i]);
  }
  ICEMKII_LOG("\r\n");


  switch ( ((ICEMKII_MSG_BODY_p)pReqBody)->MESSAGE_ID )
  {
    case CMND_GET_SIGN_ON:
      icemkii_SignOn(req, rsp, pSize);
      break;
    case CMND_SIGN_OFF:
      icemkii_SignOff(req, rsp, pSize);
      break;
    case CMND_SET_PARAMETER:
      icemkii_SetParameter(req, rsp, pSize);
      break;
    case CMND_ISP_PACKET:
      icemkii_IspPacket(req, rsp, pSize);
      break;
    case CMND_RESET:
      icemkii_Reset(req, rsp, pSize);
      break;
    case CMND_GET_PARAMETER:
      icemkii_GetParameter(req, rsp, pSize);
      break;
    case CMND_GO:
      icemkii_Go(req, rsp, pSize);
      break;
    case CMND_SET_DEVICE_DESCRIPTOR:
      icemkii_SetDvcDescr(req, rsp, pSize);
      break;
    case CMND_READ_MEMORY:
      icemkii_ReadMemory(req, rsp, pSize);
      break;
    case CMND_READ_PC:
      icemkii_ReadPc(req, rsp, pSize);
      break;
    case CMND_FORCED_STOP:
    case CMND_WRITE_MEMORY:
    case CMND_CLEAR_EVENTS:
    case CMND_RESTORE_TARGET:
    case CMND_SINGLE_STEP:
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







/*----------------------------------------------------------------------------*/

static void icemkii_Event
(
  ICEMKII_MSG_BODY_p pReq,
  ICEMKII_MSG_BODY_p pRsp,
  U32 * pSize
)
{
  ICEMKII_LOG("ICE Rx: Break Event\r\n");

  pRsp->MESSAGE_ID               = EVT_BREAK;
  pRsp->evtBreak.PROGRAM_COUNTER = 0;
  pRsp->evtBreak.BREAK_CAUSE     = BREAK_CAUSE_PROGRAM;
  *pSize = 6;
}

/*----------------------------------------------------------------------------*/

FW_BOOLEAN ICEMKII_CheckForEvents(U8 * pReqBody, U8 * pRspBody, U32 * pSize)
{
  FW_BOOLEAN result = FW_FALSE;
  ICEMKII_MSG_BODY_p req = (ICEMKII_MSG_BODY_p)pReqBody;
  ICEMKII_MSG_BODY_p rsp = (ICEMKII_MSG_BODY_p)pRspBody;
  U32 i = 0;

  switch ( ((ICEMKII_MSG_BODY_p)pReqBody)->MESSAGE_ID )
  {
    case CMND_FORCED_STOP:
    case CMND_SINGLE_STEP:
      icemkii_Event(req, rsp, pSize);
      result = FW_TRUE;
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

  return result;
}

/*----------------------------------------------------------------------------*/