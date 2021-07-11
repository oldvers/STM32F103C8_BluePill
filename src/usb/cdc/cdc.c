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

STATIC CDC_LINE_CODING   gLineCodingUART   = { 0 };
STATIC CDC_SERIAL_STATE  gNotificationUART = { 0 };
STATIC CDC_PORT          gPortUART         = { 0 };
STATIC CDC_LINE_CODING   gLineCodingI2C    = { 0 };
STATIC CDC_SERIAL_STATE  gNotificationI2C  = { 0 };
STATIC CDC_PORT          gPortI2C          = { 0 };
STATIC CDC_LINE_CODING   gLineCodingSPI    = { 0 };
STATIC CDC_SERIAL_STATE  gNotificationSPI  = { 0 };
STATIC CDC_PORT          gPortSPI          = { 0 };

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
      gPortUART.lineCoding->dwBaudRate,
      uart_FifoPutA,
      NULL,
      uart_FifoGetA,
      NULL
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
      gPortUART.lineCoding->dwBaudRate,
      uart_FifoPutB,
      NULL,
      uart_FifoGetB,
      NULL
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
  CDC_PORT * result = &gPortUART;

  if (USBD_CDC_UART_GetInterfaceNumber() == aInterface)
  {
    result = &gPortUART;
  }
  else
  {
    result = &gPortI2C;
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

  //LOG("CDC IRQ IN: len = %d\r\n", len);
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
      //LOG("CDC Setup: SetLineCoding: IF = %d L = %d\r\n",
      //      pSetup->wIndex.W, *pSize);

      port = cdc_GetPort(pSetup->wIndex.W);
      *pData = (U8 *)port->lineCoding;

      result = USB_CTRL_STAGE_WAIT;
      break;

    case CDC_REQ_GET_LINE_CODING:
      //LOG("CDC Setup: GetLineCoding: IF = %d\r\n", pSetup->wIndex.W);

      port = cdc_GetPort(pSetup->wIndex.W);
      *pData = (U8 *)port->lineCoding;

      result = USB_CTRL_STAGE_DATA;
      break;

    case CDC_REQ_SET_CONTROL_LINE_STATE:
      //LOG("CDC Setup: Set Ctrl Line State: IF = %d Val = %04X\r\n",
      //      pSetup->wIndex.W, pSetup->wValue.W);

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
  CDC_PORT * port = &gPortUART;

  switch (pSetup->bRequest)
  {
    case CDC_REQ_SET_LINE_CODING:
      port = cdc_GetPort(pSetup->wIndex.W);
      GPIO_Hi(GPIOB, 3);
      uart_Init(port->uart);
      GPIO_Lo(GPIOB, 3);
      port->ready = FW_TRUE;

      //LOG("CDC Out: Set Line Coding: IF = %d Baud = %d, Len = %d\r\n",
      //      pSetup->wIndex.W, port->lineCoding->dwBaudRate, *pSize);

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

    if (FW_TRUE == pPort->epIBlkIsTxEmpty())
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
  cdc_ProcessCollectedData(&gPortUART);
  cdc_ProcessCollectedData(&gPortI2C);
  cdc_ProcessCollectedData(&gPortSPI);
}

//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_UART_InterruptIn(U32 aEvent)
{
  cdc_IrqInStage(&gPortUART);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_UART_BulkIn(U32 aEvent)
{
  cdc_InStage(&gPortUART);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_UART_BulkOut(U32 aEvent)
{
  cdc_OutStage(&gPortUART);
}

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from USB EP buffer to the Rx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static void cdc_RxFifoPutA(U8 * pByte)
{
  (void)FIFO_Put(&gPortUART.rxFifo, pByte);
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte that need to be transmitted from the Tx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static void cdc_TxFifoGetA(U8 * pByte)
{
  (void)FIFO_Get(&gPortUART.txFifo, pByte);
}

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from UART to the Tx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static FW_BOOLEAN uart_FifoPutA(U8 * pByte)
{
  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Put(&gPortUART.txFifo, pByte));
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte that need to be transmitted from the Rx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static FW_BOOLEAN uart_FifoGetA(U8 * pByte)
{
  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Get(&gPortUART.rxFifo, pByte));
}

//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_I2C_InterruptIn(U32 aEvent)
{
  cdc_IrqInStage(&gPortI2C);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_I2C_BulkIn(U32 aEvent)
{
  cdc_InStage(&gPortI2C);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_I2C_BulkOut(U32 aEvent)
{
  cdc_OutStage(&gPortI2C);
}

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from USB EP buffer to the Rx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static void cdc_RxFifoPutB(U8 * pByte)
{
  //(void)FIFO_Put(&gPortB.rxFifo, pByte);
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte that need to be transmitted from the Tx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static void cdc_TxFifoGetB(U8 * pByte)
{
  //(void)FIFO_Get(&gPortB.txFifo, pByte);
}

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from UART to the Tx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static FW_BOOLEAN uart_FifoPutB(U8 * pByte)
{
  return FW_FALSE; //(FW_BOOLEAN)(FW_SUCCESS == FIFO_Put(&gPortB.txFifo, pByte));
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte that need to be transmitted from the Rx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static FW_BOOLEAN uart_FifoGetB(U8 * pByte)
{
  return FW_FALSE; //(FW_BOOLEAN)(FW_SUCCESS == FIFO_Get(&gPortB.rxFifo, pByte));
}



//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_SPI_InterruptIn(U32 aEvent)
{
  cdc_IrqInStage(&gPortSPI);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_SPI_BulkIn(U32 aEvent)
{
  cdc_InStage(&gPortSPI);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_SPI_BulkOut(U32 aEvent)
{
  cdc_OutStage(&gPortSPI);
}





//-----------------------------------------------------------------------------
/** @brief Initializes CDC
 *  @param None
 *  @return None
 */

void CDC_Init(void)
{
  /* Init Line Coding */
  gLineCodingUART.dwBaudRate  = 115200;
  gLineCodingUART.bCharFormat = 0;
  gLineCodingUART.bParityType = 0;
  gLineCodingUART.bDataBits   = 8;
  /* Init Notification */
  gNotificationUART.bmRequestType.BM.Recipient = REQUEST_TO_INTERFACE;
  gNotificationUART.bmRequestType.BM.Type      = REQUEST_CLASS;
  gNotificationUART.bmRequestType.BM.Dir       = REQUEST_DEVICE_TO_HOST;
  gNotificationUART.bNotification              = CDC_NTF_SERIAL_STATE;
  gNotificationUART.wValue                     = 0;
  gNotificationUART.wIndex                     = USBD_CDC_UART_GetInterfaceNumber();
  gNotificationUART.wLength                    = 2;
  gNotificationUART.Data.Raw                   = 0;
  /* Clear Port context */
  memset(&gPortUART, 0, sizeof(gPortUART));
  /* Port is not ready yet */
  gPortUART.ready = FW_FALSE;
  /* Initialize Endpoints */
  gPortUART.epOBlkRd        = USBD_CDC_UART_OEndPointRdWsCb;
  gPortUART.epOBlkIsRxEmpty = USBD_CDC_UART_OEndPointIsRxEmpty;
  gPortUART.epIBlkWr        = USBD_CDC_UART_IEndPointWrWsCb;
  gPortUART.epOBlkIsRxEmpty = USBD_CDC_UART_IEndPointIsTxEmpty;
  gPortUART.epIIrqWr        = USBD_CDC_UART_IrqEndPointWr;
  /* Initialize FIFOs */
  FIFO_Init(&gPortUART.rxFifo, gPortUART.rxBuffer, sizeof(gPortUART.rxBuffer));
  FIFO_Init(&gPortUART.txFifo, gPortUART.txBuffer, sizeof(gPortUART.txBuffer));
  gPortUART.rxFifoPutCb = cdc_RxFifoPutA;
  gPortUART.txFifoGetCb = cdc_TxFifoGetA;
  /* Initialize pointers */
  gPortUART.lineCoding = &gLineCodingUART;
  gPortUART.notification = &gNotificationUART;
  /* Initialize UART Number */
  gPortUART.uart = UART1;

  /* Init Line Coding */
  gLineCodingI2C.dwBaudRate  = 115200;
  gLineCodingI2C.bCharFormat = 0;
  gLineCodingI2C.bParityType = 0;
  gLineCodingI2C.bDataBits   = 8;
  /* Init Notification */
  gNotificationI2C.bmRequestType.BM.Recipient = REQUEST_TO_INTERFACE;
  gNotificationI2C.bmRequestType.BM.Type      = REQUEST_CLASS;
  gNotificationI2C.bmRequestType.BM.Dir       = REQUEST_DEVICE_TO_HOST;
  gNotificationI2C.bNotification              = CDC_NTF_SERIAL_STATE;
  gNotificationI2C.wValue                     = 0;
  gNotificationI2C.wIndex                     = USBD_CDC_I2C_GetInterfaceNumber();
  gNotificationI2C.wLength                    = 2;
  gNotificationI2C.Data.Raw                   = 0;
  /* Clear Port context */
  memset(&gPortI2C, 0, sizeof(gPortI2C));
  /* Port is not ready yet */
  gPortI2C.ready = FW_FALSE;
  /* Initialize Endpoints */
  gPortI2C.epOBlkRd        = USBD_CDC_I2C_OEndPointRdWsCb;
  gPortI2C.epOBlkIsRxEmpty = USBD_CDC_I2C_OEndPointIsRxEmpty;
  gPortI2C.epIBlkWr        = USBD_CDC_I2C_IEndPointWrWsCb;
  gPortI2C.epIBlkIsTxEmpty = USBD_CDC_I2C_IEndPointIsTxEmpty;
  gPortI2C.epIIrqWr        = USBD_CDC_I2C_IrqEndPointWr;
  /* Initialize FIFOs */
  FIFO_Init(&gPortI2C.rxFifo, gPortI2C.rxBuffer, sizeof(gPortI2C.rxBuffer));
  FIFO_Init(&gPortI2C.txFifo, gPortI2C.txBuffer, sizeof(gPortI2C.txBuffer));
  gPortI2C.rxFifoPutCb = cdc_RxFifoPutB;
  gPortI2C.txFifoGetCb = cdc_TxFifoGetB;
  /* Initialize pointers */
  gPortI2C.lineCoding = &gLineCodingI2C;
  gPortI2C.notification = &gNotificationI2C;
  /* Initialize UART Number */
  gPortI2C.uart = UART2;


  /* Init Line Coding */
  gLineCodingSPI.dwBaudRate  = 115200;
  gLineCodingSPI.bCharFormat = 0;
  gLineCodingSPI.bParityType = 0;
  gLineCodingSPI.bDataBits   = 8;
  /* Init Notification */
  gNotificationSPI.bmRequestType.BM.Recipient = REQUEST_TO_INTERFACE;
  gNotificationSPI.bmRequestType.BM.Type      = REQUEST_CLASS;
  gNotificationSPI.bmRequestType.BM.Dir       = REQUEST_DEVICE_TO_HOST;
  gNotificationSPI.bNotification              = CDC_NTF_SERIAL_STATE;
  gNotificationSPI.wValue                     = 0;
  gNotificationSPI.wIndex                     = USBD_CDC_SPI_GetInterfaceNumber();
  gNotificationSPI.wLength                    = 2;
  gNotificationSPI.Data.Raw                   = 0;
  /* Clear Port context */
  memset(&gPortSPI, 0, sizeof(gPortSPI));
  /* Port is not ready yet */
  gPortSPI.ready = FW_FALSE;
  /* Initialize Endpoints */
  gPortSPI.epOBlkRd        = USBD_CDC_SPI_OEndPointRdWsCb;
  gPortSPI.epOBlkIsRxEmpty = USBD_CDC_SPI_OEndPointIsRxEmpty;
  gPortSPI.epIBlkWr        = USBD_CDC_SPI_IEndPointWrWsCb;
  gPortSPI.epIBlkIsTxEmpty = USBD_CDC_SPI_IEndPointIsTxEmpty;
  gPortSPI.epIIrqWr        = USBD_CDC_SPI_IrqEndPointWr;
  /* Initialize FIFOs */
  FIFO_Init(&gPortSPI.rxFifo, gPortSPI.rxBuffer, sizeof(gPortSPI.rxBuffer));
  FIFO_Init(&gPortSPI.txFifo, gPortSPI.txBuffer, sizeof(gPortSPI.txBuffer));
  gPortSPI.rxFifoPutCb = cdc_RxFifoPutB;
  gPortSPI.txFifoGetCb = cdc_TxFifoGetB;
  /* Initialize pointers */
  gPortSPI.lineCoding = &gLineCodingSPI;
  gPortSPI.notification = &gNotificationSPI;
  /* Initialize UART Number */
  gPortSPI.uart = UART2;
}