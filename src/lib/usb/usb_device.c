#include "types.h"

#include "usb.h"
#include "usb_cfg.h"
#include "usb_defs.h"
#include "usb_core.h"
#include "msc.h"

//-----------------------------------------------------------------------------
/** @brief USB Device Reset Event Callback
 *  @param None
 *  @return None
 *  @note Called automatically on USB Reset Event
 */
#if USB_RESET_EVENT
static void usb_CbReset(void)
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
static void usb_CbSuspend(void)
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
static void usb_CbResume(void)
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
static void usb_CbWakeUp(void)
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
static void usb_CbSOF(void)
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
static void usb_CbError(U32 aError)
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
void USB_CbConfigure(U8 aConfig)
{
#if USB_CONFIGURE_EVENT
  /* Check if USB is configured */
  if (aConfig)
  {
    /* Turn On Cfg LED */
    //GPIOB->ODR |=  LED_CFG;
  }
  else
  {
    /* Turn Off Cfg LED */
    //GPIOB->ODR &= ~LED_CFG;
  }
#endif
}

//-----------------------------------------------------------------------------
/** @brief USB Core Set Interface Event Callback
 *  @param None
 *  @return None
 *  @note Called automatically on USB Set Interface Request
 */
void USB_CbInterface(void)
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
void USB_CbFeature(void)
{
#if USB_FEATURE_EVENT
  //
#endif
}

//-----------------------------------------------------------------------------
/** @brief USB Core Event Callbacks Table
 *  @param None
 *  @return None
 *  @note Called automatically on USB Set/Clear Feature Request
 */
const USB_CORE_EVENTS USBC_Events =
{
  USB_CbFeature,
  USB_CbConfigure,
  USB_CbInterface,
  MSC_CtrlSetupReq,
  NULL,
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
  USB_SetCb_Reset(usb_CbReset);
#endif
#if USB_SUSPEND_EVENT
  USB_SetCb_Suspend(usb_CbSuspend);
#endif
#if USB_WAKEUP_EVENT
  USB_SetCb_WakeUp(usb_CbWakeUp);
#endif
#if USB_SOF_EVENT
  USB_SetCb_SOF(usb_CbSOF);
#endif
#if USB_ERROR_EVENT
  USB_SetCb_Error(usb_CbError);
#endif
  /* Init Hardware */
  USB_Init(USB_EP_CNT, USB_CTRL_PACKET_SIZE);
  /* Register Callback for Control Endpoint */
  USB_SetCb_Ep(0, USBC_ControlInOut);

#if USB_MSC
  /* Init Mass Storage Device */
  MSC_Init();
  USB_SetCb_Ep(USB_MSC_EP_BULK_IN,  MSC_BulkIn);
  USB_SetCb_Ep(USB_MSC_EP_BULK_OUT, MSC_BulkOut);
#endif

  /* Init Core */
  USBC_Init(&USBC_Events);
  /* Connect USB */
  USB_Connect(TRUE);
}
