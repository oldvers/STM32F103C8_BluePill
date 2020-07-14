#include "types.h"

#include "usb.h"
#include "usb_cfg.h"
#include "usb_defs.h"
#include "usb_core.h"
#include "cdc_defs.h"
#include "cdc.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "fifo.h"

#include "debug.h"
#include "string.h"

#include "gpio.h"
#include "uart.h"
#include "board.h"
#include "interrupts.h"

//-----------------------------------------------------------------------------
/* Private definitions */

//#define CDC_CTRL_LINE_STATE_DTR   (0)
//#define CDC_CTRL_LINE_STATE_RTS   (1)

#define CDC_DEBUG

#ifdef CDC_DEBUG
#  define CDC_LOG(...)      printf(__VA_ARGS__)
#else
#  define CDC_LOG(...)
#endif

//-----------------------------------------------------------------------------
/* Private Types definitions */

#define USB_CDC_FIFO_SIZE              (USB_CDC_PACKET_SIZE * 8)

//-----------------------------------------------------------------------------
/* Vendor Specific Requests */
#define CDC_REQ_IFC_ENABLE                                 (0x00)
#define CDC_REQ_SET_BAUDDIV                                (0x01)
#define CDC_REQ_GET_BAUDDIV                                (0x02)
#define CDC_REQ_SET_LINE_CTL                               (0x03)
#define CDC_REQ_GET_LINE_CTL                               (0x04)
#define CDC_REQ_SET_BREAK                                  (0x05)
#define CDC_REQ_IMM_CHAR                                   (0x06)
#define CDC_REQ_SET_MHS                                    (0x07)
#define CDC_REQ_GET_MDMSTS                                 (0x08)
#define CDC_REQ_SET_XON                                    (0x09)
#define CDC_REQ_SET_XOFF                                   (0x0A)
#define CDC_REQ_SET_EVENTMASK                              (0x0B)
#define CDC_REQ_GET_EVENTMASK                              (0x0C)
#define CDC_REQ_GET_EVENTSTATE                             (0x16)
#define CDC_REQ_SET_CHAR                                   (0x0D)
#define CDC_REQ_GET_CHARS                                  (0x0E)
#define CDC_REQ_GET_PROPS                                  (0x0F)
#define CDC_REQ_GET_COMM_STATUS                            (0x10)
#define CDC_REQ_RESET                                      (0x11)
#define CDC_REQ_PURGE                                      (0x12)
#define CDC_REQ_SET_FLOW                                   (0x13)
#define CDC_REQ_GET_FLOW                                   (0x14)
#define CDC_REQ_EMBED_EVENTS                               (0x15)
#define CDC_REQ_GET_BAUDRATE                               (0x1D)
#define CDC_REQ_SET_BAUDRATE                               (0x1E)
#define CDC_REQ_SET_CHARS                                  (0x19)
#define CDC_REQ_VENDOR_SPECIFIC                            (0xFF)

//-----------------------------------------------------------------------------
/* Communication Properties Response */

/* Version of response in BCD: 0x0100 is Version 1.00 */
#define COMM_PROP_RSP_VERSION                              (0x0100)
/* Service provider identifier - 0x00000001 */
#define COMM_PROP_RSP_SVC_ID                               (0x00000001)
/* Kind of Device */
#define COMM_PROP_RSP_PROV_SUB_TYPE_UNSPECIFIED            (0)
#define COMM_PROP_RSP_PROV_SUB_TYPE_RS232                  (1)
#define COMM_PROP_RSP_PROV_SUB_TYPE_MODEM                  (6)
/* Capabilities Mask */
#define COMM_PROP_RSP_PROV_CAPABS_DTR_DSR                  (1 << 0)
#define COMM_PROP_RSP_PROV_CAPABS_RTS_CTS                  (1 << 1)
#define COMM_PROP_RSP_PROV_CAPABS_DCD                      (1 << 2)
#define COMM_PROP_RSP_PROV_CAPABS_CHECK_PARITY             (1 << 3)
#define COMM_PROP_RSP_PROV_CAPABS_XON_XOFF                 (1 << 4)
#define COMM_PROP_RSP_PROV_CAPABS_XOxx_CHARS               (1 << 5)
#define COMM_PROP_RSP_PROV_CAPABS_SPEC_CHARS               (1 << 8)
/* Settable parameters mask */
#define COMM_PROP_RSP_SETBL_PARAM_PARITY_TYPE              (1 << 0)
#define COMM_PROP_RSP_SETBL_PARAM_BAUD                     (1 << 1)
#define COMM_PROP_RSP_SETBL_PARAM_DATA_BITS                (1 << 2)
#define COMM_PROP_RSP_SETBL_PARAM_STOP_BITS                (1 << 3)
#define COMM_PROP_RSP_SETBL_PARAM_HANDSHAKE                (1 << 4)
#define COMM_PROP_RSP_SETBL_PARAM_PARITY                   (1 << 5)
#define COMM_PROP_RSP_SETBL_PARAM_CARRIER_DET              (1 << 6)
/* Settable baud rates mask */
#define COMM_PROP_RSP_BAUD_75                              (1 << 0)
#define COMM_PROP_RSP_BAUD_110                             (1 << 1)
#define COMM_PROP_RSP_BAUD_134                             (1 << 2)
#define COMM_PROP_RSP_BAUD_150                             (1 << 3)
#define COMM_PROP_RSP_BAUD_300                             (1 << 4)
#define COMM_PROP_RSP_BAUD_600                             (1 << 5)
#define COMM_PROP_RSP_BAUD_1200                            (1 << 6)
#define COMM_PROP_RSP_BAUD_1800                            (1 << 7)
#define COMM_PROP_RSP_BAUD_2400                            (1 << 8)
#define COMM_PROP_RSP_BAUD_4800                            (1 << 9)
#define COMM_PROP_RSP_BAUD_7200                            (1 << 10)
#define COMM_PROP_RSP_BAUD_9600                            (1 << 11)
#define COMM_PROP_RSP_BAUD_14400                           (1 << 12)
#define COMM_PROP_RSP_BAUD_19200                           (1 << 13)
#define COMM_PROP_RSP_BAUD_38400                           (1 << 14)
#define COMM_PROP_RSP_BAUD_56000                           (1 << 15)
#define COMM_PROP_RSP_BAUD_128000                          (1 << 16)
#define COMM_PROP_RSP_BAUD_115200                          (1 << 17)
#define COMM_PROP_RSP_BAUD_57600                           (1 << 18)
#define COMM_PROP_RSP_BAUD_OTHER                           (1 << 28)
/* Capabilities mask for permissible data bit settings */
#define COMM_PROP_RSP_SETBL_DATA_5                         (1 << 0)
#define COMM_PROP_RSP_SETBL_DATA_6                         (1 << 1)
#define COMM_PROP_RSP_SETBL_DATA_7                         (1 << 2)
#define COMM_PROP_RSP_SETBL_DATA_8                         (1 << 3)
#define COMM_PROP_RSP_SETBL_DATA_16                        (1 << 4)
#define COMM_PROP_RSP_SETBL_DATA_16EX                      (1 << 5)

