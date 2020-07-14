#include "types.h"

#include "usb.h"
#include "usb_defs.h"
#include "usb_cfg.h"
#include "usb_descr.h"
#include "usb_core.h"
#include "debug.h"

/* USB Endpoint Data Structure */
typedef struct _USB_CTRL_DATA
{
  U8 *pData;
  U16 Count;
} USB_CTRL_DATA;

/* Global Variables */
static U16                    gDeviceStatus;
static U8                     gDeviceAddress;
static U8                     gConfiguration;
static U32                    gEndPointMask;
static U32                    gEndPointHalt;
static U8                     gNumInterfaces;
static U8                     gAltSetting[USB_IF_CNT];
static U8                     gCBuffer[USB_CTRL_PACKET_SIZE];
static USB_CTRL_DATA          gCData;
static USB_SETUP_PACKET       gCSetupPkt;
static const USB_CORE_EVENTS *gCEvents = NULL;

//-----------------------------------------------------------------------------
/** @brief Initializes USB Core
 *  @param pEvents - Pointer to the core event callbacks table
 *  @return None
 */
void USBC_Init(const USB_CORE_EVENTS *pEvents)
{
  gCEvents = pEvents;
}

//-----------------------------------------------------------------------------
/** @brief Resets USB Core
 *  @param None
 *  @return None
 */
void USBC_Reset(void)
{
  gDeviceStatus  = USB_POWER;
  gDeviceAddress = 0;
  gConfiguration = 0;
  gEndPointMask  = ((1 << USB_EP_QUANTITY) | 1);
  gEndPointHalt  = 0x00000000;
}

//-----------------------------------------------------------------------------
/** @brief USB Request - Setup Stage
 *  @param None (global gCSetupPkt)
 *  @return None
 */
void usbc_SetupStage(void)
{
  USB_EpRead(EP0_O, (U8 *)&gCSetupPkt, USB_CTRL_PACKET_SIZE);
}

//-----------------------------------------------------------------------------
/** @brief USB Request - Data In Stage
 *  @param None (global gCData)
 *  @return None
 */
void usbc_DataInStage(void)
{
  U32 cnt;

  if (gCData.Count > USB_CTRL_PACKET_SIZE)
  {
    cnt = USB_CTRL_PACKET_SIZE;
  }
  else
  {
    cnt = gCData.Count;
  }
  cnt = USB_EpWrite(EP0_I, gCData.pData, cnt);
  gCData.pData += cnt;
  gCData.Count -= cnt;
}

//-----------------------------------------------------------------------------
/** @brief USB Request - Data Out Stage
 *  @param None (global gCData)
 *  @return None
 */
void usbc_DataOutStage(void)
{
  U32 cnt;

  cnt = USB_EpRead(EP0_O, gCData.pData, USB_CTRL_PACKET_SIZE);
  gCData.pData += cnt;
  gCData.Count -= cnt;
}

//-----------------------------------------------------------------------------
/** @brief USB Request - Status In Stage
 *  @param None
 *  @return None
 */
void usbc_StatusInStage(void)
{
  USB_EpWrite(EP0_I, NULL, 0);
}

//-----------------------------------------------------------------------------
/** @brief USB Request - Status Out Stage
 *  @param None
 *  @return None
 */
void usbc_StatusOutStage(void)
{
  USB_EpRead(EP0_O, gCBuffer, USB_CTRL_PACKET_SIZE);
}

//-----------------------------------------------------------------------------
/** @brief Get Status USB Request
 *  @param None (global gCSetupPkt)
 *  @return FW_TRUE - Success, FW_FALSE - Error
 */
