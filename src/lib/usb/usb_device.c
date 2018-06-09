#include "types.h"

#include "usb.h"
#include "usb_cfg.h"
#include "usb_defs.h"
#include "usb_core.h"
#include "msc.h"
#include "cdc.h"
#include "hid.h"

#include "debug.h"

static U8 gUsbdConfigured = FALSE;

//-----------------------------------------------------------------------------
/** @brief USB Device Reset Event Callback
 *  @param None
 *  @return None
 *  @note Called automatically on USB Reset Event
 */
#if USB_RESET_EVENT
static void usbd_CbReset(void)
{
  /* Reset Core */
  USBC_Reset();
  /* Turn Off Cfg LED */
  //GPIOB->ODR &= ~LED_CFG;
}
#endif

//-----------------------------------------------------------------------------
/** @brief USB Device Suspend Event Callback
 *  @param None
 *  @return None
 *  @note Called automatically on USB Suspend Event
 */
#if USB_SUSPEND_EVENT
static void usbd_CbSuspend(void)
{
  /* Turn On Suspend LED */
  //GPIOB->ODR |= LED_SUSP;
}
#endif

//-----------------------------------------------------------------------------
/** @brief USB Device Resume Event Callback
 *  @param None
 *  @return None
 *  @note Called automatically on USB Resume Event
 */
#if USB_RESUME_EVENT
static void usbd_CbResume(void)
{
  //
}
#endif

//-----------------------------------------------------------------------------
/** @brief USB Device Remote Wakeup Event Callback
 *  @param None
 *  @return None
 *  @note Called automatically on USB Remote Wakeup Event
 */
#if USB_WAKEUP_EVENT
static void usbd_CbWakeUp(void)
{
  /* Turn Off Suspend LED */
  //GPIOB->ODR &= ~LED_SUSP;
}
#endif

//-----------------------------------------------------------------------------
/** @brief USB Device Start of Frame Event Callback
 *  @param None
 *  @return None
 *  @note Called automatically on USB Start of Frame Event
 */
#if USB_SOF_EVENT
static void usbd_CbSOF(void)
{
  //
}
#endif

//-----------------------------------------------------------------------------
/** @brief USB Device Error Event Callback
 *  @param aError - Error Code
 *  @return None
 *  @note Called automatically on USB Error Event
 */
#if USB_ERROR_EVENT
static void usbd_CbError(U32 aError)
{
  //
}
#endif

//-----------------------------------------------------------------------------
/** @brief USB Core Set Configuration Event Callback
 *  @param aConfig - Configuration result
 *  @return None
 *  @note Called automatically on USB Set Configuration Request
 */
void usbc_CbConfigure(U8 aConfig)
{
#if USB_CONFIGURE_EVENT
  /* Check if USB is configured */
  if (aConfig)
  {
    /* Turn On Cfg LED */
    //GPIOB->ODR |=  LED_CFG;
    gUsbdConfigured = TRUE;
  }
  else
  {
    /* Turn Off Cfg LED */
    //GPIOB->ODR &= ~LED_CFG;
    gUsbdConfigured = FALSE;
  }
#endif
}

//-----------------------------------------------------------------------------
/** @brief USB Core Set Interface Event Callback
 *  @param None
 *  @return None
 *  @note Called automatically on USB Set Interface Request
 */
void usbc_CbInterface(void)
{
#if USB_INTERFACE_EVENT
  //
#endif
}

//-----------------------------------------------------------------------------
/** @brief USB Core Set/Clear Feature Event Callback
 *  @param None
 *  @return None
 *  @note Called automatically on USB Set/Clear Feature Request
 */
void usbc_CbFeature(void)
{
#if USB_FEATURE_EVENT
  //
#endif
}

//-----------------------------------------------------------------------------
/** @brief Class Control Setup USB Request
 *  @param pSetup - Pointer to Setup Packet
 *  @param pData - Pointer to place for setting the pointer to requested data
 *  @param pSize - Pointer to place for setting the requested data size
 *  @return Stage that should be performed after calling this function
 *  @note On calling this function pData points to Control Endpoint internal
 *        buffer so requested data can be placed right there if it is not
 *        exceeds Control Endpoint Max Packet size
 */
