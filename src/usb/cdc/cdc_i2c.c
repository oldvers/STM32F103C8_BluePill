#include <string.h>
#include "types.h"
#include "usb_definitions.h"
#include "usb_cdc_definitions.h"
#include "usb_descriptor.h"
#include "cdc_private.h"
#include "cdc_i2c.h"

#include "board.h"
//#include "gpio.h"
//#include "uart.h"
#include "interrupts.h"
#include "usb.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "east_message.h"
#include "block_queue.h"

//-----------------------------------------------------------------------------
/* Private Types definitions */

#define EAST_MAX_DATA_LENGTH   (32)
#define EAST_HEADER_LENGTH     (6)
#define EAST_MAX_PACKET_LENGTH (EAST_HEADER_LENGTH + EAST_MAX_DATA_LENGTH)

#define EVT_EAST_TX_COMPLETE   (1 << 0)
#define EVT_I2C_EXCH_COMPLETE  (1 << 1)


typedef FW_BOOLEAN (*CDC_EP_FUNCTION)(void);
typedef U32 (*CDC_EP_DATA_FUNCTION)(USBD_CbByte pPutByteCb, U32 aSize);

/* CDC Port Context */
typedef struct _CDC_I2C_PORT
{
  CDC_PORT             cdc;
  CDC_EP_DATA_FUNCTION fpEpOBlkRd;
  CDC_EP_FUNCTION      fpEpOBlkIsRxEmpty;
  CDC_EP_DATA_FUNCTION fpEpIBlkWr;
  CDC_EP_FUNCTION      fpEpIBlkIsTxEmpty;
  //UART_t               uart;
  //FW_BOOLEAN           rxComplete;
  //FIFO_p               rxFifo;
  //FIFO_p               txFifo;
  //USB_CbByte           fpIEastPut;
  //USB_CbByte           fpOEastGet;
  //U8                   rxBuffer[USB_CDC_PACKET_SIZE * 5 + 1];
  //U8                   txBuffer[USB_CDC_PACKET_SIZE * 5 + 1];
  EventGroupHandle_t   events;
  EAST_p               iEAST;
  EAST_p               oEAST;
  U8                   iEastCtnr[16];
  U8                   oEastCtnr[16];
  BlockQueue_p         iQueue;
  U8                   iBuffer[EAST_MAX_DATA_LENGTH * 8];
  U8                   oBuffer[EAST_MAX_DATA_LENGTH];
} CDC_I2C_PORT;

//-----------------------------------------------------------------------------
/* Global Variables */

STATIC CDC_LINE_CODING   gLineCoding   = { 0 };
STATIC CDC_SERIAL_STATE  gNotification = { 0 };
STATIC CDC_I2C_PORT      gPort         = { 0 };

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from USB EP buffer to the EAST packet. When the
 *         packet is parsed correctly - puts it into the queue.
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 *  @note If the queue is full - all the further received bytes are ignored.
 */

static void cdc_IEastPut(U8 * pByte)
{
  FW_RESULT r = FW_ERROR;
  U8 * buffer = NULL;
  U32 size = 0;

  /* Fill the EAST block */
  r = EAST_PutByte(gPort.iEAST, *pByte);
  if (FW_COMPLETE == r)
  {
    /* If the block queue is full - ignore */
    if (0 == BlockQueue_GetCountOfFree(gPort.iQueue))
    {
      return;
    }

    /* Put the block into the queue */
    r = BlockQueue_Enqueue(gPort.iQueue, EAST_GetDataSize(gPort.iEAST));
    if (FW_SUCCESS == r)
    {
      /* Allocate the memory for the next block */
      r = BlockQueue_Allocate(gPort.iQueue, &buffer, &size);
      if (FW_SUCCESS == r)
      {
        (void)EAST_SetBuffer(gPort.iEAST, buffer, size);
      }
    }
  }
}

//-----------------------------------------------------------------------------
/** @brief Gets the Byte from the EAST packet. When the packet is complete -
 *         sets the corresponding synchronization event.
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static void cdc_OEastGet(U8 * pByte)
{
  FW_RESULT r = FW_ERROR;

  /* Empty the EAST block */
  r = EAST_GetByte(gPort.oEAST, pByte);
  if (FW_COMPLETE == r)
  {
    if (FW_TRUE == IRQ_IsInExceptionMode())
    {
      (void)xEventGroupSetBits(gPort.events, EVT_EAST_TX_COMPLETE);
    }
    else
    {
      /* We have not woken a task at the start of the ISR */
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;

      (void)xEventGroupSetBitsFromISR
            (
              gPort.events,
              EVT_EAST_TX_COMPLETE,
              &xHigherPriorityTaskWoken
            );

      /* Now we can request to switch context if necessary */
      if( xHigherPriorityTaskWoken )
      {
        taskYIELD();
      }
    }
  }
}