FW_BOOLEAN usbc_CtrlSetupReqStdGetStatus(void)
{
  U32 n, m;
  FW_BOOLEAN result = FW_FALSE;

  switch (gCSetupPkt.bmRequestType.BM.Recipient)
  {
    case REQUEST_TO_DEVICE:
      gCData.pData = (U8 *)&gDeviceStatus;
      usbc_DataInStage();
      result = FW_TRUE;
      break;
    case REQUEST_TO_INTERFACE:
      if ((gConfiguration != 0) &&
          (gCSetupPkt.wIndex.WB.L < gNumInterfaces))
      {
        *((__packed U16 *)gCBuffer) = 0;
        gCData.pData = gCBuffer;
        usbc_DataInStage();
        result = FW_TRUE;
      }
      break;
    case REQUEST_TO_ENDPOINT:
      n = gCSetupPkt.wIndex.WB.L & (USB_EP_DIR_MASK | USB_EP_NUM_MASK);
      m = (n & USB_EP_DIR_MASK) ?
            ((1 << USB_EP_QUANTITY) << (n & USB_EP_NUM_MASK)) :
             (1 << n);
      if (((gConfiguration != 0) ||
          ((n & USB_EP_NUM_MASK) == 0)) && (gEndPointMask & m))
      {
        *((__packed U16 *)gCBuffer) = (gEndPointHalt & m) ? 1 : 0;
        gCData.pData = gCBuffer;
        usbc_DataInStage();
        result = FW_TRUE;
      }
      break;
    default:
      break;
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Set/Clear Feature USB Request
 *  @param aSetClear - 0 = Clear, 1 = Set
 *  @return FW_TRUE - Success, FW_FALSE - Error
 */
FW_BOOLEAN usbc_CtrlSetupReqStdSetClrFeature(U32 aSetClear)
{
  U32 n, m;
  FW_BOOLEAN result = FW_FALSE;

  switch (gCSetupPkt.bmRequestType.BM.Recipient)
  {
    case REQUEST_TO_DEVICE:
      if (gCSetupPkt.wValue.W == USB_FEATURE_REMOTE_WAKEUP)
      {
        if (aSetClear)
        {
          USB_WakeUpConfigure(FW_TRUE);
          gDeviceStatus |=  USB_GETSTATUS_REMOTE_WAKEUP;
        }
        else
        {
          USB_WakeUpConfigure(FW_FALSE);
          gDeviceStatus &= ~USB_GETSTATUS_REMOTE_WAKEUP;
        }
        result = FW_TRUE;
      }
      break;
    case REQUEST_TO_INTERFACE:
      break;
    case REQUEST_TO_ENDPOINT:
      n = gCSetupPkt.wIndex.WB.L & (USB_EP_DIR_MASK | USB_EP_NUM_MASK);
      m = (n & USB_EP_DIR_MASK) ?
            ((1 << USB_EP_QUANTITY) << (n & USB_EP_NUM_MASK)) :
             (1 << n);
      if ((gConfiguration != 0) &&
          ((n & USB_EP_NUM_MASK) != 0) && (gEndPointMask & m))
      {
        if (gCSetupPkt.wValue.W == USB_FEATURE_ENDPOINT_STALL)
        {
          if (aSetClear)
          {
            USB_EpSetStall(n);
            gEndPointHalt |=  m;
          }
          else
          {
            USB_EpClrStall(n);
            gEndPointHalt &= ~m;
          }
          result = FW_TRUE;
        }
      }
      break;
    default:
      break;
  }

  if (FW_TRUE == result)
  {
    usbc_StatusInStage();
    if (NULL != gCEvents->CbFeature) gCEvents->CbFeature();
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Get Descriptor USB Request
 *  @param None (global gCSetupPkt)
 *  @return FW_TRUE - Success, FW_FALSE - Error
 */
FW_BOOLEAN usbc_CtrlSetupReqStdGetDescriptor(void)
{
  U8 *pD;
  U32 len, n;
  FW_BOOLEAN result = FW_FALSE;

  switch (gCSetupPkt.bmRequestType.BM.Recipient)
  {
    case REQUEST_TO_DEVICE:
      switch (gCSetupPkt.wValue.WB.H)
      {
        case USB_DEVICE_DESCRIPTOR_TYPE:
          gCData.pData = USB_GetDeviceDescriptor();
          len = USB_DEVICE_DESC_SIZE;
          result = FW_TRUE;
          break;
        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
          pD = USB_GetConfigDescriptor();
          for (n = 0; n != gCSetupPkt.wValue.WB.L; n++)
          {
            if (((USB_CONFIGURATION_DESCRIPTOR *)pD)->bLength != 0)
            {
              pD += ((USB_CONFIGURATION_DESCRIPTOR *)pD)->wTotalLength;
            }
          }
          if (((USB_CONFIGURATION_DESCRIPTOR *)pD)->bLength != 0)
          {
            gCData.pData = pD;
            len = ((USB_CONFIGURATION_DESCRIPTOR *)pD)->wTotalLength;
            result = FW_TRUE;
          }
          break;
        case USB_STRING_DESCRIPTOR_TYPE:
          gCData.pData = USB_GetStringDescriptor(gCSetupPkt.wValue.WB.L);
          len = ((USB_STRING_DESCRIPTOR *)gCData.pData)->bLength;
          result = FW_TRUE;
          break;
        default:
          break;
      }
      break;
    case REQUEST_TO_INTERFACE:
      result = USB_GetItrfaceDescriptor(&gCSetupPkt,
                   &gCData.pData, (U16 *)&len);
    default:
      break;
  }

  if (FW_TRUE == result)
  {
    if (gCData.Count > len)
    {
      gCData.Count = len;
    }
    usbc_DataInStage();
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Configures Endpoint according to Attributes from Descriptor
 *  @param pD - Pointer to Endpoint Descriptor
 *  @return None
 */
void usbc_EpConfig(USB_ENDPOINT_DESCRIPTOR * pD)
{
  USB_EP_TYPE t;

  switch (pD->bmAttributes & USB_ENDPOINT_TYPE_MASK)
  {
    case USB_ENDPOINT_TYPE_CONTROL:
      t = USB_EP_TYPE_CONTROL;
      break;
    case USB_ENDPOINT_TYPE_ISOCHRONOUS:
      t = USB_EP_TYPE_ISOCHRONOUS;
      break;
    case USB_ENDPOINT_TYPE_BULK:
      t = USB_EP_TYPE_BULK;
      break;
    case USB_ENDPOINT_TYPE_INTERRUPT:
      t = USB_EP_TYPE_INTERRUPT;
      break;
  }

  USB_EpConfigure(pD->bEndpointAddress, pD->wMaxPacketSize, t);
}

//-----------------------------------------------------------------------------
/** @brief Set Configuration USB Request
 *  @param None (global gCSetupPkt)
 *  @return FW_TRUE - Success, FW_FALSE - Error
 */
FW_BOOLEAN usbc_CtrlSetupReqStdSetConfiguration(void)
{
  USB_COMMON_DESCRIPTOR *pD;
  U32                    alt, n, m;
  FW_BOOLEAN             result = FW_FALSE;

  if (REQUEST_TO_DEVICE == gCSetupPkt.bmRequestType.BM.Recipient)
  {
    if (gCSetupPkt.wValue.WB.L)
    {
      pD = (USB_COMMON_DESCRIPTOR *)USB_GetConfigDescriptor();
      USB_PreapareReConfig();
      while (pD->bLength)
      {
        switch (pD->bDescriptorType)
        {
          case USB_CONFIGURATION_DESCRIPTOR_TYPE:
            if (((USB_CONFIGURATION_DESCRIPTOR *)pD)->bConfigurationValue ==
                gCSetupPkt.wValue.WB.L)
            {
              gConfiguration = gCSetupPkt.wValue.WB.L;
              gNumInterfaces =
                ((USB_CONFIGURATION_DESCRIPTOR *)pD)->bNumInterfaces;
              for (n = 0; n < USB_IF_CNT; n++)
              {
                gAltSetting[n] = 0;
              }
              for (n = 1; n < USB_EP_QUANTITY; n++)
              {
                if (gEndPointMask & (1 << n))
                {
                  USB_EpDisable(n);
                }
                if (gEndPointMask & ((1 << USB_EP_QUANTITY) << n))
                {
                  USB_EpDisable(n | USB_EP_DIR_MASK);
                }
              }
              gEndPointMask = ((1 << USB_EP_QUANTITY) | 1);
              gEndPointHalt = 0x00000000;
              USB_Configure(FW_TRUE);
              if (((USB_CONFIGURATION_DESCRIPTOR *)pD)->bmAttributes &
                    USB_CONFIG_SELF_POWERED)
              {
                gDeviceStatus |=  USB_GETSTATUS_SELF_POWERED;
              }
              else
              {
                gDeviceStatus &= ~USB_GETSTATUS_SELF_POWERED;
              }
            }
            else
            {
              pD = (USB_COMMON_DESCRIPTOR *)((U8 *)pD +
                ((USB_CONFIGURATION_DESCRIPTOR *)pD)->wTotalLength);
              continue;
            }
            break;
          case USB_INTERFACE_DESCRIPTOR_TYPE:
            alt = ((USB_INTERFACE_DESCRIPTOR *)pD)->bAlternateSetting;
            break;
          case USB_ENDPOINT_DESCRIPTOR_TYPE:
            if (alt == 0)
            {
              n = ((USB_ENDPOINT_DESCRIPTOR *)pD)->bEndpointAddress &
                   (USB_EP_DIR_MASK | USB_EP_NUM_MASK);
              m = (n & USB_EP_DIR_MASK) ?
                    ((1 << USB_EP_QUANTITY) << (n & USB_EP_NUM_MASK)) :
                     (1 << n);
              gEndPointMask |= m;

              usbc_EpConfig((USB_ENDPOINT_DESCRIPTOR *)pD);
              USB_EpEnable(n);
              USB_EpReset(n);
            }
            break;
        }
        pD = (USB_COMMON_DESCRIPTOR *)((U8 *)pD + pD->bLength);
      }
    }
    else
    {
      gConfiguration = 0;
      for (n = 1; n < USB_EP_QUANTITY; n++)
      {
        if (gEndPointMask & (1 << n))
        {
          USB_EpDisable(n);
        }
        if (gEndPointMask & ((1 << USB_EP_QUANTITY) << n))
        {
          USB_EpDisable(n | USB_EP_DIR_MASK);
        }
      }
      gEndPointMask  = ((1 << USB_EP_QUANTITY) | 1);
      gEndPointHalt  = 0x00000000;
      USB_Configure(FW_FALSE);
    }

    if (gConfiguration == gCSetupPkt.wValue.WB.L)
    {
      usbc_StatusInStage();
      if (NULL != gCEvents->CbConfigure)
      {
        gCEvents->CbConfigure(gConfiguration);
      }
      result = FW_TRUE;
    }
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Set Interface USB Request
 *  @param None (global gCSetupPkt)
 *  @return FW_TRUE - Success, FW_FALSE - Error
 */
FW_BOOLEAN usbc_CtrlSetupReqStdSetInterface(void)
{
  USB_COMMON_DESCRIPTOR *pD;
  U32                    ifn, alt, old, msk, n, m;
  FW_BOOLEAN             result = FW_FALSE;

  if (gConfiguration == 0) return result;

  if (REQUEST_TO_INTERFACE == gCSetupPkt.bmRequestType.BM.Recipient)
  {
    pD  = (USB_COMMON_DESCRIPTOR *)USB_GetConfigDescriptor();
    USB_PreapareReConfig();
    while (pD->bLength)
    {
      switch (pD->bDescriptorType)
      {
        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
          if (((USB_CONFIGURATION_DESCRIPTOR *)pD)->bConfigurationValue !=
               gConfiguration)
          {
            pD = (USB_COMMON_DESCRIPTOR *)((U8 *)pD +
              ((USB_CONFIGURATION_DESCRIPTOR *)pD)->wTotalLength);
            continue;
          }
          break;
        case USB_INTERFACE_DESCRIPTOR_TYPE:
          ifn = ((USB_INTERFACE_DESCRIPTOR *)pD)->bInterfaceNumber;
          alt = ((USB_INTERFACE_DESCRIPTOR *)pD)->bAlternateSetting;
          msk = 0;
          if ((ifn == gCSetupPkt.wIndex.WB.L) &&
              (alt == gCSetupPkt.wValue.WB.L))
          {
            result = FW_TRUE;
            old = gAltSetting[ifn];
            gAltSetting[ifn] = (U8)alt;
          }
          break;
        case USB_ENDPOINT_DESCRIPTOR_TYPE:
          if (ifn == gCSetupPkt.wIndex.WB.L)
          {
            n = ((USB_ENDPOINT_DESCRIPTOR *)pD)->bEndpointAddress &
                 (USB_EP_DIR_MASK | USB_EP_NUM_MASK);
            m = (n & USB_EP_DIR_MASK) ?
                  ((1 << USB_EP_QUANTITY) << (n & USB_EP_NUM_MASK)) :
                   (1 << n);
            if (alt == gCSetupPkt.wValue.WB.L)
            {
              gEndPointMask |=  m;
              gEndPointHalt &= ~m;
              usbc_EpConfig((USB_ENDPOINT_DESCRIPTOR *)pD);
              USB_EpEnable(n);
              USB_EpReset(n);
              msk |= m;
            }
            else if ((alt == old) && ((msk & m) == 0))
            {
              gEndPointMask &= ~m;
              gEndPointHalt &= ~m;
              USB_EpDisable(n);
            }
          }
          break;
      }
      pD = (USB_COMMON_DESCRIPTOR *)((U8 *)pD + pD->bLength);
    }
  }

  if (FW_TRUE == result)
  {
    usbc_StatusInStage();
    if (NULL != gCEvents->CbInterface) gCEvents->CbInterface();
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Set Address USB Request
 *  @param None (global gCSetupPkt)
 *  @return FW_TRUE - Success, FW_FALSE - Error
 */
FW_BOOLEAN usbc_CtrlSetupReqStdSetAddress(void)
{
  FW_BOOLEAN result = FW_FALSE;

  if (REQUEST_TO_DEVICE == gCSetupPkt.bmRequestType.BM.Recipient)
  {
    gDeviceAddress = USB_EP_DIR_MASK | gCSetupPkt.wValue.WB.L;
    usbc_StatusInStage();
    result = FW_TRUE;
  }
  return result;
}

//-----------------------------------------------------------------------------
/** @brief Get Configuration USB Request
 *  @param None (global gCSetupPkt)
 *  @return FW_TRUE - Success, FW_FALSE - Error
 */
FW_BOOLEAN usbc_CtrlSetupReqStdGetConfiguration(void)
{
  FW_BOOLEAN result = FW_FALSE;

  if (REQUEST_TO_DEVICE == gCSetupPkt.bmRequestType.BM.Recipient)
  {
    gCData.pData = &gConfiguration;
    usbc_DataInStage();
    result = FW_TRUE;
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Get Interface USB Request
 *  @param None (global gCSetupPkt)
 *  @return FW_TRUE - Success, FW_FALSE - Error
 */
FW_BOOLEAN usbc_CtrlSetupReqStdGetInterface(void)
{
  FW_BOOLEAN result = FW_FALSE;

  if (REQUEST_TO_INTERFACE == gCSetupPkt.bmRequestType.BM.Recipient)
  {
    if ((gConfiguration != 0) &&
        (gCSetupPkt.wIndex.WB.L < gNumInterfaces))
    {
      gCData.pData = gAltSetting + gCSetupPkt.wIndex.WB.L;
      usbc_DataInStage();
      result = FW_TRUE;
    }
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Checks the next In Stage
 *  @param None (global gCSetupPkt)
 *  @return FW_TRUE - Success, FW_FALSE - Error
 */
FW_BOOLEAN usbc_CheckInStage(USB_CTRL_STAGE stage)
{
  switch (stage)
  {
    case USB_CTRL_STAGE_WAIT:
      return (FW_TRUE);
    case USB_CTRL_STAGE_DATA:
      usbc_DataInStage();
      return (FW_TRUE);
    case USB_CTRL_STAGE_STATUS:
      usbc_StatusInStage();
      return (FW_TRUE);
    case USB_CTRL_STAGE_ERROR:
    default:
      return (FW_FALSE);
  }
}

//-----------------------------------------------------------------------------
/** @brief Class USB Setup Request
 *  @param None (global gCSetupPkt)
 *  @return FW_TRUE - Success, FW_FALSE - Error
 */
FW_BOOLEAN usbc_CtrlSetupReqClass(void)
{
  USB_CTRL_STAGE stage = USB_CTRL_STAGE_ERROR;

  gCData.pData = gCBuffer;
  if (NULL != gCEvents->CtrlSetupReq)
  {
    stage = gCEvents->CtrlSetupReq(&gCSetupPkt, &gCData.pData, &gCData.Count);
  }

  return usbc_CheckInStage(stage);
}

//-----------------------------------------------------------------------------
/** @brief Class USB Out Request
 *  @param None (global gCSetupPkt)
 *  @return FW_TRUE - Success, FW_FALSE - Error
 *  @note Called when all the OUT packets have been already collected
 */
FW_BOOLEAN usbc_CtrlOutReqClass(void)
{
  USB_CTRL_STAGE stage = USB_CTRL_STAGE_ERROR;

  gCData.pData = gCBuffer;
  if (NULL != gCEvents->CtrlOutReq)
  {
    stage = gCEvents->CtrlOutReq(&gCSetupPkt, &gCData.pData, &gCData.Count);
  }

  return usbc_CheckInStage(stage);
}

//-----------------------------------------------------------------------------
/** @brief Processes USB Control Endpoint Events
 *  @param aEvent - Event
 *  @return None
 */
void USBC_ControlInOut(U32 aEvent)
{
  FW_BOOLEAN result = FW_FALSE;

  switch (aEvent)
  {
    case USB_EVNT_EP_SETUP:
      usbc_SetupStage();

      gCData.Count = gCSetupPkt.wLength;

      result = FW_FALSE;
      switch (gCSetupPkt.bmRequestType.BM.Type)
      {
        case REQUEST_STANDARD:
          switch (gCSetupPkt.bRequest)
          {
            case USB_REQUEST_GET_STATUS:
              result = usbc_CtrlSetupReqStdGetStatus();
              break;

            case USB_REQUEST_CLEAR_FEATURE:
              result = usbc_CtrlSetupReqStdSetClrFeature(FW_FALSE);
              break;

            case USB_REQUEST_SET_FEATURE:
              result = usbc_CtrlSetupReqStdSetClrFeature(FW_TRUE);
              break;

            case USB_REQUEST_SET_ADDRESS:
              result = usbc_CtrlSetupReqStdSetAddress();
              break;

            case USB_REQUEST_GET_DESCRIPTOR:
              result = usbc_CtrlSetupReqStdGetDescriptor();
              break;

            case USB_REQUEST_SET_DESCRIPTOR:
              break;

            case USB_REQUEST_GET_CONFIGURATION:
              result = usbc_CtrlSetupReqStdGetConfiguration();
              break;

            case USB_REQUEST_SET_CONFIGURATION:
              result = usbc_CtrlSetupReqStdSetConfiguration();
              break;

            case USB_REQUEST_GET_INTERFACE:
              result = usbc_CtrlSetupReqStdGetInterface();
              break;

            case USB_REQUEST_SET_INTERFACE:
              result = usbc_CtrlSetupReqStdSetInterface();
              break;

            default:
              break;
          }
          break;

        case REQUEST_CLASS:
        case REQUEST_VENDOR:
          result = usbc_CtrlSetupReqClass();
          break;

        default:
          break;

      }
      if (FW_TRUE != result)
      {
        USB_EpSetStall(EP0_I);
        gCData.Count = 0;
      }
      break;

    case USB_EVNT_EP_OUT:
      if (gCSetupPkt.bmRequestType.BM.Dir == REQUEST_HOST_TO_DEVICE)
      {
        if (gCData.Count)
        {
          usbc_DataOutStage();
          if (gCData.Count == 0)
          {
            result = FW_FALSE;
            switch (gCSetupPkt.bmRequestType.BM.Type)
            {
              case REQUEST_STANDARD:
                break;

              case REQUEST_CLASS:
              case REQUEST_VENDOR:
                result = usbc_CtrlOutReqClass();
                break;
                
              default:
                break;
            }
            if (FW_TRUE != result)
            {
              USB_EpSetStall(EP0_O);
              gCData.Count = 0;
            }
          }
        }
      }
      else
      {
        usbc_StatusOutStage();
      }
      break;

    case USB_EVNT_EP_IN:
      if (gCSetupPkt.bmRequestType.BM.Dir == REQUEST_DEVICE_TO_HOST)
      {
        usbc_DataInStage();
      }
      else
      {
        if (gDeviceAddress & USB_EP_DIR_MASK)
        {
          gDeviceAddress &= (U8)(~USB_EP_DIR_MASK);
          USB_SetAddress(gDeviceAddress);
        }
      }
      break;

    case USB_EVNT_EP_IN_STALL:
      USB_EpClrStall(EP0_I);
      break;

    case USB_EVNT_EP_OUT_STALL:
      USB_EpClrStall(EP0_O);
      break;
  }
}