typedef __packed struct COMM_PROP_RSP_S
{
  /* Size of structure in bytes. This must reflect the total available size,
     even if the host requests fewer bytes */
  U16 wLength;
  /* BCD: 0x0100 Version of response, in BCD: 0x0100 is Version 1.00 */
  U16 bcdVersion;
  /* Service provider identifier - Number: 0x00000001 for compatbility
     with NT serial.sys */
  U32 ulServiceMask;
  U32 reserved1;
  /* Number Maximum transmit queue size */
  U32 ulMaxTxQueue;
  /* Number Maximum receive queue size */
  U32 ulMaxRxQueue;
  /* Number Maximum baud rate */
  U32 ulMaxBaud;
  /* Indicates kind of device:
       0 - Unspecified
       1 - RS-232
       6 - Modem or TA
     All other values are reserved */
  U32 ulProvSubType;
  /* BitMask - Capabilities Mask. Bits are:
       0: DTR/DSR support
       1: RTS/CTS support
       2: DCD support
       3: Can check parity
       4: XON/XOFF support
       5: Can set XON/XOFF characters
       6: reserved
       7: reserved
       8: Can set special characters
       9: Supports 16-bit mode (always 0)
     All other bits are reserved */
  U32 ulProvCapabilities;
  /* BitMask - Settable parameters mask. Bits are: 
       0: Can set parity type
       1: Can set baud
       2: Can set number of data bits
       3: Can set stop-bits
       4: Can set handshaking
       5: Can set parity checking
       6: Can set carrier-detect checking
     All other bits are reserved */
  U32 ulSettableParams;
  /* BitMask - Settable baud rates mask. Bits are: 
       0 : 75 baud
       1 : 110 baud
       2 : 134.5 baud
       3 : 150 baud
       4 : 300 baud
       5 : 600 baud
       6 : 1200 baud
       7 : 1800 baud
       8 : 2400 baud
       9 : 4800 baud
       10: 7200 baud
       11: 9600 baud
       12: 14,400 baud
       13: 19,200 baud
       14: 38,400 baud
       15: 56,000 baud
       16: 128,000 baud
       17: 115,200 baud
       18: 57,600 baud
       19-27: reserved
       28: the CP210x supports additional baud rates other than those defined
           by bits 0-18. (There is no way to determine what these baud rates
           are, other than by trying to select them.)
       29-31: reserved */
  U32 ulSettableBaud;
  /* BitMask - Capabilities mask for permissible data bit settings: 
       0: 5 data bits
       1: 6 data bits
       2: 7 data bits
       3: 8 data bits
       4: 16 data bits
       5: 16 data bits, extended
       6-15: reserved */
  U16 wSettableData;
  /* Number Current size of the transmit queue (allocated) */
  U32 ulCurrentTxQueue;
  /* Number Current size of the receive queue (allocated) */
  U32 ulCurrentRxQueue;
  U32 reserved2;
  U32 reserved3;
  /* “SILABS USB Vx.y” Unicode string identifying the vendor of the device.
     The last three characters indicate the version */
  U8 uniProvName[15];
} COMM_PROP_RSP_t, * COMM_PROP_RSP_p;

static COMM_PROP_RSP_t gCommPropA =
{
  .wLength            = sizeof(COMM_PROP_RSP_t),
  .bcdVersion         = COMM_PROP_RSP_VERSION,
  .ulServiceMask      = COMM_PROP_RSP_SVC_ID,
  .ulMaxTxQueue       = USB_CDC_FIFO_SIZE,
  .ulMaxRxQueue       = USB_CDC_FIFO_SIZE,
  .ulMaxBaud          = 2000000,
  .ulProvSubType      = COMM_PROP_RSP_PROV_SUB_TYPE_RS232,
  .ulProvCapabilities = COMM_PROP_RSP_PROV_CAPABS_DTR_DSR |
                        COMM_PROP_RSP_PROV_CAPABS_RTS_CTS |
                        COMM_PROP_RSP_PROV_CAPABS_CHECK_PARITY,
  .ulSettableParams   = COMM_PROP_RSP_SETBL_PARAM_PARITY_TYPE |
                        COMM_PROP_RSP_SETBL_PARAM_BAUD |
                        COMM_PROP_RSP_SETBL_PARAM_DATA_BITS |
                        COMM_PROP_RSP_SETBL_PARAM_STOP_BITS |
                        COMM_PROP_RSP_SETBL_PARAM_HANDSHAKE |
                        COMM_PROP_RSP_SETBL_PARAM_PARITY |
                        COMM_PROP_RSP_SETBL_PARAM_CARRIER_DET,
  .ulSettableBaud     = COMM_PROP_RSP_BAUD_1200   |
                        COMM_PROP_RSP_BAUD_1800   |
                        COMM_PROP_RSP_BAUD_2400   |
                        COMM_PROP_RSP_BAUD_4800   |
                        COMM_PROP_RSP_BAUD_7200   |
                        COMM_PROP_RSP_BAUD_9600   |
                        COMM_PROP_RSP_BAUD_14400  |
                        COMM_PROP_RSP_BAUD_19200  |
                        COMM_PROP_RSP_BAUD_38400  |
                        COMM_PROP_RSP_BAUD_56000  |
                        COMM_PROP_RSP_BAUD_128000 |
                        COMM_PROP_RSP_BAUD_115200 |
                        COMM_PROP_RSP_BAUD_57600  |
                        COMM_PROP_RSP_BAUD_OTHER,
  .wSettableData      = COMM_PROP_RSP_SETBL_DATA_8,
  .uniProvName        = "SILABS USB V2.0",
};