USB_CTRL_STAGE usbc_CbCtrlSetupReqClass
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
)
{
  USB_CTRL_STAGE result = USB_CTRL_STAGE_ERROR;

  if (REQUEST_TO_INTERFACE == pSetup->bmRequestType.BM.Recipient)
  {
    switch (pSetup->wIndex.WB.L)
    {
#if (USB_MSC)
      case USB_MSC_IF_NUM:
        result = MSC_CtrlSetupReq(pSetup, pData, pSize);
        break;
#endif
#if USB_CDC
      case USB_CDC_IF_NUM0:
        result = CDC_CtrlSetupReq(pSetup, pData, pSize);
        break;
#endif
#if USB_HID
      case USB_HID_IF_NUM:
        result = HID_CtrlSetupReq(pSetup, pData, pSize);
        break;
#endif
      default:
        break;
    }
  }
  else if (REQUEST_TO_ENDPOINT == pSetup->bmRequestType.BM.Recipient)
  {
    //
  }
  
  return result;
}

//-----------------------------------------------------------------------------
/** @brief Class USB Out Request
 *  @param pSetup - Pointer to Setup Packet
 *  @param pData - Pointer to place for setting the pointer to requested data
 *  @param pSize - Pointer to place for setting the requested data size
 *  @return Stage that should be performed after calling this function
 *  @note Called when all the OUT packets have been already collected
 */
USB_CTRL_STAGE usbc_CbCtrlOutReqClass
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
)
{
  USB_CTRL_STAGE result = USB_CTRL_STAGE_ERROR;

  if (REQUEST_TO_INTERFACE == pSetup->bmRequestType.BM.Recipient)
  {
    switch (pSetup->wIndex.WB.L)
    {
#if USB_CDC
      case USB_CDC_IF_NUM0:
        result = CDC_CtrlOutReq(pSetup, pData, pSize);
        break;
#endif
#if USB_HID
      case USB_HID_IF_NUM:
        result = HID_CtrlOutReq(pSetup, pData, pSize);
        break;
#endif
      default:
        break;
    }
  }
  else if (REQUEST_TO_ENDPOINT == pSetup->bmRequestType.BM.Recipient)
  {
    //
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief USB Core Event Callbacks Table
 *  @param None
 *  @return None
 *  @note Called automatically on USB Set/Clear Feature Request
 */
const USB_CORE_EVENTS USBC_Events =
{
  usbc_CbFeature,
  usbc_CbConfigure,
  usbc_CbInterface,
  usbc_CbCtrlSetupReqClass,
  usbc_CbCtrlOutReqClass,
};

//-----------------------------------------------------------------------------
/** @brief Returns USB Device configuration state
 *  @param None
 *  @return TRUE - Configured, FALSE - Not configured
 */
U8 USBD_IsConfigured(void)
{
  return gUsbdConfigured;
}

//-----------------------------------------------------------------------------
/** @brief Initializes USB Device
 *  @param None
 *  @return None
 *  @note Initializes USB peripheral, USB core, sets event callbacks
 */
void USBD_Init(void)
{
#if USB_RESET_EVENT
  USB_SetCb_Reset(usbd_CbReset);
#endif
#if USB_SUSPEND_EVENT
  USB_SetCb_Suspend(usbd_CbSuspend);
#endif
#if USB_WAKEUP_EVENT
  USB_SetCb_WakeUp(usbd_CbWakeUp);
#endif
#if USB_SOF_EVENT
  USB_SetCb_SOF(usbd_CbSOF);
#endif
#if USB_ERROR_EVENT
  USB_SetCb_Error(usbd_CbError);
#endif

  /* Init Hardware */
  USB_Init(USB_EP_CNT, USB_CTRL_PACKET_SIZE);
  /* Register Callback for Control Endpoint */
  USB_SetCb_Ep(0, USBC_ControlInOut);

#if (USB_MSC)
  /* Init Mass Storage Device */
  MSC_Init();
#endif

#if (USB_CDC)
  /* Init Communication Device Class */
  CDC_Init();
#endif

#if (USB_HID)
  /* Init Human Interface Device */
  HID_Init();
#endif

  /* Init Core */
  USBC_Init(&USBC_Events);
  /* Connect USB */
  USB_Connect(TRUE);
}
