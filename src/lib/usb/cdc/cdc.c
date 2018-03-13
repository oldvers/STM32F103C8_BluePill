#include "types.h"

#include "usb.h"
#include "usb_cfg.h"
#include "usb_defs.h"
#include "usb_core.h"
#include "cdc_defs.h"
#include "cdc.h"
#include "debug.h"

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

/* Global Variables */
static CDC_LINE_CODING gLineCoding = {115200, 0, 0, 8};
static U8              gOBuffer[USB_CDC_PACKET_SIZE];
static U8              gIBuffer[USB_CDC_PACKET_SIZE];

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
    case CDC_REQ_SEND_ENCAPSULATED_COMMAND:
      LOG("CDC Setup: Send Enc Cmd\r\n");
      break;
    case CDC_REQ_GET_ENCAPSULATED_RESPONSE:
      LOG("CDC Setup: Get Enc Rsp\r\n");
      break;
    case CDC_REQ_SET_COMM_FEATURE:
      LOG("CDC Setup: Set Comm Feature\r\n");
      break;
    case CDC_REQ_GET_COMM_FEATURE:
      LOG("CDC Setup: Get Comm Feature\r\n");
      break;
    case CDC_REQ_CLEAR_COMM_FEATURE:
      LOG("CDC Setup: Clear Comm Feature\r\n");
      break;
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
      result = USB_CTRL_STAGE_STATUS;
      break;
    case CDC_REQ_SEND_BREAK:
      LOG("CDC Setup: Send Break\r\n");
      break;
    case CDC_REQ_NO_CMD:
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
    case CDC_REQ_SEND_ENCAPSULATED_COMMAND:
      LOG("CDC Out: Send Enc Cmd\r\n");
      break;
    case CDC_REQ_GET_ENCAPSULATED_RESPONSE:
      LOG("CDC Out: Get Enc Rsp\r\n");
      break;
    case CDC_REQ_SET_COMM_FEATURE:
      LOG("CDC Out: Set Comm Feature\r\n");
      break;
    case CDC_REQ_GET_COMM_FEATURE:
      LOG("CDC Out: Get Comm Feature\r\n");
      break;
    case CDC_REQ_CLEAR_COMM_FEATURE:
      LOG("CDC Out: Clear Comm Feature\r\n");
      break;
    case CDC_REQ_SET_LINE_CODING:
      result = USB_CTRL_STAGE_STATUS;
      LOG("CDC Out: Set Line Coding: Baud = %d\r\n", gLineCoding.dwBaudRate);
      break;
    case CDC_REQ_GET_LINE_CODING:
      LOG("CDC Out: Get Line Coding\r\n");
      break;
//    case CDC_REQ_SET_CONTROL_LINE_STATE:
//      LOG("CDC Out: Set Ctrl Line State\r\n");
//      result = USB_CTRL_STAGE_STATUS;
//      break;
    case CDC_REQ_SEND_BREAK:
      LOG("CDC Out: Send Break\r\n");
      break;
    case CDC_REQ_NO_CMD:
      break;
  }
  
  return result;
}

//-----------------------------------------------------------------------------
/** @brief CDC Interrupt In Callback
 *  @param aEvent - Event
 *  @return None
 */
void CDC_InterruptIn(U32 aEvent)
{
  LOG("CDC IRQ IN\r\n");
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk In Callback
 *  @param aEvent - Event
 *  @return None
 */
void CDC_BulkIn(U32 aEvent)
{
  LOG("CDC BULK IN\r\n");
}

//-----------------------------------------------------------------------------
/** @brief CDC Bulk Out Callback
 *  @param aEvent - Event
 *  @return None
 */
void CDC_BulkOut(U32 aEvent)
{
  U32 count;
  
  count = USB_EpRead(USB_CDC_EP_BULK_OUT, gOBuffer);
  LOG("CDC BULK OUT: Cnt = %d\r\n", count);
  USB_EpWrite(USB_CDC_EP_BULK_IN, gOBuffer, count);
}

//-----------------------------------------------------------------------------
/** @brief Initializes CDC
 *  @param None
 *  @return None
 */
void CDC_Init(void)
{
  //
}
