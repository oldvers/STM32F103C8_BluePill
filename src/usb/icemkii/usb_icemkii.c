#include "types.h"
#include "interrupts.h"
#include "usb.h"
#include "usb_definitions.h"
#include "usb_control.h"
//#include "usb_device.h"
#include "usb_descriptor.h"
#include "usb_icemkii_definitions.h"
#include "block_queue.h"
#include "usb_icemkii.h"
#include "icemkii_message.h"
#include "icemkii_processor.h"

//#define ICEMKII_TEST_MODE

#include "FreeRTOS.h"
//#ifdef ICEMKII_TEST_MODE
//#include "task.h"
//#endif
#include "event_groups.h"

//#include "east.h"

#include "debug.h"

//#undef STATIC
//#define STATIC

//-----------------------------------------------------------------------------
/* Private Type definitions */

/* Line Coding Structure */
//typedef __packed struct _CDC_LINE_CODING
//{
//  U32 dwBaudRate;  /* Number Data terminal rate, in bits per second */
//  U8  bCharFormat; /* Number of Stop bits */
//                   /*   0 - 1 Stop bit    *
//                    *   1 - 1.5 Stop bits *
//                    *   2 - 2 Stop bits   */
//  U8  bParityType; /* Number Parity */
//                   /*   0 - None    *
//                    *   1 - Odd     *
//                    *   2 - Even    *
//                    *   3 - Mark    *
//                    *   4 - Space   */
//  U8  bDataBits;   /* Number Data Bits (5, 6, 7, 8 or 16) */
//} CDC_LINE_CODING;

/* Serial State Notification Structure */
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

#define ICEMKII_TX_COMPLETE       (1 << 0)
//#define ICEMKII_RX_WAITING       (1 << 1)
//#define ICEMKII_TX_READY         (1)
//#define ICEMKII_RX_READY         (1)

#define ICEMKII_MAX_MSG_LENGTH    (1024)

typedef FW_BOOLEAN (*ICEMKII_EP_FUNCTION)(void);
typedef U32 (*ICEMKII_EP_DATA_FUNCTION)(USBD_CbByte pPutByteCb, U32 aSize);

/* JTAG ICE MkII Context */
typedef struct ICEMKII_s
{
  //CDC_PORT             cdc;
  ICEMKII_EP_DATA_FUNCTION fpEpOBlkRd;
  ICEMKII_EP_FUNCTION      fpEpOBlkIsRxEmpty;
  ICEMKII_EP_DATA_FUNCTION fpEpIBlkWr;
  ICEMKII_EP_FUNCTION      fpEpIBlkIsTxEmpty;
  //FW_RESULT            result;
  EventGroupHandle_t       events;
  ICEMKII_MSG_p            iMsg;
  ICEMKII_MSG_p            oMsg;
  U8                       iMsgHolder[ICEMKII_MSG_HOLDER_SIZE];
  U8                       oMsgHolder[ICEMKII_MSG_HOLDER_SIZE];
  BlockQueue_p             iQueue;
  U8                       iBuffer[ICEMKII_MAX_MSG_LENGTH * 4 + 40];
  U8                       oBuffer[ICEMKII_MAX_MSG_LENGTH];
} ICEMKII_t;





//-----------------------------------------------------------------------------
/* Global Variables */
//static CDC_LINE_CODING   gLineCoding =
//{
//  115200, /* dwBaudRate */
//  0,      /* bCharFormat */
//  0,      /* bParityType */
//  8,      /* bDataBits */
//};
//static CDC_SERIAL_STATE  gNotification =
//{
//  /* bmRequestType */
//  {REQUEST_TO_INTERFACE, REQUEST_CLASS, REQUEST_DEVICE_TO_HOST},
//  CDC_NTF_SERIAL_STATE, /* bNotification */
//  0,                    /* wValue */
//  USB_CDC_IF_NUM0,      /* wIndex */
//  2,                    /* wLength */
//  0,                    /* Data */
//};

static ICEMKII_t gIceMkII = {0};

//STATIC FIFO_p  gRxFifo                                     = NULL;
//STATIC FIFO_p  gTxFifo                                     = NULL;
//#ifdef ICEMKII_TEST_MODE
//static U8     gRxBuffer[USB_ICEMKII_PACKET_SIZE + 1];
//static U8     gTxBuffer[USB_ICEMKII_PACKET_SIZE + 1];
//#else
//STATIC U8      gRxBuffer[USB_ICEMKII_PACKET_SIZE * 17 + 1] = {0};
//STATIC U8      gTxBuffer[USB_ICEMKII_PACKET_SIZE * 17 + 1] = {0};
//#endif

