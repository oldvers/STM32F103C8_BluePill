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

//#define CDC_DEBUG

#ifdef CDC_DEBUG
#  define CDC_LOG           LOG
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
                        COMM_PROP_RSP_SETBL_PARAM_PARITY,
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
                        COMM_PROP_RSP_SETBL_PARAM_PARITY,
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

#define FLOW_CTRL_HDSHAKE_DTR_POS              (0)
#define FLOW_CTRL_HDSHAKE_DTR_MASK             (3 << FLOW_CTRL_HDSHAKE_DTR_POS)
#define FLOW_CTRL_HDSHAKE_DTR_HELD_INACTIVE    (0 << FLOW_CTRL_HDSHAKE_DTR_POS)
#define FLOW_CTRL_HDSHAKE_DTR_HELD_ACTIVE      (1 << FLOW_CTRL_HDSHAKE_DTR_POS)
#define FLOW_CTRL_HDSHAKE_DTR_CTRL_BY_CP210X   (2 << FLOW_CTRL_HDSHAKE_DTR_POS)

#define FLOW_CTRL_REPLACE_RTS_POS              (6)
#define FLOW_CTRL_REPLACE_RTS_MASK             (3 << FLOW_CTRL_REPLACE_RTS_POS)
#define FLOW_CTRL_REPLACE_RTS_STATIC_INACTIVE  (0 << FLOW_CTRL_REPLACE_RTS_POS)
#define FLOW_CTRL_REPLACE_RTS_STATIC_ACTIVE    (1 << FLOW_CTRL_REPLACE_RTS_POS)
#define FLOW_CTRL_REPLACE_RTS_USED_FOR_RX      (2 << FLOW_CTRL_REPLACE_RTS_POS)
#define FLOW_CTRL_REPLACE_RTS_TX_ACTIVE        (3 << FLOW_CTRL_REPLACE_RTS_POS)

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

//-----------------------------------------------------------------------------
/* Modem Status */
/* This command returns the current states of the RS-232 modem control lines
   for the specified CP210x interface. The modem control line status byte is
   defined as follows:
     bit 0: DTR state (as set by host or by handshaking logic in CP210x)
     bit 1: RTS state (as set by host or by handshaking logic in CP210x)
     bits 2–3: reserved
     bit 4: CTS state (as set by end device)
     bit 5: DSR state (as set by end device)
     bit 6: RI state (as set by end device)
     bit 7: DCD state (as set by end device) */
#define MODEM_STATUS_DTR                                   (1 << 0)
#define MODEM_STATUS_RTS                                   (1 << 1)
#define MODEM_STATUS_CTS                                   (1 << 4)
#define MODEM_STATUS_DSR                                   (1 << 5)
#define MODEM_STATUS_RI                                    (1 << 6)
#define MODEM_STATUS_DCD                                   (1 << 7)

//-----------------------------------------------------------------------------
/* Serial Status Response */
typedef __packed struct SERIAL_STATUS_S
{
  /* BitMask - Defines the current error status:
       bit 0: break
       bit 1: framing error
       bit 2: hardware overrun
       bit 3: queue overrun
       bit 4: parity error
       bits 5-31: reserved */
  U32 ulErrors;
  /* BitMask Reason(s) CP210x is holding:
       Transmit:
         bit 0: waiting for CTS
         bit 1:waiting for DSR
         bit 2: waiting for DCD
         bit 3: waiting for XON
         bit 4: XOFF sent, waiting
         bit 5: waiting on BREAK
       Receive:
         bit 6: waiting for DSR
         bits 7-31: reserved */
  U32 ulHoldReasons;
  /* Number of bytes waiting in the input queue */
  U32 ulAmountInInQueue;
  /* Number of bytes waiting in hte output queue */
  U32 ulAmountInOutQueue;
  /* Boolean - Always zero */
  U8 bEofReceived;
  /* Boolean - 0x01 if waiting for an immediate transmission to be sent */
  U8 bWaitForImmediate;
  /* Zero - Reserved for future use */
  U8 bReserved;
} SERIAL_STATUS_t, * SERIAL_STATUS_p;