static COMM_PROP_RSP_t gCommPropB =
{
  .wLength            = sizeof(COMM_PROP_RSP_t),
  .bcdVersion         = COMM_PROP_RSP_VERSION,
  .ulServiceMask      = COMM_PROP_RSP_SVC_ID,
  .ulMaxTxQueue       = USB_CDC_FIFO_SIZE,
  .ulMaxRxQueue       = USB_CDC_FIFO_SIZE,
  .ulMaxBaud          = 2000000,
  .ulProvSubType      = COMM_PROP_RSP_PROV_SUB_TYPE_RS232,
  .ulProvCapabilities = COMM_PROP_RSP_PROV_CAPABS_DTR_DSR |
                        COMM_PROP_RSP_PROV_CAPABS_RTS_CTS |
                        COMM_PROP_RSP_PROV_CAPABS_CHECK_PARITY,
  .ulSettableParams   = COMM_PROP_RSP_SETBL_PARAM_PARITY_TYPE |
                        COMM_PROP_RSP_SETBL_PARAM_BAUD |
                        COMM_PROP_RSP_SETBL_PARAM_DATA_BITS |
                        COMM_PROP_RSP_SETBL_PARAM_STOP_BITS |
                        COMM_PROP_RSP_SETBL_PARAM_HANDSHAKE |
                        COMM_PROP_RSP_SETBL_PARAM_PARITY |
                        COMM_PROP_RSP_SETBL_PARAM_CARRIER_DET,
  .ulSettableBaud     = COMM_PROP_RSP_BAUD_1200   |
                        COMM_PROP_RSP_BAUD_1800   |
                        COMM_PROP_RSP_BAUD_2400   |
                        COMM_PROP_RSP_BAUD_4800   |
                        COMM_PROP_RSP_BAUD_7200   |
                        COMM_PROP_RSP_BAUD_9600   |
                        COMM_PROP_RSP_BAUD_14400  |
                        COMM_PROP_RSP_BAUD_19200  |
                        COMM_PROP_RSP_BAUD_38400  |
                        COMM_PROP_RSP_BAUD_56000  |
                        COMM_PROP_RSP_BAUD_128000 |
                        COMM_PROP_RSP_BAUD_115200 |
                        COMM_PROP_RSP_BAUD_57600  |
                        COMM_PROP_RSP_BAUD_OTHER,
  .wSettableData      = COMM_PROP_RSP_SETBL_DATA_8,
  .uniProvName        = "SILABS USB V2.0",
};

//-----------------------------------------------------------------------------
/* Line control settings */
/* This command adjusts the line control settings for the selected CP210x
   interface, according to the value of wValue. The settings will only take
   effect if the selection is valid for the interface (see the specific
   CP210x data sheet for more details). If an invalid setting is selected,
   the CP210x will issue a USB procedural stall. The settings are as
   follows:
     Bits 3-0: Stop bits:
       0 = 1 stop bit
       1 = 1.5 stop bits
       2 = 2 stop bits
       Other values reserved.
     Bits 7-4: Parity setting:
       0 = none.
       1 = odd.
       2 = even.
       3 = mark.
       4 = space.
       Other values reserved.
     Bits 15-8: Word length, legal values are:
       5 = 5 Bits
       6 = 6 Bits
       7 = 7 Bits
       8 = 8 Bits */
#define LINE_CTRL_STOP_BITS_POS             (0)
#define LINE_CTRL_STOP_BITS_MASK            (0x0F << LINE_CTRL_STOP_BITS_POS)
#define LINE_CTRL_STOP_BITS_10              (0 << LINE_CTRL_STOP_BITS_POS)
#define LINE_CTRL_STOP_BITS_15              (1 << LINE_CTRL_STOP_BITS_POS)
#define LINE_CTRL_STOP_BITS_20              (2 << LINE_CTRL_STOP_BITS_POS)

#define LINE_CTRL_PARITY_POS                (4)
#define LINE_CTRL_PARITY_MASK               (0x0F << LINE_CTRL_PARITY_POS)
#define LINE_CTRL_PARITY_NONE               (0 << LINE_CTRL_PARITY_POS)
#define LINE_CTRL_PARITY_ODD                (1 << LINE_CTRL_PARITY_POS)
#define LINE_CTRL_PARITY_EVEN               (2 << LINE_CTRL_PARITY_POS)
#define LINE_CTRL_PARITY_MARK               (3 << LINE_CTRL_PARITY_POS)
#define LINE_CTRL_PARITY_SPACE              (4 << LINE_CTRL_PARITY_POS)

