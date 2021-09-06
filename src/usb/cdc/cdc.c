#include "types.h"

#include "usb.h"
#include "usb_definitions.h"
#include "usb_descriptor.h"
#include "usb_cdc_definitions.h"
#include "cdc.h"
#include "cdc_private.h"
#include "cdc_uart.h"
#include "cdc_i2c.h"
#include "cdc_spi.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include "debug.h"
#include "string.h"

//-----------------------------------------------------------------------------
/* Private definitions */

#define CDC_DEBUG

#ifdef CDC_DEBUG
#  define CDC_LOG    DBG
#else
#  define CDC_LOG(...)
#endif

//-----------------------------------------------------------------------------
/** @brief Gets CDC Port according to USB Interface Number
 *  @param aInterface - USB Interface Number
 *  @return Pointer to the CDC Port Context
 */

static CDC_PORT * cdc_GetPort(U16 aInterface)
{
  CDC_PORT * result = NULL;

  if (USBD_CDC_UART_GetInterfaceNumber() == aInterface)
  {
    result = CDC_UART_GetPort();
  }
  else if (USBD_CDC_I2C_GetInterfaceNumber() == aInterface)
  {
    result = CDC_I2C_GetPort();
  }
  else
  {
    result = CDC_I2C_GetPort();
  }

  return (result);
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
  (void)pPort->fpEpIIrqWr(pPort->irqBuff, len);

  pPort->irqBuff += len;
  pPort->irqBuffLen -= len;
}

//-----------------------------------------------------------------------------
/** @brief Sends Serial State notification
 *  @param pPort - Pointer to Port context
 *  @param aState - Errors/Evetns state
 *  @return None
 */

void CDC_NotifyState(CDC_PORT * pPort, U16 aState)
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
  CDC_PORT * port = NULL;

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
      port->fpSetCtrlLine(pSetup->wValue.W);

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
  CDC_PORT * port = NULL;

  switch (pSetup->bRequest)
  {
    case CDC_REQ_SET_LINE_CODING:
      port = cdc_GetPort(pSetup->wIndex.W);
      port->fpOpen();
      port->ready = FW_TRUE;

      CDC_LOG("CDC Out: Set Line Coding: IF = %d Baud = %d, Len = %d\r\n",
              pSetup->wIndex.W, port->lineCoding->dwBaudRate, *pSize);

      result = USB_CTRL_STAGE_STATUS;
      break;
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Called on each USB Start Of Frame (every 1 ms)
 *  @param None
 *  @return None
 */

void CDC_UART_SOF(void)
{
  CDC_UART_ProcessCollectedData();
}

//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_UART_InterruptIn(U32 aEvent)
{
  cdc_IrqInStage(CDC_UART_GetPort());
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_UART_BulkIn(U32 aEvent)
{
  CDC_UART_InStage();
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_UART_BulkOut(U32 aEvent)
{
  CDC_UART_OutStage();
}










//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_I2C_InterruptIn(U32 aEvent)
{
//  cdc_IrqInStage(&gPortI2C);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_I2C_BulkIn(U32 aEvent)
{
//  cdc_InStage(&gPortI2C);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_I2C_BulkOut(U32 aEvent)
{
//  cdc_OutStage(&gPortI2C);
}







//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_SPI_InterruptIn(U32 aEvent)
{
//  cdc_IrqInStage(&gPortSPI);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_SPI_BulkIn(U32 aEvent)
{
//  cdc_InStage(&gPortSPI);
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */

void CDC_SPI_BulkOut(U32 aEvent)
{
//  cdc_OutStage(&gPortSPI);
}







//-----------------------------------------------------------------------------
/** @brief Initializes CDC
 *  @param None
 *  @return None
 */

void CDC_Init(void)
{
  CDC_UART_Init();
  CDC_I2C_Init();
  CDC_SPI_Init();
}