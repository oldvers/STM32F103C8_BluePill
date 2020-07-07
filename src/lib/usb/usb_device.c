#include "types.h"

#include "usb.h"
#include "usb_cfg.h"
#include "usb_defs.h"
#include "usb_core.h"
#include "msc.h"
#include "hid.h"
#include "cdc.h"

#include "debug.h"

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
  //
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
  //
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
  CDC_SOF();
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
    //
  }
  else
  {
    //
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
#if USB_CDD
      case USB_CDD_IF_NUM0:
        result = CDC_CtrlSetupReq(pSetup, pData, pSize);
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
#if USB_CDD
      case USB_CDD_IF_NUM0:
        result = CDC_CtrlOutReq(pSetup, pData, pSize);
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

  LOG("--- MSC --------------------------------------\r\n");
  LOG("Interface Count = %d\r\n",     USB_MSC_IF_CNT);
  LOG("    IF Num 0    = %d\r\n",     USB_MSC_IF_NUM);
  LOG("Endpoint Count  = %d\r\n",     USB_MSC_EP_CNT);
  LOG("    EP 0 Blk I  = 0x%02X\r\n", USB_MSC_EP_BLK_I);
  LOG("    EP 1 Blk O  = 0x%02X\r\n", USB_MSC_EP_BLK_O);

  LOG("--- CDC 1 ------------------------------------\r\n");
  LOG("Interface Count = %d\r\n",     USB_CDC_IF_CNT);
  LOG("    IF Num 0    = %d\r\n",     USB_CDC_IF_NUM0);
  LOG("    IF Num 1    = %d\r\n",     USB_CDC_IF_NUM1);
  LOG("Endpoint Count  = %d\r\n",     USB_CDC_EP_CNT);
  LOG("    EP 0 Blk O  = 0x%02X\r\n", USB_CDC_EP_BLK_O);
  LOG("    EP 1 Blk I  = 0x%02X\r\n", USB_CDC_EP_BLK_I);
  LOG("    EP 2 Irq I  = 0x%02X\r\n", USB_CDC_EP_IRQ_I);
  
  LOG("--- HID --------------------------------------\r\n");
  LOG("Interface Count = %d\r\n",     USB_HID_IF_CNT);
  LOG("    IF Num 0    = %d\r\n",     USB_HID_IF_NUM);
  LOG("Endpoint Count  = %d\r\n",     USB_HID_EP_CNT);
  LOG("    EP 0 Irq I  = 0x%02X\r\n", USB_HID_EP_IRQ_I);
  
  LOG("--- CDC 2 ------------------------------------\r\n");
  LOG("Interface Count = %d\r\n",     USB_CDD_IF_CNT);
  LOG("    IF Num 0    = %d\r\n",     USB_CDD_IF_NUM0);
  LOG("    IF Num 1    = %d\r\n",     USB_CDD_IF_NUM1);
  LOG("Endpoint Count  = %d\r\n",     USB_CDD_EP_CNT);
  LOG("    EP 0 Blk O  = 0x%02X\r\n", USB_CDD_EP_BLK_O);
  LOG("    EP 1 Blk I  = 0x%02X\r\n", USB_CDD_EP_BLK_I);
  LOG("    EP 2 Irq I  = 0x%02X\r\n", USB_CDD_EP_IRQ_I);
  
  LOG("--- Total ------------------------------------\r\n");
  LOG("Interface Count = %d\r\n",     USB_IF_CNT);
  LOG("Endpoint Count  = %d\r\n",     USB_EP_CNT);
  LOG("    EP 0 Ctl I  = 0x%02X\r\n", EP0_I);
  LOG("    EP 1 Ctl O  = 0x%02X\r\n", EP0_O);

  /* Init Hardware */
  USB_Init(USB_EP_CNT, USB_CTRL_PACKET_SIZE);
  /* Register Callback for Control Endpoint */
  USB_SetCb_Ep(EP0_O, USBC_ControlInOut);
  USB_SetCb_Ep(EP0_I, USBC_ControlInOut);

#if (USB_MSC)
  /* Init Mass Storage Device */
  MSC_Init();
#endif

#if (0 < (USB_CDC + USB_CDD))
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
  USB_Connect(FW_TRUE);
}

//-----------------------------------------------------------------------------
/** @brief De-Initializes USB Device
 *  @param None
 *  @return None
 *  @note De-Initializes USB peripheral
 */
void USBD_DeInit(void)
{
  USB_DeInit();

#if USB_RESET_EVENT
  USB_SetCb_Reset(NULL);
#endif
#if USB_SUSPEND_EVENT
  USB_SetCb_Suspend(NULL);
#endif
#if USB_WAKEUP_EVENT
  USB_SetCb_WakeUp(NULL);
#endif
#if USB_SOF_EVENT
  USB_SetCb_SOF(NULL);
#endif
#if USB_ERROR_EVENT
  USB_SetCb_Error(NULL);
#endif
}