SERIAL_STATUS_t gSerialStatusA = {0};
SERIAL_STATUS_t gSerialStatusB = {0};

//-----------------------------------------------------------------------------
/* This command sets the modem handshaking states for the selected CP210x
   interface according to the value of wValue. DTR and RTS values can be set
   only if the current handshaking state of the interface allows direct control
   of the modem control lines.
     bit 0: DTR state
     bit 1: RTS state
     bits 2–7: reserved
     bit 8: DTR mask, if clear, DTR will not be changed
     bit 9: RTS mask, if clear, RTS will not be changed
     bits 10–15: reserved */
#define MODEM_HANDSHAKE_STATE_DTR                          (1 << 0)
#define MODEM_HANDSHAKE_STATE_RTS                          (1 << 1)
#define MODEM_HANDSHAKE_STATE_DTR_EN                       (1 << 8)
#define MODEM_HANDSHAKE_STATE_RTS_EN                       (1 << 9)

//-----------------------------------------------------------------------------
/* This command causes the CP210x to purge the selected transmit or receive
   queues, based on the value of the mask. The bit meanings are as follows:
     bit 0: Clear the transmit queue
     bit 1: Clear the receive queue
     bit 2: Clear the transmit queue
     bit 3: Clear the receive queue */
#define PURGE_CLEAR_TX_QUEUE1                              (1 << 0)
#define PURGE_CLEAR_RX_QUEUE1                              (1 << 1)
#define PURGE_CLEAR_TX_QUEUE2                              (1 << 2)
#define PURGE_CLEAR_RX_QUEUE2                              (1 << 3)

//-----------------------------------------------------------------------------
/* Vendor Specific Commands */
#define CP210X_READ_LATCH	                                 (0x00C2)
#define CP210X_GET_PARTNUM	                               (0x370B)
#define CP210X_GET_PORTCONFIG	                             (0x370C)
#define CP210X_GET_DEVICEMODE	                             (0x3711)
#define CP210X_WRITE_LATCH	                               (0x37E1)

/* Part number definitions */
#define CP210X_PARTNUM_CP2101	                             (0x01)
#define CP210X_PARTNUM_CP2102	                             (0x02)
#define CP210X_PARTNUM_CP2103	                             (0x03)
#define CP210X_PARTNUM_CP2104	                             (0x04)
#define CP210X_PARTNUM_CP2105	                             (0x05)
#define CP210X_PARTNUM_CP2108	                             (0x08)

static U8 gPartNumber = CP210X_PARTNUM_CP2105;

//-----------------------------------------------------------------------------
/* CDC Port Context */
typedef struct _CDC_PORT
{
  U8                    epBlkO;
  U8                    epBlkI;
  U16                   modemHandshake;
  U8                    modemStatus;
  FW_BOOLEAN            rxComplete;
  U32                   baudrate;
  FIFO_t                rxFifo;
  FIFO_t                txFifo;
  USB_CbByte            rxFifoPutCb;
  USB_CbByte            txFifoGetCb;
  U8                    rxBuffer[USB_CDC_FIFO_SIZE + 1];
  U8                    txBuffer[USB_CDC_FIFO_SIZE + 1];
  COMM_PROP_RSP_p       pCommProp;
  FLOW_CTRL_STATE_RSP_p pFlowCtrlState;
  SPEC_CHARS_p          pSpecChars;
  SERIAL_STATUS_p       pSerialStatus;
  UART_t                uart;
  FW_BOOLEAN            ready;
} CDC_PORT;

//-----------------------------------------------------------------------------
/* Global Variables */

STATIC CDC_PORT          gPortA = {0};
STATIC CDC_PORT          gPortB = {0};

//-----------------------------------------------------------------------------
/* Private Functions declarations */