//static U8               *gIrqBuff = NULL;
//static U8                gIrqBuffLen = 0;
//static SemaphoreHandle_t gVcpSemRx = NULL;
//static SemaphoreHandle_t gVcpMutRx = NULL;
//static SemaphoreHandle_t gVcpSemTx = NULL;
//static SemaphoreHandle_t gVcpMutTx = NULL;
//static U8                gVcpReading = FALSE;
//static EAST_STATE        gVcpRxState;
//static EAST_STATE        gVcpTxState;

//STATIC EventGroupHandle_t    hEvtGroup = {0};

//-----------------------------------------------------------------------------
/* Private Functions declarations */
//STATIC void icemkii_ProcessTx(void);
//STATIC void icemkii_ProcessRx(void);

//-----------------------------------------------------------------------------
/** @brief Processes IRQ EP data
 *  @param None
 *  @return None
 */
//void icemkii_IrqInStage(void)
//{
//  U8 len = 0;

//  if (0 == gIrqBuffLen) return;

//  if (USB_CDC_IRQ_PACKET_SIZE < gIrqBuffLen)
//  {
//    len = USB_CDC_IRQ_PACKET_SIZE;
//  }
//  else
//  {
//    len = gIrqBuffLen;
//  }

//  LOG("CDC IRQ IN: len = %d\r\n", len);
//  USB_EpWrite(USB_CDC_EP_IRQ_IN, gIrqBuff, len);

//  gIrqBuff += len;
//  gIrqBuffLen -= len;
//}

//-----------------------------------------------------------------------------
/** @brief Sends Serial State notification
 *  @param aState - Errors/Evetns state
 *  @return None
 */
//void cdc_NotifyState(U16 aState)
//{
//  gNotification.Data.Raw = aState;
//  gIrqBuff = (U8 *)&gNotification;
//  gIrqBuffLen = sizeof(gNotification);
//
//  cdc_IrqInStage();
//}

//-----------------------------------------------------------------------------
/** @brief ICEMKII Control Setup USB Request
 *  @param pSetup - Pointer to Setup Packet
 *  @param pData - Pointer to place for setting the pointer to requested data
 *  @param pSize - Pointer to place for setting the requested data size
 *  @return Stage that should be performed after calling this function
 *  @note On calling this function pData points to Control Endpoint internal
 *        buffer so requested data can be placed right there if it is not
 *        exceeds Control Endpoint Max Packet size
 */