#define LINE_CTRL_WORD_LEN_POS              (8)
#define LINE_CTRL_WORD_LEN_MASK             (0xFF << LINE_CTRL_WORD_LEN_POS)
#define LINE_CTRL_WORD_LEN_5                (5 << LINE_CTRL_WORD_LEN_POS)
#define LINE_CTRL_WORD_LEN_6                (6 << LINE_CTRL_WORD_LEN_POS)
#define LINE_CTRL_WORD_LEN_7                (7 << LINE_CTRL_WORD_LEN_POS)
#define LINE_CTRL_WORD_LEN_8                (8 << LINE_CTRL_WORD_LEN_POS)

//-----------------------------------------------------------------------------
/* Flow Control State Setting/Response */
typedef __packed struct FLOW_CTRL_STATE_RSP_S
{
  /* Control handshake:
     Bit  Name                   Size   Value  Description
     0-1  SERIAL_DTR_MASK        2      Code   This field controls the state
                                               of the DTR output for this
                                               interface. The following binary
                                               values are defined:
                                                 00: DTR is held inactive.
                                                 01: DTR is held active.
                                                 10: DTR is controlled by the
                                                     CP210x device.
                                                 11: Reserved
     2    Reserved               1      Flag   Reserved—must always be zero.
     3    SERIAL_CTS_HANDSHAKE   1      Flag   Controls how the CP210x
                                               interprets CTS from the end
                                               device:
                                                 0: CTS is simply a status input
                                                 1: CTS is a handshake line
     4    SERIAL_DSR_HANDSHAKE   1      Flag   Controls how the CP210x
                                               interprets DSR:
                                                 0: DSR is simply a status input
                                                 1: DSR is a handshake line
     5    SERIAL_DCD_HANDSHAKE   1      Flag   Controls how the CP210x
                                               interprets DCD from the end
                                               device:
                                                 0: DCD is simply a status input
                                                 1: DCD is a handshake line
     6    SERIAL_DSR_SENSITIVITY 1     Flag   Controls whether DSR controls
                                              input data reception:
                                              0: DSR is simply a status input.
                                              1: DSR low discards data
     7-31 Reserved 25 Reserved */
  U32 ulControlHandshake;
  /* Control handshake:
     Bit  Name                   Size  Value  Description
     0    SERIAL_AUTO_TRANSMIT   1     Flag   Controls whether the CP210x acts
                                              on XON/XOFF characters received
                                              from the end device.
                                                0: No XON/XOFF processing.
                                                1: XON/XOFF start/stop output
                                                   to serial port.
     1    SERIAL_AUTO_RECEIVE    1     Flag   Controls whether the CP210x will
                                              try to transmit XON/XOFF in order
                                              to start/stop the reception of
                                              data from an endl device.
                                              If set, XOFF (as defined by the
                                              SET_CHARS command) will be sent by
                                              the CP210x to the end device when
                                              the CP210x’s buffers are more
                                              than 80% full (or as selected by
                                              the XOFF threshold). XON will be
                                              sent when the interface’s buffers
                                              drop below the XON threshold, as
                                              long as it is fine to do so.
     2    SERIAL_ERROR_CHAR      1     Flag   Controls how CP210x handles
                                              characters that are received with
                                              errors:
                                                0: The character is discarded.
                                                1: The character is discarded,
                                                   and the ERROR special-
                                                   character is inserted.
                                                   The ERROR character must be
                                                   programmed by the host using
                                                   the SET_CHARS messages
                                                   (Section 5.22).
     3    SERIAL_NULL_STRIPPING  1     Flag   If set, any NULL characters
                                              received by the CP210x from the
                                              end device will be discarded and
                                              will not be passed to the host.
                                              If clear, NULL characters are 
                                              treated as data.
     4    SERIAL_BREAK_CHAR      1     Flag   If set, a received break condition
                                              causes the CP210x to insert a
                                              BREAK special character 
                                              (section 5.18) in the receive
                                              data stream. If clear, BREAK does
                                              not affect the input data stream.
                                              In either case, a received break
                                              always causes the appropriate bit
                                              of the error mask to be set.
     5    Reserved               1     Rsrvd  Reserved
     6–7  SERIAL_RTS_MASK        2     Code   This field controls the RTS line.
                                                00: RTS is statically inactive.
                                                01: RTS is statically active.
                                                10: RTS is used for receive 
                                                    flow control.
                                                11: RTS is transmit active
                                                    signal.AN571
     8–30 Reserved               23    Zero   Reserved—must be written as zero.
     31   SERIAL_XOFF_CONTINUE   1     Flag   If set, then the CP210x will send
                                              XON/XOFF receive flow control
                                              characters to the end device,
                                              even if the end device has
                                              sent XOFF to suspend output and
                                              has not yet sent XON to resume */
  U32 ulFlowReplace;
  /* Threshold for sending XON. When the available space rises above this
     amount, XON will be sent (if in auto-receive mode) */
  U32 ulXonLimit;
  /* Threshold for sending XOFF. When available space drops below this amount,
     XOFF will be sent (if in auto receive mode) */
  U32 ulXoffLimit;
} FLOW_CTRL_STATE_RSP_t, * FLOW_CTRL_STATE_RSP_p;

static FLOW_CTRL_STATE_RSP_t gFlowCtrlStateA = {0};
static FLOW_CTRL_STATE_RSP_t gFlowCtrlStateB = {0};

//-----------------------------------------------------------------------------
/* Special Characters Response */
typedef __packed struct SPEC_CHARS_S
{
  /* The character that indicates EOF (on the input) */
  U8 bEofChar;
  /* Number The character that should be inserted in the input stream when
     an error occurs */
  U8 bErrorChar;
  /* The character that should be inserted in the input stream when a break is
     detected */
  U8 bBreakChar;
  /* Number The special character that causes bit 2 of the event-occurred mask
     to be set whenever it is received */
  U8 bEventChar;
  /* The character used for XON */
  U8 bXonChar;
  /* The character used for XOFF */
  U8 bXoffChar;
} SPEC_CHARS_t, * SPEC_CHARS_p;

