#include "types.h"

#include "usb.h"
#include "usb_config.h"
#include "usb_definitions.h"
#include "usb_descriptor.h"
#include "usb_device.h"

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
/** @brief USB Device Start of Frame Event Callback
 *  @param None
 *  @return None
 *  @note Called automatically on USB Start of Frame Event
 */
#if USB_SOF_EVENT
static void usbd_CbSOF(void)
{
  U8 i = 0;

  for (i = 0; i < USBD_GetItrfacesCount(); i++)
  {
    if (NULL != USBD_IfCbDescriptor[i].CbSOF)
    {
      USBD_IfCbDescriptor[i].CbSOF();
    }
  }
}
#endif

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
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
  U8             i      = 0;

  if (REQUEST_TO_INTERFACE == pSetup->bmRequestType.BM.Recipient)
  {
    i = pSetup->wIndex.WB.L;
    if ( (i < USBD_GetItrfacesCount()) &&
         (NULL != USBD_IfCbDescriptor[i].CbCtrlSetup) )
    {
      result = USBD_IfCbDescriptor[i].CbCtrlSetup(pSetup, pData, pSize);
    }
  }
  else if (REQUEST_TO_ENDPOINT == pSetup->bmRequestType.BM.Recipient)
  {
    //
  }

  return result;
}

/* -------------------------------------------------------------------------- */
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
  U8             i      = 0;

  if (REQUEST_TO_INTERFACE == pSetup->bmRequestType.BM.Recipient)
  {
    i = pSetup->wIndex.WB.L;
    if ( (i < USBD_GetItrfacesCount()) &&
         (NULL != USBD_IfCbDescriptor[i].CbCtrlOut) )
    {
      result = USBD_IfCbDescriptor[i].CbCtrlOut(pSetup, pData, pSize);
    }
  }
  else if (REQUEST_TO_ENDPOINT == pSetup->bmRequestType.BM.Recipient)
  {
    //
  }

  return result;
}

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
/** @brief Initializes USB Device
 *  @param None
 *  @return None
 *  @note Initializes USB peripheral, USB core, sets event callbacks
 */
void USBD_Init(void)
{
  U8 i = 0;

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
  USB_Init(USB_CTRL_PACKET_SIZE);

  /* Register Callback for Control Endpoint */
  USB_SetCb_Ep(EP0_O, USBC_ControlInOut);
  USB_SetCb_Ep(EP0_I, USBC_ControlInOut);

  /* Class Specific Init */
  for (i = 0; i < USBD_GetItrfacesCount(); i++)
  {
    /* Register Callback for Endpoints */
    if (NULL != USBD_IfCbDescriptor[i].CbEndPointI)
    {
      USB_SetCb_Ep
      (
        USBD_IfCbDescriptor[i].EndPointI,
        USBD_IfCbDescriptor[i].CbEndPointI
      );
    }
    if (NULL != USBD_IfCbDescriptor[i].CbEndPointO)
    {
      USB_SetCb_Ep
      (
        USBD_IfCbDescriptor[i].EndPointO,
        USBD_IfCbDescriptor[i].CbEndPointO
      );
    }

    /* Call Specific Initialization */
    if (NULL != USBD_IfCbDescriptor[i].CbInit)
    {
      USBD_IfCbDescriptor[i].CbInit();
    }
  }

  /* Init Core */
  USBC_Init(&USBC_Events);
  /* Connect USB */
  USB_Connect(FW_TRUE);
}

/* -------------------------------------------------------------------------- */
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

/* --- Wrappers for the low level layer to avoid cross-layer includes ------- */

U8 USBD_EndPointRd(U8 aNumber, U8 *pData, U8 aSize)
{
  return (U8)USB_EpRead(aNumber, pData, aSize);
}

/* -------------------------------------------------------------------------- */

U8 USBD_EndPointWr(U8 aNumber, U8 *pData, U8 aSize)
{
  return (U8)USB_EpWrite(aNumber, pData, aSize);
}

/* -------------------------------------------------------------------------- */

U8 USBD_EndPointRdWsCb(U8 aNumber, USBD_CbByte pPutByteCb, U8 aSize)
{
  return (U8)USB_EpReadWsCb(aNumber, pPutByteCb, aSize);
}

/* -------------------------------------------------------------------------- */

U8 USBD_EndPointWrWsCb(U8 aNumber, USBD_CbByte pGetByteCb, U8 aSize)
{
  return (U8)USB_EpWriteWsCb(aNumber, pGetByteCb, aSize);
}
