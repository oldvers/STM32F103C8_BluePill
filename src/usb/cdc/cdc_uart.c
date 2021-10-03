#include <string.h>
#include "types.h"
#include "usb_definitions.h"
#include "usb_cdc_definitions.h"
#include "usb_descriptor.h"
#include "cdc.h"
#include "cdc_private.h"

#include "board.h"
#include "gpio.h"
#include "uart.h"
#include "interrupts.h"
#include "usb.h"

#include "fifo.h"

//-----------------------------------------------------------------------------
/* Private Types definitions */

typedef FW_BOOLEAN (*CDC_EP_FUNCTION)(void);
typedef U32 (*CDC_EP_DATA_FUNCTION)(USBD_CbByte pPutByteCb, U32 aSize);

/* CDC Port Context */
typedef struct _CDC_UART_PORT
{
  CDC_PORT             cdc;
  CDC_EP_DATA_FUNCTION fpEpOBlkRd;
  CDC_EP_FUNCTION      fpEpOBlkIsRxEmpty;
  CDC_EP_DATA_FUNCTION fpEpIBlkWr;
  CDC_EP_FUNCTION      fpEpIBlkIsTxEmpty;
  UART_t               uart;
  FW_BOOLEAN           rxComplete;
  FIFO_p               rxFifo;
  FIFO_p               txFifo;
  USB_CbByte           fpRxFifoPut;
  USB_CbByte           fpTxFifoGet;
  U8                   rxBuffer[USB_CDC_PACKET_SIZE * 5 + 1];
  U8                   txBuffer[USB_CDC_PACKET_SIZE * 5 + 1];
} CDC_UART_PORT;

//-----------------------------------------------------------------------------
/* Global Variables */

STATIC CDC_LINE_CODING   gLineCoding   = { 0 };
STATIC CDC_SERIAL_STATE  gNotification = { 0 };
STATIC CDC_UART_PORT     gPort         = { 0 };

//-----------------------------------------------------------------------------
/* Private Functions declarations */

static FW_BOOLEAN uart_FifoPut(U8 * pByte);
static FW_BOOLEAN uart_FifoGet(U8 * pByte);
static FW_BOOLEAN uart_RxComplete(U8 * pByte);
static FW_BOOLEAN uart_TxComplete(U8 * pByte);

//-----------------------------------------------------------------------------
/** @brief Initializes the UART
 *  @param pUART - UART Number
 *  @return None
 */

static void uart_Open(void)
{
  /* UART3: PB10 - Tx, PB11 - Rx */
  GPIO_Init(UART3_TX_PORT, UART3_TX_PIN, GPIO_TYPE_IN_PUP_PDN, 1);
  GPIO_Init(UART3_RX_PORT, UART3_RX_PIN, GPIO_TYPE_IN_PUP_PDN, 1);

  UART_DeInit(UART3);
  UART_Init
  (
    UART3,
    gPort.cdc.lineCoding->dwBaudRate,
    uart_FifoPut,
    uart_RxComplete,
    uart_FifoGet,
    uart_TxComplete
  );

  /* UART3: PB10 - Tx, PB11 - Rx */
  GPIO_Init(UART3_TX_PORT, UART3_TX_PIN, GPIO_TYPE_ALT_PP_10MHZ, 1);
  GPIO_Init(UART3_RX_PORT, UART3_RX_PIN, GPIO_TYPE_IN_PUP_PDN,   1);

  UART_RxStart(UART3);
}

//-----------------------------------------------------------------------------
/** @brief Sets DTR/RTS signals
 *  @param pUART - UART Number
 *  @param aValue - DTR/RTS signals state
 *  @return None
 */

static void uart_DTR_RTS_Set(U16 aValue)
{
  /* There are no DTR/RTS signals for UART3 */
}

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from USB EP buffer to the Rx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static void cdc_RxFifoPut(U8 * pByte)
{
  (void)FIFO_Put(gPort.rxFifo, pByte);
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte that need to be transmitted from the Tx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static void cdc_TxFifoGet(U8 * pByte)
{
  (void)FIFO_Get(gPort.txFifo, pByte);
}

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from UART to the Tx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static FW_BOOLEAN uart_FifoPut(U8 * pByte)
{
  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Put(gPort.txFifo, pByte));
}

//-----------------------------------------------------------------------------
/** @brief Receive complete callback
 *  @param pByte - Optional pointer to the latest received byte (def. NULL)
 *  @return TRUE, that means UART line idle is received
 */

static FW_BOOLEAN uart_RxComplete(U8 * pByte)
{
  gPort.rxComplete = FW_TRUE;
  return FW_TRUE;
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte that need to be transmitted from the Rx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return TRUE if byte has been gotten successfully
 */

static FW_BOOLEAN uart_FifoGet(U8 * pByte)
{
  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Get(gPort.rxFifo, pByte));
}

