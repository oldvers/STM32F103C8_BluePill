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

/* Serial Port Context */
typedef struct _CDC_PORT
{
  U8                epBlkO;
  U8                epBlkI;
  U8                epIrqI;
  U8                irqBuffLen;
  FIFO_t            rxFifo;
  FIFO_t            txFifo;
  U8                rxBuffer[USB_CDC_PACKET_SIZE * 4 + 1];
  U8                txBuffer[USB_CDC_PACKET_SIZE * 4 + 1];
  U8               *irqBuff;
  CDC_LINE_CODING  *lineCoding;
  CDC_SERIAL_STATE *notification;
//  SemaphoreHandle_t vcpSemRx;
//  SemaphoreHandle_t vcpMutRx;
//  SemaphoreHandle_t vcpSemTx;
//  SemaphoreHandle_t vcpMutTx;
//  U8                vcpReading;
//  EAST_STATE        gVcpRxState;
//  EAST_STATE        gVcpTxState;
} CDC_PORT;

//-----------------------------------------------------------------------------
/* Global Variables */
STATIC CDC_LINE_CODING   gLineCodingA =
{
  115200, /* dwBaudRate */
  0,      /* bCharFormat */
  0,      /* bParityType */
  8,      /* bDataBits */
};
STATIC CDC_SERIAL_STATE  gNotificationA =
{
  /* bmRequestType */
  {REQUEST_TO_INTERFACE, REQUEST_CLASS, REQUEST_DEVICE_TO_HOST},
  CDC_NTF_SERIAL_STATE, /* bNotification */
  0,                    /* wValue */
  USB_CDC_IF_NUM0,      /* wIndex */
  2,                    /* wLength */
  0,                    /* Data */
};
STATIC CDC_PORT          gPortA = {0};

#if (USB_CDD)
STATIC CDC_LINE_CODING   gLineCodingB =
{
  115200, /* dwBaudRate */
  0,      /* bCharFormat */
  0,      /* bParityType */
  8,      /* bDataBits */
};
STATIC CDC_SERIAL_STATE  gNotificationB =
{
  /* bmRequestType */
  {REQUEST_TO_INTERFACE, REQUEST_CLASS, REQUEST_DEVICE_TO_HOST},
  CDC_NTF_SERIAL_STATE, /* bNotification */
  0,                    /* wValue */
  USB_CDC_IF_NUM0,      /* wIndex */
  2,                    /* wLength */
  0,                    /* Data */
};
STATIC CDC_PORT          gPortB = {0};
#endif /* USB_CDD */

//-----------------------------------------------------------------------------
/* Private Functions declarations */
//void cdc_InStage(void);
//void cdc_OutStage(void);



//-----------------------------------------------------------------------------
/** @brief Processes IRQ EP data
 *  @param pPort - Pointer to Port context
 *  @return None
 */
void cdc_IrqInStage(CDC_PORT * pPort)
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

  LOG("CDC IRQ IN: len = %d\r\n", len);
  USB_EpWrite(pPort->epIrqI, pPort->irqBuff, len);

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

  switch (pSetup->bRequest)
  {
    case CDC_REQ_SET_LINE_CODING:
      LOG("CDC Setup: SetLineCoding: IF = %d L = %d\r\n",
            pSetup->wIndex.W, *pSize);

      if (USB_CDC_IF_NUM0 == pSetup->wIndex.W)
      {
        *pData = (U8 *)gPortA.lineCoding;
      }
#if (USB_CDD)
      else
      {
        *pData = (U8 *)gPortB.lineCoding;
      }
#endif
      result = USB_CTRL_STAGE_WAIT;
      break;
    case CDC_REQ_GET_LINE_CODING:
      LOG("CDC Setup: GetLineCoding: IF = %d\r\n", pSetup->wIndex.W);
      if (USB_CDC_IF_NUM0 == pSetup->wIndex.W)
      {
        *pData = (U8 *)gPortA.lineCoding;
      }
#if (USB_CDD)
      else
      {
        *pData = (U8 *)gPortB.lineCoding;
      }
#endif
      result = USB_CTRL_STAGE_DATA;
      break;
    case CDC_REQ_SET_CONTROL_LINE_STATE:
      LOG("CDC Setup: Set Ctrl Line State\r\n");
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
      if (USB_CDC_IF_NUM0 == pSetup->wIndex.W)
      {
        LOG("CDC Out: Set Line Coding: Baud = %d, Len = %d\r\n",
              gPortA.lineCoding->dwBaudRate, *pSize);
      }
#if (USB_CDD)
      else
      {
        LOG("CDC Out: Set Line Coding: Baud = %d, Len = %d\r\n",
              gPortB.lineCoding->dwBaudRate, *pSize);
      }
#endif
      result = USB_CTRL_STAGE_STATUS;
      break;
  }
  
  return result;
}

