#include "types.h"

#include "usb.h"
#include "usb_cfg.h"
#include "usb_defs.h"
#include "usb_core.h"
#include "hid_defs.h"
#include "hid.h"
#include "debug.h"

static U8 gProtocol;
static U8 gIdleTime[HID_REPORT_NUM];
static U8 gIReport = 0x03;
static U8 gOReport = 0x00;

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
USB_CTRL_STAGE HID_CtrlSetupReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
)
{
  USB_CTRL_STAGE result = USB_CTRL_STAGE_ERROR;

  switch (pSetup->bRequest)
  {
    case HID_REQUEST_GET_REPORT:
      /* ReportID = SetupPacket.wValue.WB.L */
      switch (pSetup->wValue.WB.H)
      {
        case HID_REPORT_INPUT:
          *pData[0] = gIReport;
          result = USB_CTRL_STAGE_DATA;
          break;
        case HID_REPORT_OUTPUT:
          /* Not Supported */
          break;
        case HID_REPORT_FEATURE:
          /* *pData[] = ...; */
          /* Not Supported */
          break;
      }
      break;
      
    case HID_REQUEST_SET_REPORT:
      result = USB_CTRL_STAGE_WAIT;
      break;
      
    case HID_REQUEST_GET_IDLE:
      *pData[0] = gIdleTime[pSetup->wValue.WB.L];
      result = USB_CTRL_STAGE_DATA;
      break;

    case HID_REQUEST_SET_IDLE:
      gIdleTime[pSetup->wValue.WB.L] = pSetup->wValue.WB.H;
      result = USB_CTRL_STAGE_STATUS;
      break;

    case HID_REQUEST_GET_PROTOCOL:
      *pData[0] = gProtocol;
      result = USB_CTRL_STAGE_DATA;
      break;

    case HID_REQUEST_SET_PROTOCOL:
      gProtocol = pSetup->wValue.WB.L;
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
USB_CTRL_STAGE HID_CtrlOutReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
)
{
  USB_CTRL_STAGE result = USB_CTRL_STAGE_ERROR;

  switch (pSetup->bRequest)
  {
    case HID_REQUEST_SET_REPORT:
      /* ReportID = SetupPacket.wValue.WB.L */
      switch (pSetup->wValue.WB.H)
      {
        case HID_REPORT_INPUT:
          /* Not Supported */
          break;
        case HID_REPORT_OUTPUT:
          gOReport = *pData[0];
          gIReport = gOReport;
          result = USB_CTRL_STAGE_STATUS;
          break;
        case HID_REPORT_FEATURE:
          /* Not Supported */
          break;
      }
      break;
  }
  
  return result;
}

//-----------------------------------------------------------------------------
/** @brief Initializes HID
 *  @param None
 *  @return None
 */
void HID_Init(void)
{
  //
}

//-----------------------------------------------------------------------------
/** @brief HID IRQ In Callback
 *  @param aEvent - Event
 *  @return None
 */
void HID_InterruptIn(U32 aEvent)
{
  USB_EpWrite(USB_HID_EP_IRQ_IN, &gIReport, sizeof(gIReport));
}
