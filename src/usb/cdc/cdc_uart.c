#include <string.h>
#include "types.h"
#include "usb_definitions.h"
#include "usb_cdc_definitions.h"
#include "usb_descriptor.h"
#include "cdc_private.h"

//-----------------------------------------------------------------------------
/* Global Variables */

STATIC CDC_LINE_CODING   gLineCoding   = { 0 };
STATIC CDC_SERIAL_STATE  gNotification = { 0 };
STATIC CDC_PORT          gPort         = { 0 };

//
////-----------------------------------------------------------------------------
///* Private Types definitions */
//
//typedef FW_BOOLEAN (*CDC_EP_FUNCTION)(void);
//typedef U32 (*CDC_EP_IRQ_FUNCTION)(U8 *pData, U32 aSize);
//typedef U32 (*CDC_EP_DATA_FUNCTION)(USBD_CbByte pPutByteCb, U32 aSize);
//
///* CDC Port Context */
//typedef struct _CDC_PORT
//{
//  CDC_EP_DATA_FUNCTION epOBlkRd;
//  CDC_EP_FUNCTION      epOBlkIsRxEmpty;
//  CDC_EP_DATA_FUNCTION epIBlkWr;
//  CDC_EP_FUNCTION      epIBlkIsTxEmpty;
//  CDC_EP_IRQ_FUNCTION  epIIrqWr;
//  U8                   irqBuffLen;
//  FW_BOOLEAN           rxComplete;
//  FIFO_t               rxFifo;
//  FIFO_t               txFifo;
//  USB_CbByte           rxFifoPutCb;
//  USB_CbByte           txFifoGetCb;
//  U8                   rxBuffer[USB_CDC_PACKET_SIZE * 4 + 1];
//  U8                   txBuffer[USB_CDC_PACKET_SIZE * 4 + 1];
//  U8                  *irqBuff;
//  CDC_LINE_CODING     *lineCoding;
//  CDC_SERIAL_STATE    *notification;
//  UART_t               uart;
//  FW_BOOLEAN           ready;
//} CDC_PORT;
//
////-----------------------------------------------------------------------------
///* Global Variables */
//
//STATIC CDC_LINE_CODING   gLineCodingI2C    = { 0 };
//STATIC CDC_SERIAL_STATE  gNotificationI2C  = { 0 };
//STATIC CDC_PORT          gPortI2C          = { 0 };
//STATIC CDC_LINE_CODING   gLineCodingSPI    = { 0 };
//STATIC CDC_SERIAL_STATE  gNotificationSPI  = { 0 };
//STATIC CDC_PORT          gPortSPI          = { 0 };
//
////-----------------------------------------------------------------------------
///* Private Functions declarations */
//
//