////-----------------------------------------------------------------------------
///** @brief Gets Byte that need to be transmitted from the Tx FIFO
// *  @param pByte - Pointer to the container for Byte
// *  @return None
// */
//
//static void cdc_TxFifoGetB(U8 * pByte)
//{
//  //(void)FIFO_Get(&gPortB.txFifo, pByte);
//}
//
////-----------------------------------------------------------------------------
///** @brief Puts received Byte from UART to the Tx FIFO
// *  @param pByte - Pointer to the container for Byte
// *  @return TRUE if byte has been put successfully
// */
//
//static FW_BOOLEAN uart_FifoPutB(U8 * pByte)
//{
//  return FW_FALSE; //(FW_BOOLEAN)(FW_SUCCESS == FIFO_Put(&gPortB.txFifo, pByte));
//}
//


static void vI2CTask(void * pvParameters)
{
  U8 * buffer = NULL;
  U32 size = 0;
  EventBits_t events = 0;

  while(1)
  {
    //GPIO_Lo(GPIOC, 13);
    //DBG_SetTextColorGreen();
    //printf("LED On\r\n");
    //vTaskDelay(500);
    //GPIO_Hi(GPIOC, 13);
    //DBG_SetTextColorRed();
    //printf("LED Off\r\n");

    /* Dequeue the block */
    (void)BlockQueue_Dequeue(gPort.iQueue, &buffer, &size);

    /* Process the block */
    gPort.oBuffer[0] = 0x33;
    gPort.oBuffer[1] = 0x33;
    gPort.oBuffer[2] = 0x33;
    gPort.oBuffer[3] = 0x33;
    gPort.oBuffer[4] = 0x33;
    gPort.oBuffer[5] = 0x33;
    EAST_SetBuffer(gPort.oEAST, gPort.oBuffer, 6);
    vTaskDelay(80);

    /* Transmit the block */
    CDC_I2C_InStage();

    /* Wait for transmitting complete */
    events = xEventGroupWaitBits
             (
               gPort.events,
               EVT_EAST_TX_COMPLETE,
               pdTRUE,
               pdFALSE,
					     portMAX_DELAY
             );
    if (EVT_EAST_TX_COMPLETE == (events & EVT_EAST_TX_COMPLETE))
    {
      //
    }

    /* Release the block */
    (void)BlockQueue_Release(gPort.iQueue);
  }
  //vTaskDelete(NULL);
}


////-----------------------------------------------------------------------------
///** @brief Gets Byte that need to be transmitted from the Rx FIFO
// *  @param pByte - Pointer to the container for Byte
// *  @return None
// */
//
//static FW_BOOLEAN uart_FifoGetB(U8 * pByte)
//{
//  return FW_FALSE; //(FW_BOOLEAN)(FW_SUCCESS == FIFO_Get(&gPortB.rxFifo, pByte));
//}
//

static void i2c_Open(void)
{
  //
}

static void i2c_SetControlLine(U16 aValue)
{
  //
}

//-----------------------------------------------------------------------------
/** @brief Initializes CDC I2C
 *  @param None
 *  @return None
 */