static SPEC_CHARS_t gSpecCharsA = {0};
static SPEC_CHARS_t gSpecCharsB = {0};






///* Line Coding Structure */
//typedef __packed struct _CDC_LINE_CODING
//{
//  U32 dwBaudRate;       /* Number Data terminal rate, in bits per second */
//  U8  bCharFormat;      /* Number of Stop bits */
//                        /*   0 - 1 Stop bit    *
//                         *   1 - 1.5 Stop bits *
//                         *   2 - 2 Stop bits   */
//  U8  bParityType;      /* Number Parity */
//                        /*   0 - None    *
//                         *   1 - Odd     *
//                         *   2 - Even    *
//                         *   3 - Mark    *
//                         *   4 - Space   */
//  U8  bDataBits;        /* Number Data Bits (5, 6, 7, 8 or 16) */
//} CDC_LINE_CODING;

///* Serial State Notification Structure */
//typedef __packed struct _CDC_SERIAL_STATE
//{
//  REQUEST_TYPE bmRequestType;
//  U8  bNotification;
//  U16 wValue;
//  U16 wIndex;
//  U16 wLength;
//  __packed union
//  {
//    U16 Raw;
//    __packed struct
//    {
//      U16 bRxCarrier : 1;
//      U16 bTxCarrier : 1;
//      U16 bBreak : 1;
//      U16 bRingSignal : 1;
//      U16 bFraming : 1;
//      U16 bParity : 1;
//      U16 bOverRun : 1;
//    } Bit;
//  } Data;
//} CDC_SERIAL_STATE;

/* CDC Port Context */
typedef struct _CDC_PORT
{
  U8                    epBlkO;
  U8                    epBlkI;
//  U8                epIrqI;
//  U8                irqBuffLen;
  FIFO_t                rxFifo;
  FIFO_t                txFifo;
  USB_CbByte            rxFifoPutCb;
  USB_CbByte            txFifoGetCb;
  U8                    rxBuffer[USB_CDC_FIFO_SIZE + 1];
  U8                    txBuffer[USB_CDC_FIFO_SIZE + 1];
  COMM_PROP_RSP_p       pCommProp;
  FLOW_CTRL_STATE_RSP_p pFlowCtrlState;
  SPEC_CHARS_p          pSpecChars;
  UART_t                uart;
  FW_BOOLEAN            ready;
} CDC_PORT;

//-----------------------------------------------------------------------------
/* Global Variables */

//STATIC CDC_LINE_CODING   gLineCodingA =
//{
//  115200,               /* dwBaudRate */
//  0,                    /* bCharFormat */
//  0,                    /* bParityType */
//  8,                    /* bDataBits */
//};

//STATIC CDC_SERIAL_STATE  gNotificationA =
//{
//  /* bmRequestType */
//  {REQUEST_TO_INTERFACE, REQUEST_CLASS, REQUEST_DEVICE_TO_HOST},
//  CDC_NTF_SERIAL_STATE, /* bNotification */
//  0,                    /* wValue */
//  USB_CDC_IF_NUM,       /* wIndex */
//  2,                    /* wLength */
//  0,                    /* Data */
//};

STATIC CDC_PORT          gPortA = {0};

//#if (USB_CDD)
//STATIC CDC_LINE_CODING   gLineCodingB =
//{
//  115200,               /* dwBaudRate */
//  0,                    /* bCharFormat */
//  0,                    /* bParityType */
//  8,                    /* bDataBits */
//};

//STATIC CDC_SERIAL_STATE  gNotificationB =
//{
//  /* bmRequestType */
//  {REQUEST_TO_INTERFACE, REQUEST_CLASS, REQUEST_DEVICE_TO_HOST},
//  CDC_NTF_SERIAL_STATE, /* bNotification */
//  0,                    /* wValue */
//  USB_CDC_IF_NUM,       /* wIndex */
//  2,                    /* wLength */
//  0,                    /* Data */
//};

STATIC CDC_PORT          gPortB = {0};
//#endif /* USB_CDD */

//-----------------------------------------------------------------------------
/* Private Functions declarations */

static FW_BOOLEAN uart_FifoPutA(U8 * pByte);
static FW_BOOLEAN uart_FifoGetA(U8 * pByte);
static FW_BOOLEAN uart_FifoPutB(U8 * pByte);
static FW_BOOLEAN uart_FifoGetB(U8 * pByte);

//-----------------------------------------------------------------------------
/** @brief Initializes the UART
 *  @param pUART - UART Number
 *  @return None
 */