//
////-----------------------------------------------------------------------------
///* Private Functions declarations */
//
//static FW_BOOLEAN uart_FifoPutA(U8 * pByte);
//static FW_BOOLEAN uart_FifoGetA(U8 * pByte);
//static FW_BOOLEAN uart_RxCompleteA(U8 * pByte);
//static FW_BOOLEAN uart_TxCompleteA(U8 * pByte);
//
////-----------------------------------------------------------------------------
///** @brief Initializes the UART
// *  @param pUART - UART Number
// *  @return None
// */
//
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
//      gPortUART.lineCoding->dwBaudRate,
//      uart_FifoPutA,
//      uart_RxCompleteA,
//      uart_FifoGetA,
//      uart_TxCompleteA
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
//      gPortUART.lineCoding->dwBaudRate,
//      uart_FifoPutB,
//      NULL,
//      uart_FifoGetB,
//      NULL
//    );
//
//    /* UART2: PA2 - Tx, PA3 - Rx */
//    GPIO_Init(UART2_TX_PORT, UART2_TX_PIN, GPIO_TYPE_ALT_PP_10MHZ, 1);
//    GPIO_Init(UART2_RX_PORT, UART2_RX_PIN, GPIO_TYPE_IN_PUP_PDN,   1);
//
//    UART_RxStart(UART2);
//  }
//}
//
////-----------------------------------------------------------------------------
///** @brief Sets DTR/RTS signals
// *  @param pUART - UART Number
// *  @param aValue - DTR/RTS signals state
// *  @return None
// */
//
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
//
////-----------------------------------------------------------------------------
///** @brief Processes OUT EP data (Rx via USB)
// *  @param pPort - Pointer to Port context
// *  @return None
// */
//
//static void cdc_OutStage(CDC_PORT * pPort)
//{
//  /* Read from OUT EP */
//  (void)pPort->epOBlkRd(pPort->rxFifoPutCb, FIFO_Free(&pPort->rxFifo));
//
//  /* Write to UART */
//  if (0 < FIFO_Count(&pPort->rxFifo))
//  {
//    UART_TxStart(pPort->uart);
//  }
//}
//
////-----------------------------------------------------------------------------
///** @brief Processes IN EP data (Tx via USB)
// *  @param pPort - Pointer to Port context
// *  @return None
// */
//
//static void cdc_InStage(CDC_PORT * pPort)
//{
//  /* If there are some data in FIFO */
//  if (0 < FIFO_Count(&pPort->txFifo))
//  {
//    /* Write to IN EP */
//    (void)pPort->epIBlkWr(pPort->txFifoGetCb, FIFO_Count(&pPort->txFifo));
//  }
//  else
//  {
//    pPort->rxComplete = FW_FALSE;
//  }
//}
//
////-----------------------------------------------------------------------------
///** @brief Processes Rx/Tx data if present in I/O Buffers
// *  @param pPort - Pointer to the Port context
// *  @return None
// */
//
//static void cdc_ProcessCollectedData(CDC_PORT * pPort)
//{
//  /* Check if there are some unprocessed data */
//  if (FW_TRUE == pPort->ready)
//  {
//    if (FW_FALSE == pPort->epOBlkIsRxEmpty())
//    {
//      cdc_OutStage(pPort);
//    }
//
//    if (((FW_TRUE == pPort->rxComplete) ||
//         (USB_CDC_PACKET_SIZE < FIFO_Count(&pPort->txFifo))) &&
//        (FW_TRUE == pPort->epIBlkIsTxEmpty()))
//    {
//      cdc_InStage(pPort);
//    }
//  }
//}
//
////-----------------------------------------------------------------------------
///** @brief Puts received Byte from USB EP buffer to the Rx FIFO
// *  @param pByte - Pointer to the container for Byte
// *  @return None
// */
//
//static void cdc_RxFifoPutA(U8 * pByte)
//{
//  (void)FIFO_Put(&gPortUART.rxFifo, pByte);
//}
//
////-----------------------------------------------------------------------------
///** @brief Gets Byte that need to be transmitted from the Tx FIFO
// *  @param pByte - Pointer to the container for Byte
// *  @return None
// */
//
//static void cdc_TxFifoGetA(U8 * pByte)
//{
//  (void)FIFO_Get(&gPortUART.txFifo, pByte);
//}
//
////-----------------------------------------------------------------------------
///** @brief Puts received Byte from UART to the Tx FIFO
// *  @param pByte - Pointer to the container for Byte
// *  @return None
// */
//
//static FW_BOOLEAN uart_FifoPutA(U8 * pByte)
//{
//  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Put(&gPortUART.txFifo, pByte));
//}
//
////-----------------------------------------------------------------------------
///** @brief Receive complete callback
// *  @param pByte - Optional pointer to the latest received byte (def. NULL)
// *  @return TRUE, that means UART line idle is received
// */
//
//static FW_BOOLEAN uart_RxCompleteA(U8 * pByte)
//{
//  gPortUART.rxComplete = FW_TRUE;
//  return FW_TRUE;
//}
//
////-----------------------------------------------------------------------------
///** @brief Gets Byte that need to be transmitted from the Rx FIFO
// *  @param pByte - Pointer to the container for Byte
// *  @return TRUE if byte has been gotten successfully
// */
//
//static FW_BOOLEAN uart_FifoGetA(U8 * pByte)
//{
//  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Get(&gPortUART.rxFifo, pByte));
//}
//
////-----------------------------------------------------------------------------
///** @brief Transmit complete callback
// *  @param pByte - Optional pointer to the latest received byte (def. NULL)
// *  @return TRUE, that means UART transmission is complete
// */
//
//static FW_BOOLEAN uart_TxCompleteA(U8 * pByte)
//{
//  return FW_TRUE;
//}


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

//  /* Port is not ready yet */
//  gPortUART.ready = FW_FALSE;
//  gPortUART.rxComplete = FW_FALSE;
//  /* Initialize Endpoints */
//  gPortUART.epOBlkRd        = USBD_CDC_UART_OEndPointRdWsCb;
//  gPortUART.epOBlkIsRxEmpty = USBD_CDC_UART_OEndPointIsRxEmpty;
//  gPortUART.epIBlkWr        = USBD_CDC_UART_IEndPointWrWsCb;
//  gPortUART.epOBlkIsRxEmpty = USBD_CDC_UART_IEndPointIsTxEmpty;
//  gPortUART.epIIrqWr        = USBD_CDC_UART_IrqEndPointWr;
//  /* Initialize FIFOs */
//  FIFO_Init(&gPortUART.rxFifo, gPortUART.rxBuffer, sizeof(gPortUART.rxBuffer));
//  FIFO_Init(&gPortUART.txFifo, gPortUART.txBuffer, sizeof(gPortUART.txBuffer));
//  gPortUART.rxFifoPutCb = cdc_RxFifoPutA;
//  gPortUART.txFifoGetCb = cdc_TxFifoGetA;

  /* Initialize pointers */
  gPort.lineCoding = &gLineCoding;
  gPort.notification = &gNotification;

//  /* Initialize UART Number */
//  gPortUART.uart = UART1;
}

//-----------------------------------------------------------------------------
/** @brief Returns pointer to CDC UART instance
 *  @param None
 *  @return CDC UART instance
 */

CDC_PORT * CDC_UART_GetPort(void)
{
  return &gPort;
}