USB_CTRL_STAGE ICEMKII_CtrlSetupReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
)
{
  USB_CTRL_STAGE result = USB_CTRL_STAGE_ERROR;

  DBG("ICEMKII Setup: Req = %d Len = %d\r\n", pSetup->bRequest, *pSize);

//  switch (pSetup->bRequest)
//  {
//    case CDC_REQ_SET_LINE_CODING:
//      LOG("CDC Setup: Set Line Coding: Len = %d\r\n", *pSize);
//      *pData = (U8 *)&gLineCoding;
//      result = USB_CTRL_STAGE_WAIT;
//      break;
//    case CDC_REQ_GET_LINE_CODING:
//      LOG("CDC Setup: Get Line Coding\r\n");
//      *pData = (U8 *)&gLineCoding;
//      result = USB_CTRL_STAGE_DATA;
//      break;
//    case CDC_REQ_SET_CONTROL_LINE_STATE:
//      LOG("CDC Setup: Set Ctrl Line State\r\n");
//      result = USB_CTRL_STAGE_STATUS;
//      break;
//  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief ICEMKII USB Out Request
 *  @param pSetup - Pointer to Setup Packet
 *  @param pData - Pointer to place for setting the pointer to requested data
 *  @param pSize - Pointer to place for setting the requested data size
 *  @return Stage that should be performed after calling this function
 *  @note Called when all the OUT packets have been already collected
 */
USB_CTRL_STAGE ICEMKII_CtrlOutReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
)
{
  USB_CTRL_STAGE result = USB_CTRL_STAGE_ERROR;

  DBG("ICEMKII Out: Req = %d Len = %d\r\n", pSetup->bRequest, *pSize);

//  switch (pSetup->bRequest)
//  {
//    case CDC_REQ_SET_LINE_CODING:
//      LOG("CDC Out: Set Line Coding: Baud = %d, Len = %d\r\n", gLineCoding.dwBaudRate, *pSize);
//      result = USB_CTRL_STAGE_STATUS;
//      break;
//  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief ICEMKII Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */
//void cdc_InterruptIn(U32 aEvent)
//{
//  cdc_IrqInStage();
//}

//-----------------------------------------------------------------------------
/** @brief Gets the Byte from the message. When the message is complete -
 *         sets the corresponding synchronization event.
 *  @param pByte - Pointer to the container for the Byte
 *  @return None
 */

static void icemkii_OMsgGet(U8 * pByte)
{
  FW_RESULT result = FW_ERROR;

  /* Empty the EAST block */
  result = ICEMKII_MSG_GetByte(gIceMkII.oMsg, pByte);
  if (FW_COMPLETE == result)
  {
    if (FW_FALSE == IRQ_IsInExceptionMode())
    {
      (void)xEventGroupSetBits(gIceMkII.events, ICEMKII_TX_COMPLETE);
    }
    else
    {
      /* We have not woken a task at the start of the ISR */
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;

      (void)xEventGroupSetBitsFromISR
            (
              gIceMkII.events,
              ICEMKII_TX_COMPLETE,
              &xHigherPriorityTaskWoken
            );

      /* Now we can request to switch context if necessary */
      if (xHigherPriorityTaskWoken)
      {
        taskYIELD();
      }
    }
  }
}

//-----------------------------------------------------------------------------
/** @brief ICEMKII Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */
void ICEMKII_BulkIn(U32 aEvent)
{
  (void)aEvent;
  /* If there are some data in the message */
  if (0 < ICEMKII_MSG_GetPacketSize(gIceMkII.oMsg))
  {
    /* Write to IN EP */
    (void)gIceMkII.fpEpIBlkWr
    (
      icemkii_OMsgGet,
      ICEMKII_MSG_GetPacketSize(gIceMkII.oMsg)
    );
  }
}





//-----------------------------------------------------------------------------
/** @brief Puts received Byte from USB EP buffer to the message. When the
 *         message is parsed correctly - puts it into the queue.
 *  @param pByte - Pointer to the container for the Byte
 *  @return None
 *  @note If the queue is full - all the further received bytes are ignored.
 */

static void icemkii_IMsgPut(U8 * pByte)
{
  FW_RESULT result = FW_ERROR;
  U8 * buffer = NULL;
  U32 size = 0;

  /* Fill the message */
  result = ICEMKII_MSG_PutByte(gIceMkII.iMsg, *pByte);
  if (FW_COMPLETE == result)
  {
    /* If the block queue is full - ignore */
    if (0 == BlockQueue_GetCountOfFree(gIceMkII.iQueue))
    {
      return;
    }

    /* Put the message into the queue */
    result = BlockQueue_Enqueue
             (
                 gIceMkII.iQueue,
                 ICEMKII_MSG_GetDataSize(gIceMkII.iMsg)
             );
    if (FW_SUCCESS == result)
    {
      /* Allocate the memory for the next message */
      result = BlockQueue_Allocate(gIceMkII.iQueue, &buffer, &size);
      if (FW_SUCCESS == result)
      {
        (void)ICEMKII_MSG_SetBuffer(gIceMkII.iMsg, buffer, size);
      }
    }
  }
}

//-----------------------------------------------------------------------------
/** @brief ICEMKII Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */
void ICEMKII_BulkOut(U32 aEvent)
{
  (void)aEvent;
  //icemkii_ProcessRx();
  /* Read from OUT EP */
  (void)gIceMkII.fpEpOBlkRd(icemkii_IMsgPut, ICEMKII_MAX_MSG_LENGTH);
}





//-----------------------------------------------------------------------------
/** @brief Sends the response via USB
 *  @param pRsp - Pointer to the response container
 *  @param size - Size of the response
 *  @return None
 */

static void icemkii_SendResponse(U8 * pRsp, U32 size)
{
  EventBits_t events = 0;

  if (0 == size) return;

  /* Reset the message */
  ICEMKII_MSG_SetBuffer(gIceMkII.oMsg, pRsp, size);

  /* Send the response */
  ICEMKII_BulkIn(0);

  /* Wait for transmitting complete */
  events = xEventGroupWaitBits
           (
             gIceMkII.events,
             ICEMKII_TX_COMPLETE,
             pdTRUE,
             pdFALSE,
             portMAX_DELAY
           );
  if (ICEMKII_TX_COMPLETE == (events & ICEMKII_TX_COMPLETE))
  {
    /* Check for errors */
  }
}







//-----------------------------------------------------------------------------
/** @brief Thread function
 *  @param pvParameters - Pointer to the parameters
 *  @return None
 */

static void ICEMKII_Task(void * pvParameters)
{
  U8 * req = NULL, * rsp = gIceMkII.oBuffer;
  U32 size = 0;

  while (FW_TRUE)
  {
    /* Dequeue the request */
    (void)BlockQueue_Dequeue(gIceMkII.iQueue, &req, &size);

    /* Prepare the response */
    ICEMKII_Process(req, rsp, &size);

    //rsp->opcode = req->opcode;
    //rsp->address = req->address;

    ///* Process the request */
    //if ((I2C_OPCODE_READ == req->opcode) && (4 == size))
    //{
    //  cdc_I2cRead(req, rsp, &size);
    //}
    //else if ((I2C_OPCODE_WRITE == req->opcode) && (4 < size))
    //{
    //  cdc_I2cWrite(req, rsp, &size);
    //}
    //else
    //{
    //  cdc_I2cError(req, rsp, &size);
    //}

    /* Send the response */
    icemkii_SendResponse(rsp, size);

    /* Release the block */
    (void)BlockQueue_Release(gIceMkII.iQueue);
  }
  //vTaskDelete(NULL);
}

//-----------------------------------------------------------------------------
/** @brief Initializes ICEMKII
 *  @param None
 *  @return None
 */
//#ifdef ICEMKII_TEST_MODE
//void vTestTxTask(void * pvParameters)
//{
//  while(TRUE)
//  {
//    vTaskDelay(500);
//    icemkii_OutStage();
//  }
//}
//
//U8 ICEMKII_ReadByte(void);
//
//void vTestRxTask(void * pvParameters)
//{
//  vTaskDelay(5000);
//
//  while(TRUE)
//  {
//    ICEMKII_ReadByte();
//  }
//}
//#endif

void ICEMKII_Init(void)
{
  /* Event Group for flow control */
//  hEvtGroup = xEventGroupCreate();

//  /* FIFOs */
//  gRxFifo = FIFO_Init(gRxBuffer, sizeof(gRxBuffer));
//  gTxFifo = FIFO_Init(gTxBuffer, sizeof(gTxBuffer));

//#ifdef ICEMKII_TEST_MODE
//  xTaskCreate
//  (
//    vTestRxTask,
//    "TestRxTask",
//    configMINIMAL_STACK_SIZE,
//    NULL,
//    tskIDLE_PRIORITY + 2,
//    NULL
//  );
//
//  xTaskCreate
//  (
//    vTestTxTask,
//    "TestTxTask",
//    configMINIMAL_STACK_SIZE,
//    NULL,
//    tskIDLE_PRIORITY + 1,
//    NULL
//  );
//#else
  /* Register appropriate EP callbacks */
//  USB_SetCb_Ep(USB_ICEMKII_EP_BULK_OUT, icemkii_BulkOut);
//  USB_SetCb_Ep(USB_ICEMKII_EP_BULK_IN,  icemkii_BulkIn);
//#endif

  /* Create Semaphores/Mutex for VCP */
//  gVcpSemRx = xSemaphoreCreateBinary();
//  gVcpMutRx = xSemaphoreCreateMutex();
//  gVcpSemTx = xSemaphoreCreateBinary();
//  gVcpMutTx = xSemaphoreCreateMutex();


//  //-----------------------------------------------------------------------------
///** @brief Initializes CDC I2C
// *  @param None
// *  @return None
// */
//
//void CDC_I2C_Init(void)
//{
  U8 * buffer = NULL;
  U32 size = 0;

  /* Clear Port context */
  //memset(&gPort, 0, sizeof(gPort));
  /* Port is not ready yet */
  //gPort.cdc.ready = FW_FALSE;

  /* Initialize Endpoints */
  gIceMkII.fpEpOBlkRd        = USBD_ICEMKII_OEndPointRdWsCb;
  gIceMkII.fpEpOBlkIsRxEmpty = USBD_ICEMKII_OEndPointIsRxEmpty;
  gIceMkII.fpEpIBlkWr        = USBD_ICEMKII_IEndPointWrWsCb;
  gIceMkII.fpEpIBlkIsTxEmpty = USBD_ICEMKII_IEndPointIsTxEmpty;

  /* Initialize EAST packet containers */
  gIceMkII.iMsg = ICEMKII_MSG_Init
                  (
                    gIceMkII.iMsgHolder,
                    sizeof(gIceMkII.iMsgHolder),
                    NULL,
                    0
                  );
  gIceMkII.oMsg = ICEMKII_MSG_Init
                  (
                    gIceMkII.oMsgHolder,
                    sizeof(gIceMkII.oMsgHolder),
                    NULL,
                    0
                  );

  /* Initialize the EAST packet queue */
  gIceMkII.iQueue = BlockQueue_Init
                    (
                      gIceMkII.iBuffer,
                      sizeof(gIceMkII.iBuffer),
                      ICEMKII_MAX_MSG_LENGTH
                    );
  /* Allocate the memory for the first input packet */
  (void)BlockQueue_Allocate(gIceMkII.iQueue, &buffer, &size);
  /* Setup/Reset the input packet */
  (void)ICEMKII_MSG_SetBuffer(gIceMkII.iMsg, buffer, size);

  /* Create the event group for synchronization */
  gIceMkII.events = xEventGroupCreate();
  (void)xEventGroupClearBits
        (
          gIceMkII.events,
          ICEMKII_TX_COMPLETE //| EVT_I2C_EXCH_COMPLETE
        );

  /* Create the I2C task */
  xTaskCreate
  (
    ICEMKII_Task,
    "ICEMKII",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );
}

//-----------------------------------------------------------------------------
/** @brief Opens Virtual COM Port
 *  @param None
 *  @return TRUE - Port opened, FALSE - Error
 */
//U32 VCP_Open(void)
//{
//  /* Check if VCP Semaphore/Mutex were created successfuly */
//  if ( (NULL == gVcpSemRx) || (NULL == gVcpMutRx) ||
//       (NULL == gVcpSemTx) || (NULL == gVcpMutTx) )
//  {
//    return FALSE;
//  }

//  /* Indicate that there is no reading in progress */
//  gVcpReading = FALSE;
//
//  return TRUE;
//}

//-----------------------------------------------------------------------------
/** @brief Callback function - VCP read completed
 *  @param None
 *  @return None
 */
//void cdc_OutCompleted(void)
//{
//  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//  gVcpReading = FALSE;
//  xSemaphoreGiveFromISR(gVcpSemRx, &xHigherPriorityTaskWoken);
//}

//-----------------------------------------------------------------------------
/** @brief Processes OUT EP data
 *  @param None
 *  @return None
 */
////STATIC U8 icemkii_Put(U8 * pByte)
//STATIC void icemkii_Put(U8 * pByte)
//{
//  DBG(" %0.2X", *pByte);
//  //return (U8)
//  (void)FIFO_Put(gRxFifo, pByte);
//}
////typedef U8   (*USB_CbEpGet)(U8 * pByte);
////EventBits_t uxReturned;
////uxReturned = xEventGroupWaitBits( xEventGroup, ebALL_SYNC_BITS, pdFALSE, pdTRUE, portMAX_DELAY );
////uxReturned = xEventGroupWaitBits( xEventGroup,	/* The event group that contains the event bits being queried. */
////										 ebBIT_1,		/* The bit to wait for. */
////										 pdTRUE,		/* Clear the bit on exit. */
////										 pdTRUE,		/* Wait for all the bits (only one in this case anyway). */
////										 portMAX_DELAY ); /* Block indefinitely to wait for the condition to be met. */
////if( xEventGroupSetBits( xEventGroup, 0x00 ) != 0 )
////		{
////			xError = pdTRUE;
////		}
////    xEventGroupSetBits( xEventGroup, ( ebALL_SYNC_BITS & ~ebSET_BIT_TASK_SYNC_BIT ) );
////#define ICEMKII_RX_READY         (1 << 0)
////#define ICEMKII_RX_WAITING       (1 << 1)

//STATIC void icemkii_ProcessRx(void)
//{
//  BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult;
//  EventBits_t uxBitsToSet = 0;
//  U32 size = 0;
//
////#ifdef ICEMKII_TEST_MODE
////  do
////  {
////    if (13 > FIFO_Free(&gRxFifo)) break;
////
////    //LOG("--- ICEMKII Tx Task ---------------------------\r\n");
////    LOG("--- ICEMKII OUT -------------------------------\r\n  - ");
////    for (size = 0; size < 13; size++) icemkii_Put((U8 *)&size);
////    LOG(" : Len = %d\r\n", size);
////  }
////  while ( 0 );
////#else
//  /* Read from OUT EP */
//  DBG("ICEMKII OUT:\r\n - ");
//  size = USBD_ICEMKII_OEndPointRdWsCb(icemkii_Put, FIFO_Free(gRxFifo));
//  DBG(" : Len = %d\r\n", size);
////#endif
//
//  /* Successful reading */
//  if (0 != size)
//  {
//    uxBitsToSet = ICEMKII_RX_READY;
//  }
//  /* Not enough space - reading should be repeated later */
//  else
//  {
//    uxBitsToSet = ICEMKII_RX_WAITING;
//  }
//
//  /* If there is no reading in progress - ignore */
////  while ((TRUE == gVcpReading) && (i < len))
////  {
////    EAST_PutByte(&gVcpRxState, gOBuffer[i++]);
////  }
//
////#ifdef ICEMKII_TEST_MODE
////  xResult = xEventGroupSetBits
////            (
////              hEvtGroup,   /* The event group being updated */
////              uxBitsToSet  /* The bits being set */
////            );
////#else
//  /* Set bits in EventGroup */
//  xResult = xEventGroupSetBitsFromISR
//            (
//              hEvtGroup,   /* The event group being updated */
//              uxBitsToSet, /* The bits being set */
//              &xHigherPriorityTaskWoken
//            );
//
//  /* Was the message posted successfully? */
//  if( xResult != pdFAIL )
//  {
//      /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
//      switch should be requested.  The macro used is port specific and will
//      be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
//      the documentation page for the port being used. */
//      portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
//  }
////#endif
//}

//-----------------------------------------------------------------------------
/** @brief Reads byte from JTAG ICE mkII input FIFO buffer
 *  @param pValue - pointer to the place where byte will be read
 *  @param aTimeOutMs - timeout for waiting a byte
 *  @return FW_SUCCESS, FW_TIMOUT
 *  @note In case FIFO is empty the caller will be blocked until at least
 *        one byte available or time is out
 */
//FW_RESULT ICEMKII_ReadByte(U8 * pValue, U32 aTimeOutMs)
//{
//    EventBits_t uxReturned;
//    FW_RESULT result = FW_SUCCESS;
//    U32 size = 0;
//
//    //uxReturned = xEventGroupWaitBits( xEventGroup, ebALL_SYNC_BITS, pdFALSE, pdTRUE, portMAX_DELAY );
//    //pdMS_TO_TICKS( xTimeInMs )
//    uxReturned = xEventGroupWaitBits
//                 (
//                     hEvtGroup,                             /* The event group that contains the event bits being queried */
//                     ICEMKII_RX_READY | ICEMKII_RX_WAITING, /* The bit to wait for */
//                     pdFALSE,                               /* Clear the bit on exit */
//                     pdFALSE,                               /* Wait for all the bits (only one in this case anyway) */
//                     pdMS_TO_TICKS(aTimeOutMs)              /* Block indefinitely to wait for the condition to be met */
//                 );
//
//    //LOG("--- ICEMKII ReadByte --------------------------\r\n  - ");
//
//    if (0 == uxReturned)
//    {
//        result = FW_TIMEOUT;
//    }
//    else
//    {
//        if (0 != (uxReturned & ICEMKII_RX_WAITING))
//        {
//    //#ifdef ICEMKII_TEST_MODE
//    //    do
//    //    {
//    //      if (13 > FIFO_Free(&gRxFifo)) break;
//    //
//    //      LOG("PENDING READ:\r\n  - ");
//    //      for (size = 0; size < 13; size++) icemkii_Put((U8 *)&size);
//    //      LOG(" : Len = %d\r\n", size);
//    //
//    //      (void)xEventGroupClearBits(hEvtGroup, ICEMKII_RX_WAITING);
//    //    }
//    //    while ( 0 );
//    //#else
//            /* Read from OUT EP */
//            DBG("ICEMKII READ:\r\n  - ");
//            size = USBD_ICEMKII_OEndPointRdWsCb
//                   (
//                     icemkii_Put,
//                     FIFO_Free(gRxFifo)
//                   );
//            DBG(" : Len = %d\r\n", size);
//
//        //(void)xEventGroupClearBits(hEvtGroup, ICEMKII_RX_READY);
//    //#endif
//        }
//
//        if (0 != (uxReturned & ICEMKII_RX_READY))
//        {
//            size = FIFO_Get(gRxFifo, pValue);
//            DBG("  - %0.2X\r\n", result);
//
//            if (FW_EMPTY == size)
//            {
//                (void)xEventGroupClearBits(hEvtGroup, ICEMKII_RX_READY);
//            }
//        }
//    }
//
////if( xEventGroupSetBits( xEventGroup, 0x00 ) != 0 )
////{
////xError = pdTRUE;
////}
////xEventGroupSetBits( xEventGroup, ( ebALL_SYNC_BITS & ~ebSET_BIT_TASK_SYNC_BIT ) );
////#define ICEMKII_RX_READY         (1 << 0)
////#define ICEMKII_RX_WAITING       (1 << 1)
//
//    return (result);
//}

//U32 VCP_Read(U8 * pData, U32 aSize, U32 aTimeout)
//{
//  U32 result = 0;

//  if (pdFALSE == xSemaphoreTake(gVcpMutRx, portMAX_DELAY))
//  {
//    return 0;
//  }

//  /* Init EAST state for reading */
//  gVcpRxState.MaxSize = aSize;
//  gVcpRxState.ActSize = 0;
//  gVcpRxState.Index = 0;
//  gVcpRxState.Buffer = pData;
//  gVcpRxState.OnComplete = cdc_OutCompleted;

//  /* Indicate VCP reading is in progress */
//  gVcpReading = TRUE;

//  /* Wait for Rx Complete */
//  if (pdTRUE == xSemaphoreTake(gVcpSemRx, aTimeout))
//  {
//    result = gVcpRxState.ActSize;
//  }
//  else
//  {
//    result = 0;
//  }

//  xSemaphoreGive(gVcpMutRx);

//  return result;
//}

//-----------------------------------------------------------------------------
/** @brief Callback function - VCP write completed
 *  @param None
 *  @return None
 */
//void cdc_InCompleted(void)
//{
//  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//  xSemaphoreGiveFromISR(gVcpSemTx, &xHigherPriorityTaskWoken);
//}

//-----------------------------------------------------------------------------
/** @brief Processes IN EP data
 *  @param None
 *  @return None
 */
//STATIC void icemkii_ProcessTx(void)
//{
////  U8 /*data,*/ len = 0;
//
////  while
////  (
////    (len < USB_ICEMKII_PACKET_SIZE) &&
////    (TRUE == EAST_GetByte(&gVcpTxState, &data))
////  )
////  {
////    gIBuffer[len++] = data;
////  }
//
//  //LOG("CDC IN: len = %d\r\n", len);
//  //if (0 < len) USB_EpWrite(USB_ICEMKII_EP_BULK_IN, gIBuffer, len);
//}

//-----------------------------------------------------------------------------
/** @brief Writes to Virtual COM Port
 *  @param pData - Buffer of data to be written
 *  @param aSize - Size of data to be written
 *  @return Number of bytes written
 */
//U32 VCP_Write(U8 * pData, U32 aSize, U32 aTimeout)
//{
//  U32 result = 0;

//  if (pdFALSE == xSemaphoreTake(gVcpMutTx, portMAX_DELAY))
//  {
//    return 0;
//  }

//  /* Init EAST state for writing */
//  gVcpTxState.MaxSize = aSize;
//  gVcpTxState.ActSize = 0;
//  gVcpTxState.Index = 0;
//  gVcpTxState.Buffer = pData;
//  gVcpTxState.OnComplete = cdc_InCompleted;

//  /* Process IN EP */
//  cdc_InStage();

//  /* Wait for Tx complete */
//  if (pdTRUE == xSemaphoreTake(gVcpSemTx, aTimeout))
//  {
//    result = gVcpTxState.ActSize;
//  }
//  else
//  {
//    result = 0;
//  }

//  xSemaphoreGive(gVcpMutTx);

//  return result;
//}

//-----------------------------------------------------------------------------
/** @brief Closes Virtual COM Port
 *  @param None
 *  @return None
 */
//void VCP_Close(void)
//{
//  /* Indicate that there is no reading in progress */
//  gVcpReading = FALSE;
//}