void CDC_I2C_Init(void)
{
  U8 * buffer = NULL;
  U32 size = 0;

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
  gNotification.wIndex                     = USBD_CDC_I2C_GetInterfaceNumber();
  gNotification.wLength                    = 2;
  gNotification.Data.Raw                   = 0;

  /* Clear Port context */
  memset(&gPort, 0, sizeof(gPort));
  /* Port is not ready yet */
  gPort.cdc.ready = FW_FALSE;

  /* Initialize Endpoints */
  gPort.fpEpOBlkRd          = USBD_CDC_I2C_OEndPointRdWsCb;
  gPort.fpEpOBlkIsRxEmpty   = USBD_CDC_I2C_OEndPointIsRxEmpty;
  gPort.fpEpIBlkWr          = USBD_CDC_I2C_IEndPointWrWsCb;
  gPort.fpEpIBlkIsTxEmpty   = USBD_CDC_I2C_IEndPointIsTxEmpty;
  gPort.cdc.fpEpIIrqWr      = USBD_CDC_I2C_IrqEndPointWr;
  gPort.cdc.fpOpen          = i2c_Open;
  gPort.cdc.fpSetCtrlLine   = i2c_SetControlLine;
  /* Initialize FIFOs */
  //FIFO_Init(&gPortI2C.rxFifo, gPortI2C.rxBuffer, sizeof(gPortI2C.rxBuffer));
  //FIFO_Init(&gPortI2C.txFifo, gPortI2C.txBuffer, sizeof(gPortI2C.txBuffer));
  //gPort.fpIEastPut = cdc_IEastPut;
  //gPortI2C.txFifoGetCb = cdc_TxFifoGetB;





  //FW_BOOLEAN result = FW_FALSE;
  //FW_RESULT status = FW_ERROR;

  gPort.iEAST = EAST_Init
                (
                  gPort.iEastCtnr,
                  sizeof(gPort.iEastCtnr),
                  NULL,
                  0
                );
  gPort.oEAST = EAST_Init
                (
                  gPort.oEastCtnr,
                  sizeof(gPort.oEastCtnr),
                  NULL,
                  0
                );
  //result = (FW_BOOLEAN)(NULL != pEAST);
  //if (FW_FALSE == result) return result;

  //(void)EAST_SetBuffer(gPort.pInEAST, eastBuffer, sizeof(eastBuffer));
  //result = (FW_BOOLEAN)(FW_SUCCESS == status);


  gPort.iQueue = BlockQueue_Init
                 (
                   gPort.iBuffer,
                   sizeof(gPort.iBuffer),
                   EAST_MAX_DATA_LENGTH
                 );
  //result = (FW_BOOLEAN)(NULL != pQueue);

  //FW_BOOLEAN result = FW_FALSE;
  //FW_RESULT status = FW_ERROR;


  //  DBG("*** EAST Block Success Fill Queue Test ***\r\n");

  //  DBG(" - Allocate the Block from Queue\r\n");
  (void)BlockQueue_Allocate(gPort.iQueue, &buffer, &size);
  //  result = (FW_BOOLEAN)(FW_SUCCESS == status);
  //  result &= (FW_BOOLEAN)(NULL != buffer);
  //  result &= (FW_BOOLEAN)(0 != size);
  //  if (FW_FALSE == result) break;

  /* Setup/Reset the EAST packet */
  (void)EAST_SetBuffer(gPort.iEAST, buffer, size);
  //  result &= (FW_BOOLEAN)(FW_SUCCESS == status);
  //  if (FW_FALSE == result) break;







  /* Initialize pointers */
  gPort.cdc.lineCoding = &gLineCoding;
  gPort.cdc.notification = &gNotification;


	/* Create the event group */
	gPort.events = xEventGroupCreate();
  (void)xEventGroupClearBits
        (
          gPort.events,
          EVT_EAST_TX_COMPLETE | EVT_I2C_EXCH_COMPLETE
        );

	// Was the event group created successfully?

  xTaskCreate
  (
    vI2CTask,
    "I2C",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );
}

//-----------------------------------------------------------------------------
/** @brief Returns pointer to CDC I2C instance
 *  @param None
 *  @return CDC I2C instance
 */

CDC_PORT * CDC_I2C_GetPort(void)
{
  return &gPort.cdc;
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param None
 *  @return None
 */

void CDC_I2C_OutStage(void)
{
  /* Read from OUT EP */
  (void)gPort.fpEpOBlkRd(cdc_IEastPut, EAST_MAX_PACKET_LENGTH);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param None
 *  @return None
 */

void CDC_I2C_InStage(void)
{
//  /* If there are some data in FIFO */
//  if (0 < FIFO_Count(gPort.txFifo))
//  {
  /* Write to IN EP */
  (void)gPort.fpEpIBlkWr(cdc_OEastGet, EAST_GetPacketSize(gPort.oEAST));
//  }
//  else
//  {
//    gPort.rxComplete = FW_FALSE;
//  }
}

//-----------------------------------------------------------------------------
/** @brief CDC SOF Callback
 *  @param None
 *  @return None
 */

void CDC_I2C_ProcessCollectedData(void)
{
//  /* Check if there are some unprocessed data */
//  if (FW_TRUE == gPort.cdc.ready)
//  {
//    if (FW_FALSE == gPort.fpEpOBlkIsRxEmpty())
//    {
//      CDC_UART_OutStage();
//    }
//
//    if (((FW_TRUE == gPort.rxComplete) ||
//        (USB_CDC_PACKET_SIZE < FIFO_Count(gPort.txFifo))) &&
//        (FW_TRUE == gPort.fpEpIBlkIsTxEmpty()))
//    {
//      CDC_UART_InStage();
//    }
//  }
}
