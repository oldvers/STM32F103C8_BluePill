#include <string.h>
#include "types.h"
#include "usb_definitions.h"
#include "usb_cdc_definitions.h"
#include "usb_descriptor.h"
#include "cdc_private.h"
#include "cdc_spi.h"

#include "board.h"
#include "gpio.h"
#include "spi.h"
#include "interrupts.h"
#include "usb.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "east_packet.h"
#include "block_queue.h"

//-----------------------------------------------------------------------------
/* Private Types definitions */

#define EAST_MAX_DATA_LENGTH      (128)
#define EAST_HEADER_LENGTH        (6)
#define EAST_MAX_PACKET_LENGTH    (EAST_HEADER_LENGTH + EAST_MAX_DATA_LENGTH)

#define EVT_EAST_TX_COMPLETE      (1 << 0)
#define EVT_SPI_EXCH_COMPLETE     (1 << 1)
#define EVT_SPI_EXCH_TIMEOUT      (100)

#define SPI_OPCODE_READ           (0x00)
#define SPI_OPCODE_WRITE          (0x01)
#define SPI_STATUS_SUCCESS        (0x00)
#define SPI_STATUS_ERROR          (0xFF)

#define CDC_CTRL_LINE_STATE_DTR   (0)
#define CDC_CTRL_LINE_STATE_RTS   (1)


typedef FW_BOOLEAN (*CDC_EP_FUNCTION)(void);
typedef U32 (*CDC_EP_DATA_FUNCTION)(USBD_CbByte pPutByteCb, U32 aSize);

/* CDC Port Context */
typedef struct _CDC_SPI_PORT
{
  CDC_PORT             cdc;
  CDC_EP_DATA_FUNCTION fpEpOBlkRd;
  CDC_EP_FUNCTION      fpEpOBlkIsRxEmpty;
  CDC_EP_DATA_FUNCTION fpEpIBlkWr;
  CDC_EP_FUNCTION      fpEpIBlkIsTxEmpty;
  FW_RESULT            result;
  EventGroupHandle_t   events;
  EAST_p               iEAST;
  EAST_p               oEAST;
  U8                   iEastCtnr[16];
  U8                   oEastCtnr[16];
  BlockQueue_p         iQueue;
  U8                   iBuffer[EAST_MAX_DATA_LENGTH * 8];
  U8                   oBuffer[EAST_MAX_DATA_LENGTH];
} CDC_SPI_PORT;

typedef struct _SPI_PACKET
{
  U8 opcode;
  U8 status;
  U8 address;
  U8 size;
  U8 data[EAST_MAX_DATA_LENGTH - 4];
} SPI_PACKET;

//-----------------------------------------------------------------------------
/* Global Variables */

STATIC CDC_LINE_CODING   gLineCoding   = { 0 };
STATIC CDC_SERIAL_STATE  gNotification = { 0 };
STATIC CDC_SPI_PORT      gPort         = { 0 };

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
    if (FW_FALSE == IRQ_IsInExceptionMode())
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
      if (xHigherPriorityTaskWoken)
      {
        taskYIELD();
      }
    }
  }
}

//-----------------------------------------------------------------------------
/** @brief SPI callback function
 *  @param aResult - SPI transaction result
 *  @return None
 */

static FW_BOOLEAN spi_Complete(FW_RESULT aResult)
{
  BaseType_t xHigherPriorityTaskWoken;

  xHigherPriorityTaskWoken = pdFALSE;

  (void)xEventGroupSetBitsFromISR
        (
          gPort.events,
          EVT_SPI_EXCH_COMPLETE,
          &xHigherPriorityTaskWoken
        );
  gPort.result = aResult;

  if (xHigherPriorityTaskWoken)
  {
    taskYIELD();
  }

  return FW_TRUE;
}

//-----------------------------------------------------------------------------
/** @brief Waits for SPI transaction completion
 *  @param None
 *  @return None
 */