static FW_BOOLEAN uart_FifoPutA(U8 * pByte);
static FW_BOOLEAN uart_FifoGetA(U8 * pByte);
static FW_BOOLEAN uart_RxCompleteA(U8 * pByte);
static FW_BOOLEAN uart_TxCompleteA(U8 * pByte);
static FW_BOOLEAN uart_FifoPutB(U8 * pByte);
static FW_BOOLEAN uart_FifoGetB(U8 * pByte);
static FW_BOOLEAN uart_RxCompleteB(U8 * pByte);
static FW_BOOLEAN uart_TxCompleteB(U8 * pByte);

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
/** @brief Gets DTR/RTS signals' state
 *  @param pPort - COM Port context
 *  @return Pointer to DTR/RTS signals' state value
 */

static U8 * cdc_GetUartDtrRts(CDC_PORT * pPort)
{
  /* Bit 0 - DTR state, Bit 1 - RTS state */
  if (UART1 == pPort->uart)
  {
    if (0 == GPIO_In(UART1_DTR_PORT, UART1_DTR_PIN))
    {
      pPort->modemStatus |= MODEM_STATUS_DTR;
    }
    else
    {
      pPort->modemStatus &= ~MODEM_STATUS_DTR;
    }

    if (0 == GPIO_In(UART1_RTS_PORT, UART1_RTS_PIN))
    {
      pPort->modemStatus |= MODEM_STATUS_RTS;
    }
    else
    {
      pPort->modemStatus &= ~MODEM_STATUS_RTS;
    }
  }

  return (U8 *)&pPort->modemStatus;
}

//-----------------------------------------------------------------------------
/** @brief Gets COM Port status (rx/tx queue, errors, hold reasons)
 *  @param pPort - COM Port context
 *  @return Pointer to COM Port status structure
 */

static U8 * cdc_GetUartStatus(CDC_PORT * pPort)
{
  /* COM Port Rx/Tx Queue Status */
  pPort->pSerialStatus->ulAmountInInQueue = FIFO_Count(&pPort->txFifo);
  pPort->pSerialStatus->ulAmountInOutQueue = FIFO_Count(&pPort->rxFifo);
  /* Errors not implemented yet */
  //pPort->pSerialStatus->ulErrors = 0;
  //pPort->pSerialStatus->ulHoldReasons = 0;

  return (U8 *)pPort->pSerialStatus;
}

//-----------------------------------------------------------------------------
/** @brief Sets COM Port Parameters
 *  @param pPort - COM Port context
 *  @return None
 */

static void cdc_SetUartParameters(CDC_PORT * pPort, U16 aValue)
{
  /* Not implemented yet */
  (void)pPort;
  (void)aValue;
  /* Stop bits, Parity, Word length */
  //pSetup->wValue.W & LINE_CTRL_STOP_BITS_MASK
  //pSetup->wValue.W & LINE_CTRL_PARITY_MASK) >> LINE_CTRL_PARITY_POS
  //pSetup->wValue.W & LINE_CTRL_WORD_LEN_MASK) >> LINE_CTRL_WORD_LEN_POS
}

//-----------------------------------------------------------------------------
/** @brief In/Deitializes the UART
 *  @param pPort - COM Port context
 *  @param aValue - 0 - deinitialize COM Port, else - Initialize COM Port
 *  @return None
 */