//-----------------------------------------------------------------------------
/** @brief Processes OUT EP data
 *  @param pPort - Pointer to Port context
 *  @return None
 */
void cdc_OutStage(CDC_PORT * pPort)
{
  U32 len, i = 0;

  /* Read from OUT EP */
  len = USB_EpRead(pPort->epBlkO, pPort->rxBuffer, USB_CDC_PACKET_SIZE);
  LOG("CDC OUT: len = %d\r\n", len);

//  /* If there is no reading in progress - ignore */
//  LOG(" EAST Rx:");
//  while ((FW_TRUE == gVcpReading) && (i < len))
//  {
//    LOG(" %0.2X", gOBuffer[i]);
//    EAST_PutByte(&gVcpRxState, gOBuffer[i++]);
//  }
//  LOG("\r\n");
}

//-----------------------------------------------------------------------------
/** @brief Processes IN EP data
 *  @param pPort - Pointer to Port context
 *  @return None
 */
void cdc_InStage(CDC_PORT * pPort)
{
  U8 data, len = 0;

//  LOG(" EAST Tx:");
//  while
//  (
//    (len < USB_CDC_PACKET_SIZE) &&
//    (FW_TRUE == EAST_GetByte(&gVcpTxState, &data))
//  )
//  {
//    LOG(" %0.2X", data);
//    gIBuffer[len++] = data;
//  }
//  LOG("\r\n");

  if (0 < len)
  {
    len = USB_EpWrite(pPort->epBlkI, pPort->txBuffer, len);
    LOG("CDC IN: len = %d\r\n", len);
  }
}









//-----------------------------------------------------------------------------
/** @brief Opens Virtual COM Port
 *  @param None
 *  @return TRUE - Port opened, FALSE - Error
 */
U32 VCP_Open(void)
{
  /* Check if VCP Semaphore/Mutex were created successfuly */
//  if ( (NULL == gVcpSemRx) || (NULL == gVcpMutRx) ||
//       (NULL == gVcpSemTx) || (NULL == gVcpMutTx) )
//  {
//    return FW_FALSE;
//  }
//
//  /* Indicate that there is no reading in progress */
//  gVcpReading = FW_FALSE;
  
  return FW_TRUE;
}

//-----------------------------------------------------------------------------
/** @brief Callback function - VCP read completed
 *  @param None
 *  @return None
 */