static FW_BOOLEAN spi_WaitForComplete(void)
{
  FW_BOOLEAN result = FW_TRUE;
  EventBits_t events = 0;

  events = xEventGroupWaitBits
           (
             gPort.events,
             EVT_SPI_EXCH_COMPLETE,
             pdTRUE,
             pdFALSE,
             EVT_SPI_EXCH_TIMEOUT
           );

  if (EVT_SPI_EXCH_COMPLETE == (events & EVT_SPI_EXCH_COMPLETE))
  {
    if (FW_COMPLETE == gPort.result)
    {
      /* Success */
    }
    else
    {
      result = FW_FALSE;
    }
  }
  else
  {
    /* Timeout */
    result = FW_FALSE;
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Prepares the error response
 *  @param pReq - Pointer to the request container
 *  @param pRsp - Pointer to the response container
 *  @param pSize - Pointer to the size container
 *  @return None
 */

static void cdc_SpiError(SPI_PACKET * pReq, SPI_PACKET * pRsp, U32 * pSize)
{
  pRsp->status = SPI_STATUS_ERROR;
  pRsp->size = 0;
  *pSize = 4;
}

//-----------------------------------------------------------------------------
/** @brief Performs the SPI read transaction
 *  @param pReq - Pointer to the request container
 *  @param pRsp - Pointer to the response container
 *  @param pSize - Pointer to the size container
 *  @return None
 */

static void cdc_SpiRead(SPI_PACKET * pReq, SPI_PACKET * pRsp, U32 * pSize)
{
  GPIO_Lo(SPI1_CS_PORT, SPI1_CS_PIN);
  SPI_MExchange(SPI_1, NULL, pRsp->data, pReq->size);

  if (FW_TRUE == spi_WaitForComplete())
  {
    pRsp->status = SPI_STATUS_SUCCESS;
    pRsp->size = pReq->size;
    *pSize = (pReq->size + 4);
  }
  else
  {
    cdc_SpiError(pReq, pRsp, pSize);
  }
  GPIO_Hi(SPI1_CS_PORT, SPI1_CS_PIN);
}

//-----------------------------------------------------------------------------
/** @brief Performs the SPI write transaction
 *  @param pReq - Pointer to the request container
 *  @param pRsp - Pointer to the response container
 *  @param pSize - Pointer to the size container
 *  @return None
 */

static void cdc_SpiWrite(SPI_PACKET * pReq, SPI_PACKET * pRsp, U32 * pSize)
{
  GPIO_Lo(SPI1_CS_PORT, SPI1_CS_PIN);
  SPI_MExchange(SPI_1, pReq->data, NULL, pReq->size);

  if (FW_TRUE == spi_WaitForComplete())
  {
    pRsp->status = SPI_STATUS_SUCCESS;
    pRsp->size = pReq->size;
    *pSize = 4;
  }
  else
  {
    cdc_SpiError(pReq, pRsp, pSize);
  }
  GPIO_Hi(SPI1_CS_PORT, SPI1_CS_PIN);
}

//-----------------------------------------------------------------------------
/** @brief Sends the response via CDC port
 *  @param pReq - Pointer to the request container
 *  @param size - Size of the response
 *  @return None
 */

static void cdc_SendResponse(SPI_PACKET * pRsp, U16 size)
{
  EventBits_t events = 0;

  /* Reset the EAST */
  EAST_SetBuffer(gPort.oEAST, (U8 *)pRsp, size);

  /* Send the response */
  CDC_SPI_InStage();

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
    /* Check for errors */
  }
}

//-----------------------------------------------------------------------------
/** @brief Thread function
 *  @param pvParameters - Pointer to the parameters
 *  @return None
 */

static void vSPITask(void * pvParameters)
{
  SPI_PACKET * req = NULL, * rsp = (SPI_PACKET *)gPort.oBuffer;
  U32 size = 0;

  while (FW_TRUE)
  {
    /* Dequeue the request */
    (void)BlockQueue_Dequeue(gPort.iQueue, (U8 **)&req, &size);

    /* Prepare the response */
    rsp->opcode = req->opcode;
    rsp->address = req->address;

    /* Process the request */
    if ((SPI_OPCODE_READ == req->opcode) && (4 == size))
    {
      cdc_SpiRead(req, rsp, &size);
    }
    else if ((SPI_OPCODE_WRITE == req->opcode) && (4 < size))
    {
      cdc_SpiWrite(req, rsp, &size);
    }
    else
    {
      cdc_SpiError(req, rsp, &size);
    }

    /* Send the response */
    cdc_SendResponse(rsp, size);

    /* Release the block */
    (void)BlockQueue_Release(gPort.iQueue);
  }
  //vTaskDelete(NULL);
}

//-----------------------------------------------------------------------------
/** @brief Initializes the SPI peripheral
 *  @param None
 *  @return None
 */

static void spi_Open(void)
{
  SPI_Init(SPI_1, spi_Complete);

  GPIO_Init(SPI1_CS_PORT,   SPI1_CS_PIN,   GPIO_TYPE_OUT_PP_2MHZ,  1);
  GPIO_Init(SPI1_SCK_PORT,  SPI1_SCK_PIN,  GPIO_TYPE_ALT_PP_50MHZ, 0);
  GPIO_Init(SPI1_MOSI_PORT, SPI1_MOSI_PIN, GPIO_TYPE_ALT_PP_50MHZ, 0);
  GPIO_Init(SPI1_MISO_PORT, SPI1_MISO_PIN, GPIO_TYPE_ALT_PP_50MHZ, 0);
  GPIO_Init(SPI1_DTR_PORT,  SPI1_DTR_PIN,  GPIO_TYPE_OUT_PP_10MHZ, 1);
  GPIO_Init(SPI1_RTS_PORT,  SPI1_RTS_PIN,  GPIO_TYPE_OUT_PP_10MHZ, 1);

  xEventGroupClearBits(gPort.events, EVT_SPI_EXCH_COMPLETE);
}

//-----------------------------------------------------------------------------
/** @brief Sets CDC handshake signals
 *  @param None
 *  @return None
 */

static void spi_SetControlLine(U16 aValue)
{
  /* DTR signal */
  if ( 0 == (aValue & (1 << CDC_CTRL_LINE_STATE_DTR)) )
  {
    GPIO_Hi(SPI1_DTR_PORT, SPI1_DTR_PIN);
  }
  else
  {
    GPIO_Lo(SPI1_DTR_PORT, SPI1_DTR_PIN);
  }

  /* RTS signal */
  if ( 0 == (aValue & (1 << CDC_CTRL_LINE_STATE_RTS)) )
  {
    GPIO_Hi(SPI1_RTS_PORT, SPI1_RTS_PIN);
  }
  else
  {
    GPIO_Lo(SPI1_RTS_PORT, SPI1_RTS_PIN);
  }
}

//-----------------------------------------------------------------------------
/** @brief Initializes CDC SPI
 *  @param None
 *  @return None
 */

void CDC_SPI_Init(void)
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
  gNotification.wIndex                     = USBD_CDC_SPI_GetInterfaceNumber();
  gNotification.wLength                    = 2;
  gNotification.Data.Raw                   = 0;

  /* Clear Port context */
  memset(&gPort, 0, sizeof(gPort));
  /* Port is not ready yet */
  gPort.cdc.ready = FW_FALSE;

  /* Initialize Endpoints */
  gPort.fpEpOBlkRd          = USBD_CDC_SPI_OEndPointRdWsCb;
  gPort.fpEpOBlkIsRxEmpty   = USBD_CDC_SPI_OEndPointIsRxEmpty;
  gPort.fpEpIBlkWr          = USBD_CDC_SPI_IEndPointWrWsCb;
  gPort.fpEpIBlkIsTxEmpty   = USBD_CDC_SPI_IEndPointIsTxEmpty;
  gPort.cdc.fpEpIIrqWr      = USBD_CDC_SPI_IrqEndPointWr;
  gPort.cdc.fpOpen          = spi_Open;
  gPort.cdc.fpSetCtrlLine   = spi_SetControlLine;

  /* Initialize EAST packet containers */
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

  /* Initialize the EAST packet queue */
  gPort.iQueue = BlockQueue_Init
                 (
                   gPort.iBuffer,
                   sizeof(gPort.iBuffer),
                   EAST_MAX_DATA_LENGTH
                 );
  /* Allocate the memory for the first input EAST packet */
  (void)BlockQueue_Allocate(gPort.iQueue, &buffer, &size);
  /* Setup/Reset the input EAST packet */
  (void)EAST_SetBuffer(gPort.iEAST, buffer, size);

  /* Initialize pointers */
  gPort.cdc.lineCoding = &gLineCoding;
  gPort.cdc.notification = &gNotification;

  /* Create the event group for synchronization */
  gPort.events = xEventGroupCreate();
  (void)xEventGroupClearBits
        (
          gPort.events,
          EVT_EAST_TX_COMPLETE | EVT_SPI_EXCH_COMPLETE
        );

  /* Create the I2C task */
  xTaskCreate
  (
    vSPITask,
    "SPI",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );
}

//-----------------------------------------------------------------------------
/** @brief Returns pointer to CDC SPI instance
 *  @param None
 *  @return CDC SPI instance
 */

CDC_PORT * CDC_SPI_GetPort(void)
{
  return &gPort.cdc;
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param None
 *  @return None
 */

void CDC_SPI_OutStage(void)
{
  /* Read from OUT EP */
  (void)gPort.fpEpOBlkRd(cdc_IEastPut, EAST_MAX_PACKET_LENGTH);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param None
 *  @return None
 */

void CDC_SPI_InStage(void)
{
  /* If there are some data in EAST packet */
  if (0 < EAST_GetPacketSize(gPort.oEAST))
  {
    /* Write to IN EP */
    (void)gPort.fpEpIBlkWr(cdc_OEastGet, EAST_GetPacketSize(gPort.oEAST));
  }
}

//-----------------------------------------------------------------------------
/** @brief CDC SOF Callback
 *  @param None
 *  @return None
 */

void CDC_SPI_ProcessCollectedData(void)
{
  //
}

//-----------------------------------------------------------------------------