//static void uart_Init(UART_t aUART)
//{
//  if (UART1 == aUART)
//  {
//    /* UART1: PA9 - Tx, PA10 - Rx, DTR - PB8, RTS - PB6 */
//    GPIO_Init(UART1_TX_PORT,  UART1_TX_PIN,  GPIO_TYPE_OUT_PP_10MHZ, 1);
//    GPIO_Init(UART1_RX_PORT,  UART1_RX_PIN,  GPIO_TYPE_IN_PUP_PDN,   1);
//    GPIO_Init(UART1_DTR_PORT, UART1_DTR_PIN, GPIO_TYPE_IN_PUP_PDN,   1);
//    GPIO_Init(UART1_RTS_PORT, UART1_RTS_PIN, GPIO_TYPE_IN_PUP_PDN,   1);
//   
//    UART_DeInit(UART1);
//    UART_Init
//    (
//      UART1,
//      gPortA.lineCoding->dwBaudRate,
//      uart_FifoPutA,
//      uart_FifoGetA
//    );
//    
//    /* UART1: PA9 - Tx, PA10 - Rx, DTR - PB8, RTS - PB6 */
//    GPIO_Init(UART1_TX_PORT,  UART1_TX_PIN,  GPIO_TYPE_ALT_PP_10MHZ, 1);
//    GPIO_Init(UART1_RX_PORT,  UART1_RX_PIN,  GPIO_TYPE_IN_PUP_PDN,   1);
//    GPIO_Init(UART1_DTR_PORT, UART1_DTR_PIN, GPIO_TYPE_OUT_OD_10MHZ, 1);
//    GPIO_Init(UART1_RTS_PORT, UART1_RTS_PIN, GPIO_TYPE_OUT_PP_10MHZ, 1);
//    
//    UART_RxStart(UART1);
//  }
//  else
//  {
//    /* UART2: PA2 - Tx, PA3 - Rx */
//    GPIO_Init(UART2_TX_PORT, UART2_TX_PIN, GPIO_TYPE_IN_PUP_PDN, 1);
//    GPIO_Init(UART2_RX_PORT, UART2_RX_PIN, GPIO_TYPE_IN_PUP_PDN, 1);
//    
//    UART_DeInit(UART2);
//    UART_Init
//    (
//      UART2,
//      gPortB.lineCoding->dwBaudRate,
//      uart_FifoPutB,
//      uart_FifoGetB
//    );
//    
//    /* UART2: PA2 - Tx, PA3 - Rx */
//    GPIO_Init(UART2_TX_PORT, UART2_TX_PIN, GPIO_TYPE_ALT_PP_10MHZ, 1);
//    GPIO_Init(UART2_RX_PORT, UART2_RX_PIN, GPIO_TYPE_IN_PUP_PDN,   1);
//
//    UART_RxStart(UART2);
//  }
//}

//-----------------------------------------------------------------------------
/** @brief Gets CDC Port according to USB Interface Number
 *  @param aInterface - USB Interface Number
 *  @return Pointer to the CDC Port Context
 */

static CDC_PORT * cdc_GetPort(U16 aInterface)
{
  CDC_PORT * result = &gPortA;

  if (USB_CDC_IF_NUM == aInterface)
  {
    result = &gPortA;
  }
  else
  {
    result = &gPortB;
  }

  return (result);
}

//-----------------------------------------------------------------------------
/** @brief Sets DTR/RTS signals
 *  @param pUART - UART Number
 *  @param aValue - DTR/RTS signals state
 *  @return None
 */

//static void uart_DTR_RTS_Set(UART_t aUART, U16 aValue)
//{
//  if (UART1 == aUART)
//  {
//    /* DTR signal */
//    if ( 0 == (aValue & (1 << CDC_CTRL_LINE_STATE_DTR)) )
//    {
//      GPIO_Hi(UART1_DTR_PORT, UART1_DTR_PIN);
//    }
//    else
//    {
//      GPIO_Lo(UART1_DTR_PORT, UART1_DTR_PIN);
//    }
//
//    /* RTS signal */
//    if ( 0 == (aValue & (1 << CDC_CTRL_LINE_STATE_RTS)) )
//    {
//      GPIO_Hi(UART1_RTS_PORT, UART1_RTS_PIN);
//    }
//    else
//    {
//      GPIO_Lo(UART1_RTS_PORT, UART1_RTS_PIN);
//    }
//  }
//}

//-----------------------------------------------------------------------------
/** @brief Processes IRQ EP data
 *  @param pPort - Pointer to Port context
 *  @return None
 */

//static void cdc_IrqInStage(CDC_PORT * pPort)
//{
//  U8 len = 0;
//
//  if (0 == pPort->irqBuffLen) return;
//
//  //if (USB_CDC_IRQ_PACKET_SIZE < pPort->irqBuffLen)
//  //{
//  //  len = USB_CDC_IRQ_PACKET_SIZE;
//  //}
//  //else
//  //{
//  //  len = pPort->irqBuffLen;
//  //}
//
//  //CDC_LOG("CDC IRQ IN: len = %d\r\n", len);
//  USB_EpWrite(pPort->epIrqI, pPort->irqBuff, len);
//
//  pPort->irqBuff += len;
//  pPort->irqBuffLen -= len;
//}

//-----------------------------------------------------------------------------
/** @brief Sends Serial State notification
 *  @param pPort - Pointer to Port context
 *  @param aState - Errors/Evetns state
 *  @return None
 */

//void cdc_NotifyState(CDC_PORT * pPort, U16 aState)
//{
//  pPort->notification->Data.Raw = aState;
//  pPort->irqBuff = (U8 *)pPort->notification;
//  pPort->irqBuffLen = sizeof(CDC_SERIAL_STATE);
//
//  cdc_IrqInStage(pPort);
//}

//-----------------------------------------------------------------------------
/** @brief CDC Control Setup USB Request
 *  @param pSetup - Pointer to Setup Packet
 *  @param pData - Pointer to place for setting the pointer to requested data
 *  @param pSize - Pointer to place for setting the requested data size
 *  @return Stage that should be performed after calling this function
 *  @note On calling this function pData points to Control Endpoint internal
 *        buffer so requested data can be placed right there if it is not
 *        exceeds Control Endpoint Max Packet size
 */

