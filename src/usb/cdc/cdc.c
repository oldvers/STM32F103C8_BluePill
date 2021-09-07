#include "types.h"

#include "usb.h"
#include "usb_definitions.h"
#include "usb_descriptor.h"
#include "usb_cdc_definitions.h"
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

#define CDC_CTRL_LINE_STATE_DTR   (0)
#define CDC_CTRL_LINE_STATE_RTS   (1)

#define CDC_DEBUG

#ifdef CDC_DEBUG
#  define CDC_LOG    DBG
#else
#  define CDC_LOG(...)
#endif

//-----------------------------------------------------------------------------
/* Private Types definitions */
/* Line Coding Structure */
typedef __packed struct _CDC_LINE_CODING
{
  U32 dwBaudRate;       /* Number Data terminal rate, in bits per second */
  U8  bCharFormat;      /* Number of Stop bits */
                        /*   0 - 1 Stop bit    *
                         *   1 - 1.5 Stop bits *
                         *   2 - 2 Stop bits   */
  U8  bParityType;      /* Number Parity */
                        /*   0 - None    *
                         *   1 - Odd     *
                         *   2 - Even    *
                         *   3 - Mark    *
                         *   4 - Space   */
  U8  bDataBits;        /* Number Data Bits (5, 6, 7, 8 or 16) */
} CDC_LINE_CODING;

/* Serial State Notification Structure */
typedef __packed struct _CDC_SERIAL_STATE
{
  REQUEST_TYPE bmRequestType;
  U8  bNotification;
  U16 wValue;
  U16 wIndex;
  U16 wLength;
  __packed union
  {
    U16 Raw;
    __packed struct
    {
      U16 bRxCarrier : 1;
      U16 bTxCarrier : 1;
      U16 bBreak : 1;
      U16 bRingSignal : 1;
      U16 bFraming : 1;
      U16 bParity : 1;
      U16 bOverRun : 1;
    } Bit;
  } Data;
} CDC_SERIAL_STATE;

typedef FW_BOOLEAN (*CDC_EP_FUNCTION)(void);
typedef U32 (*CDC_EP_IRQ_FUNCTION)(U8 *pData, U32 aSize);
typedef U32 (*CDC_EP_DATA_FUNCTION)(USBD_CbByte pPutByteCb, U32 aSize);

/* CDC Port Context */
typedef struct _CDC_PORT
{
  CDC_EP_DATA_FUNCTION epOBlkRd;
  CDC_EP_FUNCTION      epOBlkIsRxEmpty;
  CDC_EP_DATA_FUNCTION epIBlkWr;
  CDC_EP_FUNCTION      epIBlkIsTxEmpty;
  CDC_EP_IRQ_FUNCTION  epIIrqWr;
  U8                   irqBuffLen;
  FW_BOOLEAN           rxComplete;
  FIFO_t               rxFifo;
  FIFO_t               txFifo;
  USB_CbByte           rxFifoPutCb;
  USB_CbByte           txFifoGetCb;
  U8                   rxBuffer[USB_CDC_PACKET_SIZE * 4 + 1];
  U8                   txBuffer[USB_CDC_PACKET_SIZE * 4 + 1];
  U8                  *irqBuff;
  CDC_LINE_CODING     *lineCoding;
  CDC_SERIAL_STATE    *notification;
  UART_t               uart;
  FW_BOOLEAN           ready;
} CDC_PORT;

//-----------------------------------------------------------------------------
/* Global Variables */

STATIC CDC_LINE_CODING   gLineCodingA   = { 0 };
STATIC CDC_SERIAL_STATE  gNotificationA = { 0 };
STATIC CDC_PORT          gPortA         = { 0 };
STATIC CDC_LINE_CODING   gLineCodingB   = { 0 };
STATIC CDC_SERIAL_STATE  gNotificationB = { 0 };
STATIC CDC_PORT          gPortB         = { 0 };

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
/** @brief Initializes the UART
 *  @param pUART - UART Number
 *  @return None
 */