void cdc_OutCompleted(void)
{
//  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//  gVcpReading = FW_FALSE;
//  xSemaphoreGiveFromISR(gVcpSemRx, &xHigherPriorityTaskWoken);
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

//  if (pdFALSE == xSemaphoreTake(gVcpMutRx, portMAX_DELAY))
//  {
//    return 0;
//  }
//
//  /* Init EAST state for reading */
//  gVcpRxState.MaxSize = aSize;
//  gVcpRxState.ActSize = 0;
//  gVcpRxState.Index = 0;
//  gVcpRxState.Buffer = pData;
//  gVcpRxState.OnComplete = cdc_OutCompleted;
//
//  /* Indicate VCP reading is in progress */
//  gVcpReading = FW_TRUE;
//
//  /* Wait for Rx Complete */
//  if (pdTRUE == xSemaphoreTake(gVcpSemRx, aTimeout))
//  {
//    result = gVcpRxState.ActSize;
//  }
//  else
//  {
//    result = 0;
//  }
//
//  xSemaphoreGive(gVcpMutRx);

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Callback function - VCP write completed
 *  @param None
 *  @return None
 */
void cdc_InCompleted(void)
{
//  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//  xSemaphoreGiveFromISR(gVcpSemTx, &xHigherPriorityTaskWoken);
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

//  if (pdFALSE == xSemaphoreTake(gVcpMutTx, portMAX_DELAY))
//  {
//    return 0;
//  }
//
//  /* Init EAST state for writing */
//  gVcpTxState.MaxSize = aSize;
//  gVcpTxState.ActSize = 0;
//  gVcpTxState.Index = 0;
//  gVcpTxState.Buffer = pData;
//  gVcpTxState.OnComplete = cdc_InCompleted;
//
//  /* Process IN EP */
//  cdc_InStage();
//
//  /* Wait for Tx complete */
//  if (pdTRUE == xSemaphoreTake(gVcpSemTx, aTimeout))
//  {
//    result = gVcpTxState.ActSize;
//  }
//  else
//  {
//    result = 0;
//  }
//
//  xSemaphoreGive(gVcpMutTx);

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
//  gVcpReading = FW_FALSE;
}











//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */
void cdc_InterruptAIn(U32 aEvent)
{
  cdc_IrqInStage(&gPortA);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */
void cdc_BulkAIn(U32 aEvent)
{
  //cdc_InStage();
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */
void cdc_BulkAOut(U32 aEvent)
{
  //cdc_OutStage();
}

#if (USB_CDD)
//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */
void cdc_InterruptBIn(U32 aEvent)
{
  cdc_IrqInStage(&gPortB);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */
void cdc_BulkBIn(U32 aEvent)
{
  //cdc_InStage();
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */
void cdc_BulkBOut(U32 aEvent)
{
  //cdc_OutStage();
}
#endif /* USB_CDD */

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
  USB_SetCb_Ep(USB_CDC_EP_IRQ_I, cdc_InterruptAIn);
  /* Clear Port context */
  memset(&gPortA, 0, sizeof(gPortA));
  /* Initialize Endpoints */
  gPortA.epBlkO = USB_CDC_EP_BLK_O;
  gPortA.epBlkI = USB_CDC_EP_BLK_I;
  gPortA.epIrqI = USB_CDC_EP_IRQ_I;
  /* Initialize FIFOs */
  FIFO_Init(&gPortA.rxFifo, gPortA.rxBuffer, sizeof(gPortA.rxBuffer));
  FIFO_Init(&gPortA.txFifo, gPortA.txBuffer, sizeof(gPortA.txBuffer));
  /* Initialize pointers */
  gPortA.lineCoding = &gLineCodingA;
  gPortA.notification = &gNotificationA;
  
#if (USB_CDD)
  /* Register appropriate EP callbacks */
  USB_SetCb_Ep(USB_CDC_EP_BLK_O, cdc_BulkBOut);
  USB_SetCb_Ep(USB_CDC_EP_BLK_I, cdc_BulkBIn);
  USB_SetCb_Ep(USB_CDC_EP_IRQ_I, cdc_InterruptBIn);
  /* Clear Port context */
  memset(&gPortB, 0, sizeof(gPortB));
  /* Initialize Endpoints */
  gPortB.epBlkO = USB_CDD_EP_BLK_O;
  gPortB.epBlkI = USB_CDD_EP_BLK_I;
  gPortB.epIrqI = USB_CDD_EP_IRQ_I;
  /* Initialize FIFOs */
  FIFO_Init(&gPortB.rxFifo, gPortB.rxBuffer, sizeof(gPortB.rxBuffer));
  FIFO_Init(&gPortB.txFifo, gPortB.txBuffer, sizeof(gPortB.txBuffer));
  /* Initialize pointers */
  gPortB.lineCoding = &gLineCodingB;
  gPortB.notification = &gNotificationB;
#endif

  /* Create Semaphores/Mutex for VCP */
//  gVcpSemRx = xSemaphoreCreateBinary();
//  gVcpMutRx = xSemaphoreCreateMutex();
//  gVcpSemTx = xSemaphoreCreateBinary();
//  gVcpMutTx = xSemaphoreCreateMutex();
}