USB_CTRL_STAGE CDC_CtrlSetupReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
)
{
  USB_CTRL_STAGE result = USB_CTRL_STAGE_ERROR;
  CDC_PORT * port = cdc_GetPort(pSetup->wIndex.W);

  CDC_LOG
  (
    "CDC Setup = 0x%02X, V = 0x%04X, I = 0x%04X, L = %d\r\n",
    pSetup->bRequest,
    pSetup->wValue.W,
    pSetup->wIndex.W,
    *pSize
  );
  
  switch (pSetup->bRequest)
  {
//    case CDC_REQ_GET_PROPS:
//      //CDC_LOG("CDC Setup: SetLineCoding: IF = %d L = %d\r\n",
//      //      pSetup->wIndex.W, *pSize);
//
//      port = cdc_GetPort(pSetup->wIndex.W);
//      *pData = (U8 *)port->lineCoding;
//
//      result = USB_CTRL_STAGE_WAIT;
//      break;

    case CDC_REQ_GET_PROPS:
      CDC_LOG(" - Get Props\r\n");

      *pData = (U8 *)port->pCommProp;

      result = USB_CTRL_STAGE_DATA;
      break;

    case CDC_REQ_SET_LINE_CTL:
      CDC_LOG
      (
        " - Set Line Ctrl: Stop = %d, Parity = %d, WordLen = %d\r\n",
        (pSetup->wValue.W & LINE_CTRL_STOP_BITS_MASK),
        (pSetup->wValue.W & LINE_CTRL_PARITY_MASK) >> LINE_CTRL_PARITY_POS,
        (pSetup->wValue.W & LINE_CTRL_WORD_LEN_MASK) >> LINE_CTRL_WORD_LEN_POS
      );

      //uart_DTR_RTS_Set(port->uart, pSetup->wValue.W);
      result = USB_CTRL_STAGE_STATUS;
      break;
      
    case CDC_REQ_SET_FLOW:
      CDC_LOG(" - Set Flow Control State\r\n");

      *pData = (U8 *)port->pFlowCtrlState;

      result = USB_CTRL_STAGE_WAIT;
      break;
      
    case CDC_REQ_SET_CHARS:
      CDC_LOG(" - Set Spec Chars\r\n");

      *pData = (U8 *)port->pSpecChars;

      result = USB_CTRL_STAGE_WAIT;
      break;
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief CDC USB Out Request
 *  @param pSetup - Pointer to Setup Packet
 *  @param pData - Pointer to place for setting the pointer to requested data
 *  @param pSize - Pointer to place for setting the requested data size
 *  @return Stage that should be performed after calling this function
 *  @note Called when all the OUT packets have been already collected
 */

USB_CTRL_STAGE CDC_CtrlOutReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
)
{
  USB_CTRL_STAGE result = USB_CTRL_STAGE_ERROR;
  CDC_PORT * port = cdc_GetPort(pSetup->wIndex.W);

  CDC_LOG
  (
    "CDC Out L = %d D = 0x%08X\r\n",
    *pSize,
    *((U32 *)*pData)
  );
  
  switch (pSetup->bRequest)
  {
    case CDC_REQ_SET_FLOW:
      CDC_LOG
      (
        " - Set Flow: CH = %d FR = %d, XNL = %d XFL = %d\r\n",
        port->pFlowCtrlState->ulControlHandshake,
        port->pFlowCtrlState->ulFlowReplace,
        port->pFlowCtrlState->ulXonLimit,
        port->pFlowCtrlState->ulXoffLimit
      );
      break;
      
    case CDC_REQ_SET_CHARS:
      CDC_LOG
      (
        " - Set Spec Chars: Xon = 0x%02X Xoff = 0x%02X\r\n",
        port->pSpecChars->bXonChar,
        port->pSpecChars->bXoffChar
      );
      break;

//    case CDC_REQ_SET_LINE_CODING:
//      port = cdc_GetPort(pSetup->wIndex.W);
//      GPIO_Hi(GPIOB, 3);
//      uart_Init(port->uart);
//      GPIO_Lo(GPIOB, 3);
//      port->ready = FW_TRUE;
//
//      //CDC_LOG("CDC Out: Set Line Coding: IF = %d Baud = %d, Len = %d\r\n",
//      //      pSetup->wIndex.W, port->lineCoding->dwBaudRate, *pSize);
//
//      result = USB_CTRL_STAGE_STATUS;
//      break;
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Processes OUT EP data (Rx via USB)
 *  @param pPort - Pointer to Port context
 *  @return None
 */

static void cdc_OutStage(CDC_PORT * pPort)
{
  /* Read from OUT EP */
  (void)USB_EpReadWsCb
  (
    pPort->epBlkO,
    pPort->rxFifoPutCb,
    FIFO_Free(&pPort->rxFifo)
  );

//  /* Write to UART */
//  if (0 < FIFO_Count(&pPort->rxFifo))
//  {
//    UART_TxStart(pPort->uart);
//  }
}

//-----------------------------------------------------------------------------
/** @brief Processes IN EP data (Tx via USB)
 *  @param pPort - Pointer to Port context
 *  @return None
 */

static void cdc_InStage(CDC_PORT * pPort)
{
  /* If there are some data in FIFO */
  if (0 < FIFO_Count(&pPort->txFifo))
  {
    /* Write to IN EP */
    (void)USB_EpWriteWsCb
    (
      pPort->epBlkI,
      pPort->txFifoGetCb,
      FIFO_Count(&pPort->txFifo)
    );
  }
}

//-----------------------------------------------------------------------------
/** @brief Processes Rx/Tx data if present in I/O Buffers
 *  @param pPort - Pointer to the Port context
 *  @return None
 */

static void cdc_ProcessCollectedData(CDC_PORT * pPort)
{
  /* Check if there are some unprocessed data */
  if (FW_TRUE == pPort->ready)
  {
    if (FW_FALSE == USB_EpIsRxEmpty(pPort->epBlkO))
    {
      cdc_OutStage(pPort);
    }

    if (FW_TRUE == USB_EpIsTxEmpty(pPort->epBlkI))
    {
      cdc_InStage(pPort);
    }
  }
}

//-----------------------------------------------------------------------------
/** @brief Called on each USB Start Of Frame (every 1 ms)
 *  @param None
 *  @return None
 */

void CDC_SOF(void)
{
  cdc_ProcessCollectedData(&gPortA);
#if (USB_CDD)
  cdc_ProcessCollectedData(&gPortB);
#endif
}

//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */

//static void cdc_InterruptAIn(U32 aEvent)
//{
//  cdc_IrqInStage(&gPortA);
//}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */

static void cdc_BulkAIn(U32 aEvent)
{
  cdc_InStage(&gPortA);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */

static void cdc_BulkAOut(U32 aEvent)
{
  cdc_OutStage(&gPortA);
}

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from USB EP buffer to the Rx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static void cdc_RxFifoPutA(U8 * pByte)
{
  (void)FIFO_Put(&gPortA.rxFifo, pByte);
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte that need to be transmitted from the Tx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static void cdc_TxFifoGetA(U8 * pByte)
{
  (void)FIFO_Get(&gPortA.txFifo, pByte);
}

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from UART to the Tx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static FW_BOOLEAN uart_FifoPutA(U8 * pByte)
{
  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Put(&gPortA.txFifo, pByte));
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte that need to be transmitted from the Rx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static FW_BOOLEAN uart_FifoGetA(U8 * pByte)
{
  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Get(&gPortA.rxFifo, pByte));
}

//#if (USB_CDD)
//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */

//static void cdc_InterruptBIn(U32 aEvent)
//{
//  cdc_IrqInStage(&gPortB);
//}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */

static void cdc_BulkBIn(U32 aEvent)
{
  cdc_InStage(&gPortB);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */

static void cdc_BulkBOut(U32 aEvent)
{
  cdc_OutStage(&gPortB);
}

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from USB EP buffer to the Rx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static void cdc_RxFifoPutB(U8 * pByte)
{
  (void)FIFO_Put(&gPortB.rxFifo, pByte);
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte that need to be transmitted from the Tx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static void cdc_TxFifoGetB(U8 * pByte)
{
  (void)FIFO_Get(&gPortB.txFifo, pByte);
}

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from UART to the Tx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static FW_BOOLEAN uart_FifoPutB(U8 * pByte)
{
  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Put(&gPortB.txFifo, pByte));
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte that need to be transmitted from the Rx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static FW_BOOLEAN uart_FifoGetB(U8 * pByte)
{
  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Get(&gPortB.rxFifo, pByte));
}
//#endif /* USB_CDD */

//-----------------------------------------------------------------------------
/** @brief Initializes CDC
 *  @param None
 *  @return None
 */

void CDC_Init(void)
{
  /* Register appropriate EP callbacks */
  USB_SetCb_Ep(USB_CDC_EP_BLK_O, cdc_BulkAOut);
  USB_SetCb_Ep(USB_CDC_EP_BLK_I, cdc_BulkAIn);
  //USB_SetCb_Ep(USB_CDC_EP_IRQ_I, cdc_InterruptAIn);
  /* Clear Port context */
  memset(&gPortA, 0, sizeof(gPortA));
  /* Port is not ready yet */
  gPortA.ready = FW_FALSE;
  /* Initialize Endpoints */
  gPortA.epBlkO = USB_CDC_EP_BLK_O;
  gPortA.epBlkI = USB_CDC_EP_BLK_I;
//  gPortA.epIrqI = USB_CDC_EP_IRQ_I;
  /* Initialize FIFOs */
  FIFO_Init(&gPortA.rxFifo, gPortA.rxBuffer, sizeof(gPortA.rxBuffer));
  FIFO_Init(&gPortA.txFifo, gPortA.txBuffer, sizeof(gPortA.txBuffer));
  gPortA.rxFifoPutCb = cdc_RxFifoPutA;
  gPortA.txFifoGetCb = cdc_TxFifoGetA;
  /* Initialize pointers */
//  gPortA.lineCoding = &gLineCodingA;
//  gPortA.notification = &gNotificationA;
  gPortA.pCommProp = &gCommPropA;
  gPortA.pFlowCtrlState = &gFlowCtrlStateA;
  gPortA.pSpecChars = &gSpecCharsA;
  /* Initialize UART Number */
  gPortA.uart = UART1;

//#if (USB_CDD)
  /* Register appropriate EP callbacks */
  USB_SetCb_Ep(USB_CDD_EP_BLK_O, cdc_BulkBOut);
  USB_SetCb_Ep(USB_CDD_EP_BLK_I, cdc_BulkBIn);
//  USB_SetCb_Ep(USB_CDD_EP_IRQ_I, cdc_InterruptBIn);
  /* Clear Port context */
  memset(&gPortB, 0, sizeof(gPortB));
  /* Port is not ready yet */
  gPortB.ready = FW_FALSE;
  /* Initialize Endpoints */
  gPortB.epBlkO = USB_CDD_EP_BLK_O;
  gPortB.epBlkI = USB_CDD_EP_BLK_I;
  //gPortB.epIrqI = USB_CDD_EP_IRQ_I;
  /* Initialize FIFOs */
  FIFO_Init(&gPortB.rxFifo, gPortB.rxBuffer, sizeof(gPortB.rxBuffer));
  FIFO_Init(&gPortB.txFifo, gPortB.txBuffer, sizeof(gPortB.txBuffer));
  gPortB.rxFifoPutCb = cdc_RxFifoPutB;
  gPortB.txFifoGetCb = cdc_TxFifoGetB;
  /* Initialize pointers */
//  gPortB.lineCoding = &gLineCodingB;
//  gPortB.notification = &gNotificationB;
  gPortB.pCommProp = &gCommPropB;
  gPortB.pFlowCtrlState = &gFlowCtrlStateB;
  gPortB.pSpecChars = &gSpecCharsB;
  /* Initialize UART Number */
  gPortB.uart = UART2;
//#endif
}