static void uart_Init(UART_t aUART)
{
  if (UART1 == aUART)
  {
    /* UART1: PA9 - Tx, PA10 - Rx, DTR - PB8, RTS - PB6 */
    GPIO_Init(UART1_TX_PORT,  UART1_TX_PIN,  GPIO_TYPE_OUT_PP_10MHZ, 1);
    GPIO_Init(UART1_RX_PORT,  UART1_RX_PIN,  GPIO_TYPE_IN_PUP_PDN,   1);
    GPIO_Init(UART1_DTR_PORT, UART1_DTR_PIN, GPIO_TYPE_IN_PUP_PDN,   1);
    GPIO_Init(UART1_RTS_PORT, UART1_RTS_PIN, GPIO_TYPE_IN_PUP_PDN,   1);

    UART_DeInit(UART1);
    UART_Init
    (
      UART1,
      gPortA.lineCoding->dwBaudRate,
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

    UART_RxStart(UART1);
  }
  else
  {
    /* UART2: PA2 - Tx, PA3 - Rx */
    GPIO_Init(UART2_TX_PORT, UART2_TX_PIN, GPIO_TYPE_IN_PUP_PDN, 1);
    GPIO_Init(UART2_RX_PORT, UART2_RX_PIN, GPIO_TYPE_IN_PUP_PDN, 1);

    UART_DeInit(UART2);
    UART_Init
    (
      UART2,
      gPortB.lineCoding->dwBaudRate,
      uart_FifoPutB,
      uart_RxCompleteB,
      uart_FifoGetB,
      uart_TxCompleteB
    );

    /* UART2: PA2 - Tx, PA3 - Rx */
    GPIO_Init(UART2_TX_PORT, UART2_TX_PIN, GPIO_TYPE_ALT_PP_10MHZ, 1);
    GPIO_Init(UART2_RX_PORT, UART2_RX_PIN, GPIO_TYPE_IN_PUP_PDN,   1);

    UART_RxStart(UART2);
  }
}

//-----------------------------------------------------------------------------
/** @brief Gets CDC Port according to USB Interface Number
 *  @param aInterface - USB Interface Number
 *  @return Pointer to the CDC Port Context
 */

static CDC_PORT * cdc_GetPort(U16 aInterface)
{
  CDC_PORT * result = &gPortA;

  if (USBD_CDC_GetInterfaceNumber() == aInterface)
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

static void uart_DTR_RTS_Set(UART_t aUART, U16 aValue)
{
  if (UART1 == aUART)
  {
    /* DTR signal */
    if ( 0 == (aValue & (1 << CDC_CTRL_LINE_STATE_DTR)) )
    {
      GPIO_Hi(UART1_DTR_PORT, UART1_DTR_PIN);
    }
    else
    {
      GPIO_Lo(UART1_DTR_PORT, UART1_DTR_PIN);
    }

    /* RTS signal */
    if ( 0 == (aValue & (1 << CDC_CTRL_LINE_STATE_RTS)) )
    {
      GPIO_Hi(UART1_RTS_PORT, UART1_RTS_PIN);
    }
    else
    {
      GPIO_Lo(UART1_RTS_PORT, UART1_RTS_PIN);
    }
  }
}

//-----------------------------------------------------------------------------
/** @brief Processes IRQ EP data
 *  @param pPort - Pointer to Port context
 *  @return None
 */

static void cdc_IrqInStage(CDC_PORT * pPort)
{
  U8 len = 0;

  if (0 == pPort->irqBuffLen) return;

  if (USB_CDC_IRQ_PACKET_SIZE < pPort->irqBuffLen)
  {
    len = USB_CDC_IRQ_PACKET_SIZE;
  }
  else
  {
    len = pPort->irqBuffLen;
  }

  CDC_LOG("CDC IRQ IN: len = %d\r\n", len);
  (void)pPort->epIIrqWr(pPort->irqBuff, len);

  pPort->irqBuff += len;
  pPort->irqBuffLen -= len;
}

//-----------------------------------------------------------------------------
/** @brief Sends Serial State notification
 *  @param pPort - Pointer to Port context
 *  @param aState - Errors/Evetns state
 *  @return None
 */

void cdc_NotifyState(CDC_PORT * pPort, U16 aState)
{
  pPort->notification->Data.Raw = aState;
  pPort->irqBuff = (U8 *)pPort->notification;
  pPort->irqBuffLen = sizeof(CDC_SERIAL_STATE);

  cdc_IrqInStage(pPort);
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
  CDC_PORT * port;

  switch (pSetup->bRequest)
  {
    case CDC_REQ_SET_LINE_CODING:
      CDC_LOG("CDC Setup: SetLineCoding: IF = %d L = %d\r\n",
              pSetup->wIndex.W, *pSize);

      port = cdc_GetPort(pSetup->wIndex.W);
      *pData = (U8 *)port->lineCoding;

      result = USB_CTRL_STAGE_WAIT;
      break;

    case CDC_REQ_GET_LINE_CODING:
      CDC_LOG("CDC Setup: GetLineCoding: IF = %d\r\n", pSetup->wIndex.W);

      port = cdc_GetPort(pSetup->wIndex.W);
      *pData = (U8 *)port->lineCoding;

      result = USB_CTRL_STAGE_DATA;
      break;

    case CDC_REQ_SET_CONTROL_LINE_STATE:
      CDC_LOG("CDC Setup: Set Ctrl Line State: IF = %d Val = %04X\r\n",
              pSetup->wIndex.W, pSetup->wValue.W);

      port = cdc_GetPort(pSetup->wIndex.W);
      uart_DTR_RTS_Set(port->uart, pSetup->wValue.W);

      result = USB_CTRL_STAGE_STATUS;
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
  CDC_PORT * port = &gPortA;

  switch (pSetup->bRequest)
  {
    case CDC_REQ_SET_LINE_CODING:
      port = cdc_GetPort(pSetup->wIndex.W);
      GPIO_Hi(GPIOB, 3);
      uart_Init(port->uart);
      GPIO_Lo(GPIOB, 3);
      port->ready = FW_TRUE;

      CDC_LOG("CDC Out: Set Line Coding: IF = %d Baud = %d, Len = %d\r\n",
              pSetup->wIndex.W, port->lineCoding->dwBaudRate, *pSize);

      result = USB_CTRL_STAGE_STATUS;
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
  (void)pPort->epOBlkRd(pPort->rxFifoPutCb, FIFO_Free(&pPort->rxFifo));

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
    (void)pPort->epIBlkWr(pPort->txFifoGetCb, FIFO_Count(&pPort->txFifo));
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
    if (FW_FALSE == pPort->epOBlkIsRxEmpty())
    {
      cdc_OutStage(pPort);
    }

    if (((FW_TRUE == pPort->rxComplete) ||
         (USB_CDC_PACKET_SIZE < FIFO_Count(&pPort->txFifo))) &&
        (FW_TRUE == pPort->epIBlkIsTxEmpty()))
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
  cdc_ProcessCollectedData(&gPortB);
}

//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_InterruptAIn(U32 aEvent)
{
  cdc_IrqInStage(&gPortA);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_BulkAIn(U32 aEvent)
{
  cdc_InStage(&gPortA);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_BulkAOut(U32 aEvent)
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
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_InterruptBIn(U32 aEvent)
{
  cdc_IrqInStage(&gPortB);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_BulkBIn(U32 aEvent)
{
  cdc_InStage(&gPortB);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_BulkBOut(U32 aEvent)
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
  /* Init Line Coding */
  gLineCodingA.dwBaudRate  = 115200;
  gLineCodingA.bCharFormat = 0;
  gLineCodingA.bParityType = 0;
  gLineCodingA.bDataBits   = 8;
  /* Init Notification */
  gNotificationA.bmRequestType.BM.Recipient = REQUEST_TO_INTERFACE;
  gNotificationA.bmRequestType.BM.Type      = REQUEST_CLASS;
  gNotificationA.bmRequestType.BM.Dir       = REQUEST_DEVICE_TO_HOST;
  gNotificationA.bNotification              = CDC_NTF_SERIAL_STATE;
  gNotificationA.wValue                     = 0;
  gNotificationA.wIndex                     = USBD_CDC_GetInterfaceNumber();
  gNotificationA.wLength                    = 2;
  gNotificationA.Data.Raw                   = 0;
  /* Clear Port context */
  memset(&gPortA, 0, sizeof(gPortA));
  /* Port is not ready yet */
  gPortA.ready = FW_FALSE;
  gPortA.rxComplete = FW_FALSE;
  /* Initialize Endpoints */
  gPortA.epOBlkRd        = USBD_CDC_OEndPointRdWsCb;
  gPortA.epOBlkIsRxEmpty = USBD_CDC_OEndPointIsRxEmpty;
  gPortA.epIBlkWr        = USBD_CDC_IEndPointWrWsCb;
  gPortA.epIBlkIsTxEmpty = USBD_CDC_IEndPointIsTxEmpty;
  gPortA.epIIrqWr        = USBD_CDC_IrqEndPointWr;
  /* Initialize FIFOs */
  FIFO_Init(&gPortA.rxFifo, gPortA.rxBuffer, sizeof(gPortA.rxBuffer));
  FIFO_Init(&gPortA.txFifo, gPortA.txBuffer, sizeof(gPortA.txBuffer));
  gPortA.rxFifoPutCb = cdc_RxFifoPutA;
  gPortA.txFifoGetCb = cdc_TxFifoGetA;
  /* Initialize pointers */
  gPortA.lineCoding = &gLineCodingA;
  gPortA.notification = &gNotificationA;
  /* Initialize UART Number */
  gPortA.uart = UART1;

  /* Init Line Coding */
  gLineCodingB.dwBaudRate  = 115200;
  gLineCodingB.bCharFormat = 0;
  gLineCodingB.bParityType = 0;
  gLineCodingB.bDataBits   = 8;
  /* Init Notification */
  gNotificationB.bmRequestType.BM.Recipient = REQUEST_TO_INTERFACE;
  gNotificationB.bmRequestType.BM.Type      = REQUEST_CLASS;
  gNotificationB.bmRequestType.BM.Dir       = REQUEST_DEVICE_TO_HOST;
  gNotificationB.bNotification              = CDC_NTF_SERIAL_STATE;
  gNotificationB.wValue                     = 0;
  gNotificationB.wIndex                     = USBD_CDD_GetInterfaceNumber();
  gNotificationB.wLength                    = 2;
  gNotificationB.Data.Raw                   = 0;
  /* Clear Port context */
  memset(&gPortB, 0, sizeof(gPortB));
  /* Port is not ready yet */
  gPortB.ready = FW_FALSE;
  gPortB.rxComplete = FW_FALSE;
  /* Initialize Endpoints */
  gPortB.epOBlkRd        = USBD_CDD_OEndPointRdWsCb;
  gPortB.epOBlkIsRxEmpty = USBD_CDD_OEndPointIsRxEmpty;
  gPortB.epIBlkWr        = USBD_CDD_IEndPointWrWsCb;
  gPortB.epIBlkIsTxEmpty = USBD_CDD_IEndPointIsTxEmpty;
  gPortB.epIIrqWr        = USBD_CDD_IrqEndPointWr;
  /* Initialize FIFOs */
  FIFO_Init(&gPortB.rxFifo, gPortB.rxBuffer, sizeof(gPortB.rxBuffer));
  FIFO_Init(&gPortB.txFifo, gPortB.txBuffer, sizeof(gPortB.txBuffer));
  gPortB.rxFifoPutCb = cdc_RxFifoPutB;
  gPortB.txFifoGetCb = cdc_TxFifoGetB;
  /* Initialize pointers */
  gPortB.lineCoding = &gLineCodingB;
  gPortB.notification = &gNotificationB;
  /* Initialize UART Number */
  gPortB.uart = UART2;
}