#include "types.h"
#include "interrupts.h"
#include "usb.h"
#include "usb_definitions.h"
#include "usb_control.h"
#include "usb_descriptor.h"
#include "usb_icemkii_definitions.h"
#include "block_queue.h"
#include "usb_icemkii.h"
#include "icemkii_message.h"
#include "icemkii_processor.h"
#include "dwire_processor.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "event_groups.h"

/* -------------------------------------------------------------------------- */

#define ICEMKII_TX_COMPLETE                 (1 << 0)
#define ICEMKII_MAX_MSG_LENGTH              (1024)
#define ICEMKII_MSG_QUEUE_WAIT_TIMEOUT      (15)

typedef FW_BOOLEAN (*ICEMKII_EP_FUNCTION)(void);
typedef U32 (*ICEMKII_EP_DATA_FUNCTION)(USBD_CbByte pPutByteCb, U32 aSize);

/* JTAG ICE MkII Context */
typedef struct ICEMKII_s
{
  ICEMKII_EP_DATA_FUNCTION fpEpOBlkRd;
  ICEMKII_EP_FUNCTION      fpEpOBlkIsRxEmpty;
  ICEMKII_EP_DATA_FUNCTION fpEpIBlkWr;
  ICEMKII_EP_FUNCTION      fpEpIBlkIsTxEmpty;
  EventGroupHandle_t       events;
  ICEMKII_MSG_p            iMsg;
  ICEMKII_MSG_p            oMsg;
  U8                       iMsgHolder[ICEMKII_MSG_HOLDER_SIZE];
  U8                       oMsgHolder[ICEMKII_MSG_HOLDER_SIZE];
  BlockQueue_p             iQueue;
  U8                       iBuffer[ICEMKII_MAX_MSG_LENGTH * 4 + 40];
  U8                       oBuffer[ICEMKII_MAX_MSG_LENGTH];
} ICEMKII_t;

/* -------------------------------------------------------------------------- */

static ICEMKII_t gIceMkII = {0};

/* -------------------------------------------------------------------------- */
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

  return result;
}

/* -------------------------------------------------------------------------- */
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

  return result;
}

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
/** @brief ICEMKII Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */
void ICEMKII_BulkOut(U32 aEvent)
{
  (void)aEvent;

  /* Read from OUT EP */
  (void)gIceMkII.fpEpOBlkRd(icemkii_IMsgPut, ICEMKII_MAX_MSG_LENGTH);
}

/* -------------------------------------------------------------------------- */
/** @brief Sends the response via USB
 *  @param pRsp - Pointer to the response container
 *  @param size - Size of the response
 *  @return None
 */

static void icemkii_SendResponse(U8 * pRsp, U32 size, FW_BOOLEAN evt)
{
  EventBits_t events = 0;
  U16 sn = 0;

  if (0 == size) return;

  /* Reset the message */
  ICEMKII_MSG_SetBuffer(gIceMkII.oMsg, pRsp, size);

  if (FW_TRUE == evt)
  {
    sn = 0xFFFF;
  }
  else
  {
    sn = ICEMKII_MSG_GetSequenceNumber(gIceMkII.iMsg);
  }
  ICEMKII_MSG_SetSequenceNumber(gIceMkII.oMsg, sn);

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

/* -------------------------------------------------------------------------- */
/** @brief Thread function
 *  @param pvParameters - Pointer to the parameters
 *  @return None
 */

static void icemkii_Task(void * pvParameters)
{
  U8 * req = NULL, * rsp = gIceMkII.oBuffer;
  U32 size = 0;

  while (FW_TRUE)
  {
    /* Dequeue the request */
    if (FW_SUCCESS == BlockQueue_Dequeue(gIceMkII.iQueue, &req, &size))
    {
      /* Prepare the response */
      ICEMKII_Process(req, rsp, &size);

      /* Send the response */
      icemkii_SendResponse(rsp, size, FW_FALSE);

      /* Release the block */
      (void)BlockQueue_Release(gIceMkII.iQueue);
    }

    if (FW_TRUE == ICEMKII_CheckForEvents(rsp, &size))
    {
      icemkii_SendResponse(rsp, size, FW_TRUE);
    }
  }
  //vTaskDelete(NULL);
}

/* -------------------------------------------------------------------------- */
/** @brief Initializes ICEMKII
 *  @param None
 *  @return None
 */
void ICEMKII_Init(void)
{
  U8 * buffer = NULL;
  U32 size = 0;

  /* Initialize Endpoints */
  gIceMkII.fpEpOBlkRd        = USBD_ICEMKII_OEndPointRdWsCb;
  gIceMkII.fpEpOBlkIsRxEmpty = USBD_ICEMKII_OEndPointIsRxEmpty;
  gIceMkII.fpEpIBlkWr        = USBD_ICEMKII_IEndPointWrWsCb;
  gIceMkII.fpEpIBlkIsTxEmpty = USBD_ICEMKII_IEndPointIsTxEmpty;

  /* Initialize ICE mkII message containers */
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

  /* Initialize the ICE mkII message queue */
  gIceMkII.iQueue = BlockQueue_Init
                    (
                      gIceMkII.iBuffer,
                      sizeof(gIceMkII.iBuffer),
                      ICEMKII_MAX_MSG_LENGTH
                    );
  BlockQueue_SetTimeout(gIceMkII.iQueue, 15);

  /* Allocate the memory for the first input message */
  (void)BlockQueue_Allocate(gIceMkII.iQueue, &buffer, &size);
  /* Setup/Reset the input packet */
  (void)ICEMKII_MSG_SetBuffer(gIceMkII.iMsg, buffer, size);

  /* Create the event group for synchronization */
  gIceMkII.events = xEventGroupCreate();
  (void)xEventGroupClearBits
        (
          gIceMkII.events,
          ICEMKII_TX_COMPLETE
        );

  /* Create the ICE mkII task */
  xTaskCreate
  (
    icemkii_Task,
    "ICEMKII",
    5 * configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );
}

/* -------------------------------------------------------------------------- */
