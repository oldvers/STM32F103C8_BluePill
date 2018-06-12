#include "types.h"

#include "usb.h"
#include "usb_cfg.h"
#include "usb_defs.h"
#include "usb_core.h"
#include "cdc_defs.h"
#include "cdc.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "east.h"

#include "debug.h"

//-----------------------------------------------------------------------------
/* Private Types definitions */
/* Line Coding Structure */
typedef __packed struct _CDC_LINE_CODING
{
  U32 dwBaudRate;  /* Number Data terminal rate, in bits per second */
  U8  bCharFormat; /* Number of Stop bits */
                   /*   0 - 1 Stop bit    *
                    *   1 - 1.5 Stop bits *
                    *   2 - 2 Stop bits   */
  U8  bParityType; /* Number Parity */
                   /*   0 - None    *
                    *   1 - Odd     *
                    *   2 - Even    *
                    *   3 - Mark    *
                    *   4 - Space   */
  U8  bDataBits;   /* Number Data Bits (5, 6, 7, 8 or 16) */
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

//-----------------------------------------------------------------------------
/* Global Variables */
static CDC_LINE_CODING   gLineCoding =
{
  115200, /* dwBaudRate */
  0,      /* bCharFormat */
  0,      /* bParityType */
  8,      /* bDataBits */
};
static CDC_SERIAL_STATE  gNotification =
{
  /* bmRequestType */
  {REQUEST_TO_INTERFACE, REQUEST_CLASS, REQUEST_DEVICE_TO_HOST},
  CDC_NTF_SERIAL_STATE, /* bNotification */
  0,                    /* wValue */
  USB_CDC_IF_NUM0,      /* wIndex */
  2,                    /* wLength */
  0,                    /* Data */
};
static U8                gOBuffer[USB_CDC_PACKET_SIZE];
static U8                gIBuffer[USB_CDC_PACKET_SIZE];
static U8               *gIrqBuff = NULL;
static U8                gIrqBuffLen = 0;
static SemaphoreHandle_t gVcpSemRx = NULL;
static SemaphoreHandle_t gVcpMutRx = NULL;
static SemaphoreHandle_t gVcpSemTx = NULL;
static SemaphoreHandle_t gVcpMutTx = NULL;
static U8                gVcpReading = FALSE;
static U8                gVcpOpened  = FALSE;
static EAST_STATE        gVcpRxState;
static EAST_STATE        gVcpTxState;

//-----------------------------------------------------------------------------
/* Private Functions declarations */
void cdc_InStage(void);
void cdc_OutStage(void);

//-----------------------------------------------------------------------------
/** @brief Processes IRQ EP data
 *  @param None
 *  @return None
 */
void cdc_IrqInStage(void)
{
  U8 len = 0;

  if (0 == gIrqBuffLen) return;

  if (USB_CDC_IRQ_PACKET_SIZE < gIrqBuffLen)
  {
    len = USB_CDC_IRQ_PACKET_SIZE;
  }
  else
  {
    len = gIrqBuffLen;
  }

  LOG("CDC IRQ IN: len = %d\r\n", len);
  USB_EpWrite(USB_CDC_EP_IRQ_IN, gIrqBuff, len);

  gIrqBuff += len;
  gIrqBuffLen -= len;
}

//-----------------------------------------------------------------------------
/** @brief Sends Serial State notification
 *  @param aState - Errors/Evetns state
 *  @return None
 */
void cdc_NotifyState(U16 aState)
{
  gNotification.Data.Raw = aState;
  gIrqBuff = (U8 *)&gNotification;
  gIrqBuffLen = sizeof(gNotification);
  
  cdc_IrqInStage();
}

//-----------------------------------------------------------------------------
/** @brief HID Control Setup USB Request
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
  
  switch (pSetup->bRequest)
  {
    case CDC_REQ_SET_LINE_CODING:
      LOG("CDC Setup: Set Line Coding: Len = %d\r\n", *pSize);
      *pData = (U8 *)&gLineCoding;
      result = USB_CTRL_STAGE_WAIT;
      break;
    case CDC_REQ_GET_LINE_CODING:
      LOG("CDC Setup: Get Line Coding\r\n");
      *pData = (U8 *)&gLineCoding;
      result = USB_CTRL_STAGE_DATA;
      break;
    case CDC_REQ_SET_CONTROL_LINE_STATE:
      LOG("CDC Setup: Set Ctrl Line State\r\n");
      gVcpOpened = (1 == (pSetup->wValue.W & 1));
      if (TRUE == gVcpOpened)
      {
        LOG("CDC Setup: OPENED\r\n");
      }
      else
      {
        LOG("CDC Setup: CLOSED\r\n");
      }
      result = USB_CTRL_STAGE_STATUS;
      break;
  }
  
  return result;
}

//-----------------------------------------------------------------------------
/** @brief HID USB Out Request
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
  
  switch (pSetup->bRequest)
  {
    case CDC_REQ_SET_LINE_CODING:
      LOG("CDC Out: Set Line Coding: Baud = %d, Len = %d\r\n", gLineCoding.dwBaudRate, *pSize);
      result = USB_CTRL_STAGE_STATUS;
      break;
  }
  
  return result;
}

//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */
void cdc_InterruptIn(U32 aEvent)
{
  cdc_IrqInStage();
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */
void cdc_BulkIn(U32 aEvent)
{
  cdc_InStage();
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */
void cdc_BulkOut(U32 aEvent)
{
  cdc_OutStage();
}

//-----------------------------------------------------------------------------
/** @brief Initializes CDC
 *  @param None
 *  @return None
 */
void CDC_Init(void)
{
  /* Register appropriate EP callbacks */
  USB_SetCb_Ep(USB_CDC_EP_BULK_OUT, cdc_BulkOut);
  USB_SetCb_Ep(USB_CDC_EP_BULK_IN,  cdc_BulkIn);
  USB_SetCb_Ep(USB_CDC_EP_IRQ_IN,   cdc_InterruptIn);

  /* Create Semaphores/Mutex for VCP */
  gVcpSemRx = xSemaphoreCreateBinary();
  gVcpMutRx = xSemaphoreCreateMutex();
  gVcpSemTx = xSemaphoreCreateBinary();
  gVcpMutTx = xSemaphoreCreateMutex();
}

//-----------------------------------------------------------------------------
/** @brief Opens Virtual COM Port
 *  @param None
 *  @return TRUE - Port opened, FALSE - Error
 */
U32 VCP_Open(void)
{
  /* Check if VCP Semaphore/Mutex were created successfuly */
  if ( (NULL == gVcpSemRx) || (NULL == gVcpMutRx) ||
       (NULL == gVcpSemTx) || (NULL == gVcpMutTx) )
  {
    return FALSE;
  }

  /* Indicate that there is no reading in progress */
  gVcpReading = FALSE;
  
  return TRUE;
}

//-----------------------------------------------------------------------------
/** @brief Callback function - VCP read completed
 *  @param None
 *  @return None
 */
void cdc_OutCompleted(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  gVcpReading = FALSE;
  xSemaphoreGiveFromISR(gVcpSemRx, &xHigherPriorityTaskWoken);
}

//-----------------------------------------------------------------------------
/** @brief Processes OUT EP data
 *  @param None
 *  @return None
 */
void cdc_OutStage(void)
{
  U32 len, i = 0;

  /* Read from OUT EP */
  len = USB_EpRead(USB_CDC_EP_BULK_OUT, gOBuffer);
  //LOG("CDC OUT: len = %d\r\n", len);

  /* If there is no reading in progress - ignore */
  while ((TRUE == gVcpReading) && (i < len))
  {
    EAST_PutByte(&gVcpRxState, gOBuffer[i++]);
  }
}

//-----------------------------------------------------------------------------
/** @brief Reads from Vortual COM Port
 *  @param pData - Buffer for data to be read
 *  @param aSize - Size of data to be read
 *  @param aTimeout - Time to waiting a data
 *  @return Number of bytes read
 */
U32 VCP_Read(U8 * pData, U32 aSize, U32 aTimeout)
{
  U32 result = 0;

  if (pdFALSE == xSemaphoreTake(gVcpMutRx, portMAX_DELAY))
  {
    return 0;
  }

  /* Init EAST state for reading */
  gVcpRxState.MaxSize = aSize;
  gVcpRxState.ActSize = 0;
  gVcpRxState.Index = 0;
  gVcpRxState.Buffer = pData;
  gVcpRxState.OnComplete = cdc_OutCompleted;

  /* Indicate VCP reading is in progress */
  gVcpReading = TRUE;

  /* Wait for Rx Complete */
  if (pdTRUE == xSemaphoreTake(gVcpSemRx, aTimeout))
  {
    result = gVcpRxState.ActSize;
  }
  else
  {
    result = 0;
  }

  xSemaphoreGive(gVcpMutRx);

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Callback function - VCP write completed
 *  @param None
 *  @return None
 */
void cdc_InCompleted(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(gVcpSemTx, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

//-----------------------------------------------------------------------------
/** @brief Processes IN EP data
 *  @param None
 *  @return None
 */
void cdc_InStage(void)
{
  U8 data, len = 0;

  while
  (
    (len < USB_CDC_PACKET_SIZE) &&
    (TRUE == EAST_GetByte(&gVcpTxState, &data))
  )
  {
    gIBuffer[len++] = data;
  }

  //LOG("CDC IN: len = %d\r\n", len);
  if (0 < len) USB_EpWrite(USB_CDC_EP_BULK_IN, gIBuffer, len);
}

//-----------------------------------------------------------------------------
/** @brief Writes to Virtual COM Port
 *  @param pData - Buffer of data to be written
 *  @param aSize - Size of data to be written
 *  @return Number of bytes written
 */
U32 VCP_Write(U8 * pData, U32 aSize, U32 aTimeout)
{
  U32 result = 0;

  if (FALSE == gVcpOpened)
  {
    return 0;
  }

  if (pdFALSE == xSemaphoreTake(gVcpMutTx, portMAX_DELAY))
  {
    return 0;
  }

  /* Init EAST state for writing */
  gVcpTxState.MaxSize = aSize;
  gVcpTxState.ActSize = 0;
  gVcpTxState.Index = 0;
  gVcpTxState.Buffer = pData;
  gVcpTxState.OnComplete = cdc_InCompleted;

  /* Process IN EP */
  cdc_InStage();

  /* Wait for Tx complete */
  if (pdTRUE == xSemaphoreTake(gVcpSemTx, aTimeout))
  {
    result = gVcpTxState.ActSize;
  }
  else
  {
    result = 0;
  }

  xSemaphoreGive(gVcpMutTx);

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Closes Virtual COM Port
 *  @param None
 *  @return None
 */
void VCP_Close(void)
{
  /* Indicate that there is no reading in progress */
  gVcpReading = FALSE;
}