static void cdc_SetUartEnabled(CDC_PORT * pPort, U16 aValue)
{
  /* COM Port Open/Close */
  if (0 == aValue)
  {
    if (UART1 == pPort->uart)
    {
      /* UART1: PA9 - Tx, PA10 - Rx, DTR - PB8, RTS - PB6 */
      GPIO_Init(UART1_TX_PORT,  UART1_TX_PIN,  GPIO_TYPE_OUT_PP_10MHZ, 1);
      GPIO_Init(UART1_RX_PORT,  UART1_RX_PIN,  GPIO_TYPE_IN_PUP_PDN,   1);
      GPIO_Init(UART1_DTR_PORT, UART1_DTR_PIN, GPIO_TYPE_IN_PUP_PDN,   1);
      GPIO_Init(UART1_RTS_PORT, UART1_RTS_PIN, GPIO_TYPE_IN_PUP_PDN,   1);
    }
    else
    {
      /* UART2: PA2 - Tx, PA3 - Rx */
      GPIO_Init(UART2_TX_PORT, UART2_TX_PIN, GPIO_TYPE_IN_PUP_PDN, 1);
      GPIO_Init(UART2_RX_PORT, UART2_RX_PIN, GPIO_TYPE_IN_PUP_PDN, 1);
    }
    UART_DeInit(pPort->uart);
    pPort->ready = FW_FALSE;
  }
  else
  {
    if (UART1 == pPort->uart)
    {
      UART_Init
      (
        UART1,
        gPortA.baudrate,
        uart_FifoPutA,
        uart_RxCompleteA,
        uart_FifoGetA,
        uart_TxCompleteA
      );

      /* UART1: PA9 - Tx, PA10 - Rx, DTR - PB8, RTS - PB6 */
      GPIO_Init(UART1_TX_PORT,  UART1_TX_PIN,  GPIO_TYPE_ALT_PP_10MHZ, 1);
      GPIO_Init(UART1_RX_PORT,  UART1_RX_PIN,  GPIO_TYPE_IN_PUP_PDN,   1);
      GPIO_Init(UART1_DTR_PORT, UART1_DTR_PIN, GPIO_TYPE_OUT_OD_10MHZ, 1);
      GPIO_Init(UART1_RTS_PORT, UART1_RTS_PIN, GPIO_TYPE_OUT_PP_10MHZ, 1);
    }
    else
    {
      UART_Init
      (
        UART2,
        gPortB.baudrate,
        uart_FifoPutB,
        uart_RxCompleteB,
        uart_FifoGetB,
        uart_TxCompleteB
      );

      /* UART2: PA2 - Tx, PA3 - Rx */
      GPIO_Init(UART2_TX_PORT, UART2_TX_PIN, GPIO_TYPE_ALT_PP_10MHZ, 1);
      GPIO_Init(UART2_RX_PORT, UART2_RX_PIN, GPIO_TYPE_IN_PUP_PDN,   1);
    }
    pPort->ready = FW_TRUE;
    pPort->modemStatus = (MODEM_STATUS_DTR | MODEM_STATUS_RTS);
    UART_RxStart(pPort->uart);
  }
}

//-----------------------------------------------------------------------------
/** @brief Purges Rx/Tx buffers
 *  @param pPort - COM Port context
 *  @param aValue - Bitfield, identifies which buffer should be cleared
 *  @return None
 */

static void cdc_UartPurge(CDC_PORT * pPort, U16 aValue)
{
  if (0 != (aValue & (PURGE_CLEAR_TX_QUEUE1 | PURGE_CLEAR_TX_QUEUE2)))
  {
    FIFO_Clear(&pPort->txFifo);
  }

  if (0 != (aValue & (PURGE_CLEAR_RX_QUEUE1 | PURGE_CLEAR_RX_QUEUE2)))
  {
    FIFO_Clear(&pPort->rxFifo);
  }
}

//-----------------------------------------------------------------------------
/** @brief Sets DTR/RTS signals
 *  @param pPort - COM Port context
 *  @param aValue - DTR/RTS signals state
 *  @return None
 */

static void cdc_SetUartDtrRts(CDC_PORT * pPort, U16 aValue)
{
  pPort->modemHandshake = aValue;

  if (UART1 != pPort->uart) return;

  if (0 != (aValue & MODEM_HANDSHAKE_STATE_DTR_EN))
  {
    if (0 == (aValue & MODEM_HANDSHAKE_STATE_DTR))
    {
      CDC_LOG(" --- DTR Set\r\n");
      GPIO_Hi(UART1_DTR_PORT, UART1_DTR_PIN);
    }
    else
    {
      CDC_LOG(" --- DTR Clear\r\n");
      GPIO_Lo(UART1_DTR_PORT, UART1_DTR_PIN);
    }
  }

  if (0 != (aValue & MODEM_HANDSHAKE_STATE_RTS_EN))
  {
    if (0 == (aValue & MODEM_HANDSHAKE_STATE_RTS))
    {
      CDC_LOG(" --- RTS Set\r\n");
      GPIO_Hi(UART1_RTS_PORT, UART1_RTS_PIN);
    }
    else
    {
      CDC_LOG(" --- RTS Clear\r\n");
      GPIO_Lo(UART1_RTS_PORT, UART1_RTS_PIN);
    }
  }
}