//-----------------------------------------------------------------------------
/** @brief Transmit complete callback
 *  @param pByte - Optional pointer to the latest received byte (def. NULL)
 *  @return TRUE, that means UART transmission is complete
 */

static FW_BOOLEAN uart_TxComplete(U8 * pByte)
{
  return FW_TRUE;
}

//-----------------------------------------------------------------------------
/** @brief Processes OUT EP data (Rx via USB)
 *  @param None
 *  @return None
 */

void CDC_UART_OutStage(void)
{
  /* Read from OUT EP */
  (void)gPort.fpEpOBlkRd(gPort.fpRxFifoPut, FIFO_Free(gPort.rxFifo));

  /* Write to UART */
  if (0 < FIFO_Count(gPort.rxFifo))
  {
    UART_TxStart(UART3);
  }
}

//-----------------------------------------------------------------------------
/** @brief Processes IN EP data (Tx via USB)
 *  @param None
 *  @return None
 */

void CDC_UART_InStage(void)
{
  /* If there are some data in FIFO */
  if (0 < FIFO_Count(gPort.txFifo))
  {
    /* Write to IN EP */
    (void)gPort.fpEpIBlkWr(gPort.fpTxFifoGet, FIFO_Count(gPort.txFifo));
  }
  else
  {
    gPort.rxComplete = FW_FALSE;
  }
}

//-----------------------------------------------------------------------------
/** @brief Processes Rx/Tx data if present in I/O Buffers
 *  @param None
 *  @return None
 */

void CDC_UART_ProcessCollectedData(void)
{
  /* Check if there are some unprocessed data */
  if (FW_TRUE == gPort.cdc.ready)
  {
    if (FW_FALSE == gPort.fpEpOBlkIsRxEmpty())
    {
      CDC_UART_OutStage();
    }

    if (((FW_TRUE == gPort.rxComplete) ||
        (USB_CDC_PACKET_SIZE < FIFO_Count(gPort.txFifo))) &&
        (FW_TRUE == gPort.fpEpIBlkIsTxEmpty()))
    {
      CDC_UART_InStage();
    }
  }
}

//-----------------------------------------------------------------------------
/** @brief Initializes CDC UART
 *  @param None
 *  @return None
 */

void CDC_UART_Init(void)
{
  /* Init Line Coding */
  gLineCoding.dwBaudRate  = 115200;
  gLineCoding.bCharFormat = 0;
  gLineCoding.bParityType = 0;
  gLineCoding.bDataBits   = 8;

  /* Init Notification */
  gNotification.bmRequestType.BM.Recipient = REQUEST_TO_INTERFACE;
  gNotification.bmRequestType.BM.Type      = REQUEST_CLASS;
  gNotification.bmRequestType.BM.Dir       = REQUEST_DEVICE_TO_HOST;
  gNotification.bNotification              = CDC_NTF_SERIAL_STATE;
  gNotification.wValue                     = 0;
  gNotification.wIndex                     = USBD_CDC_UART_GetInterfaceNumber();
  gNotification.wLength                    = 2;
  gNotification.Data.Raw                   = 0;

  /* Clear Port context */
  memset(&gPort, 0, sizeof(gPort));

  /* Port is not ready yet */
  gPort.cdc.ready = FW_FALSE;
  gPort.rxComplete = FW_FALSE;

  /* Initialize Endpoints' callbacks */
  gPort.fpEpOBlkRd        = USBD_CDC_UART_OEndPointRdWsCb;
  gPort.fpEpOBlkIsRxEmpty = USBD_CDC_UART_OEndPointIsRxEmpty;
  gPort.fpEpIBlkWr        = USBD_CDC_UART_IEndPointWrWsCb;
  gPort.fpEpIBlkIsTxEmpty = USBD_CDC_UART_IEndPointIsTxEmpty;
  gPort.cdc.fpEpIIrqWr    = USBD_CDC_UART_IrqEndPointWr;
  gPort.cdc.fpOpen        = uart_Open;
  gPort.cdc.fpSetCtrlLine = uart_DTR_RTS_Set;

  /* Initialize FIFOs */
  gPort.rxFifo = FIFO_Init(gPort.rxBuffer, sizeof(gPort.rxBuffer));
  gPort.txFifo = FIFO_Init(gPort.txBuffer, sizeof(gPort.txBuffer));
  gPort.fpRxFifoPut = cdc_RxFifoPut;
  gPort.fpTxFifoGet = cdc_TxFifoGet;

  /* Initialize pointers */
  gPort.cdc.lineCoding = &gLineCoding;
  gPort.cdc.notification = &gNotification;

  /* Initialize UART Number */
  gPort.uart = UART3;
}

//-----------------------------------------------------------------------------
/** @brief Returns pointer to CDC UART instance
 *  @param None
 *  @return CDC UART instance
 */

CDC_PORT * CDC_UART_GetPort(void)
{
  return &gPort.cdc;
}