//-----------------------------------------------------------------------------
/** @brief Sets COM Port BaudRate
 *  @param pPort - COM Port context
 *  @return None
 */

static void cdc_SetUartBaudrate(CDC_PORT * pPort)
{
  UART_SetBaudrate(pPort->uart, pPort->baudrate);
}

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

  switch (pSetup->bRequest)
  {
    /* Get Requests */

    case CDC_REQ_GET_PROPS:
      CDC_LOG(" - Get Props\r\n");
      *pData = (U8 *)port->pCommProp;
      result = USB_CTRL_STAGE_DATA;
      break;

    case CDC_REQ_GET_MDMSTS:
      CDC_LOG(" - Get Modem Status\r\n");
      *pData = cdc_GetUartDtrRts(port);
      result = USB_CTRL_STAGE_DATA;
      break;

    case CDC_REQ_GET_COMM_STATUS:
      CDC_LOG(" - Get Serial Status\r\n");
      *pData = cdc_GetUartStatus(port);
      result = USB_CTRL_STAGE_DATA;
      break;

    /* Set Requests without OUT stage */

    case CDC_REQ_SET_LINE_CTL:
      CDC_LOG
      (
        " - Set Line Ctrl: Stop = %d, Parity = %d, WordLen = %d\r\n",
        (pSetup->wValue.W & LINE_CTRL_STOP_BITS_MASK),
        (pSetup->wValue.W & LINE_CTRL_PARITY_MASK) >> LINE_CTRL_PARITY_POS,
        (pSetup->wValue.W & LINE_CTRL_WORD_LEN_MASK) >> LINE_CTRL_WORD_LEN_POS
      );
      cdc_SetUartParameters(port, pSetup->wValue.W);
      result = USB_CTRL_STAGE_STATUS;
      break;

    case CDC_REQ_IFC_ENABLE:
      CDC_LOG(" - Port %d Enable = %d\r\n", port->uart, pSetup->wValue.W);
      cdc_SetUartEnabled(port, pSetup->wValue.W);
      result = USB_CTRL_STAGE_STATUS;
      break;

    case CDC_REQ_PURGE:
      CDC_LOG(" - Purge - 0x%04X\r\n", pSetup->wValue.W);
      cdc_UartPurge(port, pSetup->wValue.W);
      result = USB_CTRL_STAGE_STATUS;
      break;

    case CDC_REQ_SET_MHS:
      CDC_LOG(" - Set Mdm Handshake = 0x%04X\r\n", pSetup->wValue.W);
      cdc_SetUartDtrRts(port, pSetup->wValue.W);
      result = USB_CTRL_STAGE_STATUS;
      break;

    /* Set Requests with OUT stage */

    case CDC_REQ_SET_FLOW:
      CDC_LOG(" - Set Flow Control State\r\n");
      /* DTR/RTS operation mode: hardcoded as staticaly active both */
      *pData = (U8 *)port->pFlowCtrlState;
      result = USB_CTRL_STAGE_WAIT;
      break;

    case CDC_REQ_SET_CHARS:
      CDC_LOG(" - Set Spec Chars\r\n");
      *pData = (U8 *)port->pSpecChars;
      result = USB_CTRL_STAGE_WAIT;
      break;

    case CDC_REQ_SET_BAUDRATE:
      CDC_LOG(" - Set Baud Rate\r\n");
      *pData = (U8 *)&port->baudrate;
      result = USB_CTRL_STAGE_WAIT;
      break;

    /* Vendor Specific */

    case CDC_REQ_VENDOR_SPECIFIC:
      switch(pSetup->wValue.W)
      {
        case CP210X_GET_PARTNUM:
          CDC_LOG(" - Get Part Number\r\n");
          *pData = (U8 *)&gPartNumber;
          result = USB_CTRL_STAGE_DATA;
          break;

        default:
          CDC_LOG(" - Vendor Specific - 0x%04X\r\n", pSetup->wValue.W);
          break;
      }
      break;

    case CDC_REQ_SET_BAUDDIV:
    case CDC_REQ_GET_BAUDDIV:
    case CDC_REQ_GET_LINE_CTL:
    case CDC_REQ_SET_BREAK:
    case CDC_REQ_IMM_CHAR:
    case CDC_REQ_SET_XON:
    case CDC_REQ_SET_XOFF:
    case CDC_REQ_SET_EVENTMASK:
    case CDC_REQ_GET_EVENTMASK:
    case CDC_REQ_GET_EVENTSTATE:
    case CDC_REQ_SET_CHAR:
    case CDC_REQ_GET_CHARS:
    case CDC_REQ_RESET:
    case CDC_REQ_GET_FLOW:
    case CDC_REQ_EMBED_EVENTS:
    case CDC_REQ_GET_BAUDRATE:
    default:
      CDC_LOG
      (
        "CDC Setup = 0x%02X, V = 0x%04X, I = 0x%04X, L = %d\r\n",
        pSetup->bRequest,
        pSetup->wValue.W,
        pSetup->wIndex.W,
        *pSize
      );
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
#ifdef CDC_DEBUG
      U32 ctrlHndshake, flowReplace;
#endif /* CDC_DEBUG */

  switch (pSetup->bRequest)
  {
    case CDC_REQ_SET_FLOW:
#ifdef CDC_DEBUG
      ctrlHndshake = port->pFlowCtrlState->ulControlHandshake;
      ctrlHndshake &= FLOW_CTRL_HDSHAKE_DTR_MASK;
      ctrlHndshake >>= FLOW_CTRL_HDSHAKE_DTR_POS;

      flowReplace = port->pFlowCtrlState->ulFlowReplace;
      flowReplace &= FLOW_CTRL_REPLACE_RTS_MASK;
      flowReplace >>= FLOW_CTRL_REPLACE_RTS_POS;
#endif /* CDC_DEBUG */

      CDC_LOG
      (
        " - Set Flow Ctrl: DTR = %d RTS = %d\r\n",
        ctrlHndshake,
        flowReplace
      );

      result = USB_CTRL_STAGE_STATUS;
      break;

    case CDC_REQ_SET_CHARS:
      CDC_LOG
      (
        " - Set Spec Chars: Xon = 0x%02X Xoff = 0x%02X\r\n",
        port->pSpecChars->bXonChar,
        port->pSpecChars->bXoffChar
      );
      result = USB_CTRL_STAGE_STATUS;
      break;

    case CDC_REQ_SET_BAUDRATE:
      CDC_LOG(" --- Velue = %d\r\n", port->baudrate);
      cdc_SetUartBaudrate(port);
      result = USB_CTRL_STAGE_STATUS;
      break;

    default:
      CDC_LOG
      (
        "CDC Out L = %d D = 0x%08X\r\n",
        *pSize,
        *((U32 *)*pData)
      );
      break;
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

  /* Write to UART */
  if (0 < FIFO_Count(&pPort->rxFifo))
  {
    UART_TxStart(pPort->uart);
  }
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
  else
  {
    pPort->rxComplete = FW_FALSE;
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

    if (((FW_TRUE == pPort->rxComplete) ||
         (USB_CDC_PACKET_SIZE < FIFO_Count(&pPort->txFifo))) &&
        (FW_TRUE == USB_EpIsTxEmpty(pPort->epBlkI)))
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
 *  @return TRUE if byte has been put successfully
 */

static FW_BOOLEAN uart_FifoPutA(U8 * pByte)
{
  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Put(&gPortA.txFifo, pByte));
}

//-----------------------------------------------------------------------------
/** @brief Receive complete callback
 *  @param pByte - Optional pointer to the latest received byte (def. NULL)
 *  @return TRUE, that means UART line idle is received
 */

static FW_BOOLEAN uart_RxCompleteA(U8 * pByte)
{
  gPortA.rxComplete = FW_TRUE;
  return FW_TRUE;
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte that need to be transmitted from the Rx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return TRUE if byte has been gotten successfully
 */

static FW_BOOLEAN uart_FifoGetA(U8 * pByte)
{
  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Get(&gPortA.rxFifo, pByte));
}

//-----------------------------------------------------------------------------
/** @brief Transmit complete callback
 *  @param pByte - Optional pointer to the latest received byte (def. NULL)
 *  @return TRUE, that means UART transmission is complete
 */

static FW_BOOLEAN uart_TxCompleteA(U8 * pByte)
{
  return FW_TRUE;
}

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
 *  @return TRUE if byte has been put successfully
 */

static FW_BOOLEAN uart_FifoPutB(U8 * pByte)
{
  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Put(&gPortB.txFifo, pByte));
}

//-----------------------------------------------------------------------------
/** @brief Receive complete callback
 *  @param pByte - Optional pointer to the latest received byte (def. NULL)
 *  @return TRUE, that means UART line idle received
 */

static FW_BOOLEAN uart_RxCompleteB(U8 * pByte)
{
  gPortB.rxComplete = FW_TRUE;
  return FW_TRUE;
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

//-----------------------------------------------------------------------------
/** @brief Transmit complete callback
 *  @param pByte - Optional pointer to the latest received byte (def. NULL)
 *  @return TRUE, that means UART transmission is complete
 */

static FW_BOOLEAN uart_TxCompleteB(U8 * pByte)
{
  return FW_TRUE;
}

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
  /* Clear Port context */
  memset(&gPortA, 0, sizeof(gPortA));
  /* Port is not ready yet */
  gPortA.ready = FW_FALSE;
  gPortA.rxComplete = FW_FALSE;
  /* Initialize Endpoints */
  gPortA.epBlkO = USB_CDC_EP_BLK_O;
  gPortA.epBlkI = USB_CDC_EP_BLK_I;
  /* Initialize FIFOs */
  FIFO_Init(&gPortA.rxFifo, gPortA.rxBuffer, sizeof(gPortA.rxBuffer));
  FIFO_Init(&gPortA.txFifo, gPortA.txBuffer, sizeof(gPortA.txBuffer));
  gPortA.rxFifoPutCb = cdc_RxFifoPutA;
  gPortA.txFifoGetCb = cdc_TxFifoGetA;
  /* Initialize pointers */
  gPortA.pCommProp = &gCommPropA;
  gPortA.pFlowCtrlState = &gFlowCtrlStateA;
  gPortA.pSpecChars = &gSpecCharsA;
  gPortA.pSerialStatus = &gSerialStatusA;
  /* Initialize UART Number */
  gPortA.uart = UART1;

  /* Register appropriate EP callbacks */
  USB_SetCb_Ep(USB_CDD_EP_BLK_O, cdc_BulkBOut);
  USB_SetCb_Ep(USB_CDD_EP_BLK_I, cdc_BulkBIn);
  /* Clear Port context */
  memset(&gPortB, 0, sizeof(gPortB));
  /* Port is not ready yet */
  gPortB.ready = FW_FALSE;
  gPortB.rxComplete = FW_FALSE;
  /* Initialize Endpoints */
  gPortB.epBlkO = USB_CDD_EP_BLK_O;
  gPortB.epBlkI = USB_CDD_EP_BLK_I;
  /* Initialize FIFOs */
  FIFO_Init(&gPortB.rxFifo, gPortB.rxBuffer, sizeof(gPortB.rxBuffer));
  FIFO_Init(&gPortB.txFifo, gPortB.txBuffer, sizeof(gPortB.txBuffer));
  gPortB.rxFifoPutCb = cdc_RxFifoPutB;
  gPortB.txFifoGetCb = cdc_TxFifoGetB;
  /* Initialize pointers */
  gPortB.pCommProp = &gCommPropB;
  gPortB.pFlowCtrlState = &gFlowCtrlStateB;
  gPortB.pSpecChars = &gSpecCharsB;
  gPortB.pSerialStatus = &gSerialStatusB;
  /* Initialize UART Number */
  gPortB.uart = UART2;
}