#include "types.h"

#include "usb.h"
#include "usb_defs.h"
#include "usb_cfg.h"
#include "usb_descr.h"
#include "usb_core.h"
#include "usb_device.h"
#include "debug.h"

//#if (USB_AUDIO)
//#include "audio.h"
//#include "adcuser.h"
//#endif
//
//#if (USB_HID)
//#include "hid.h"
//#include "hiduser.h"
//#endif
//
//#if (USB_MSC)
//#include "msc.h"
//#include "mscuser.h"
//#endif


//#pragma diag_suppress 111,1441

/* USB Endpoint Data Structure */
typedef struct _USB_EP_DATA
{
  U8 *pData;
  U16 Count;
} USB_EP_DATA;

static U16 gUSB_DeviceStatus;
static U8  gUSB_DeviceAddress;
static U8  gUSB_Configuration;
static U32 gUSB_EndPointMask;
static U32 gUSB_EndPointHalt;
static U8  gUSB_NumInterfaces;
static U8  gUSB_AltSetting[16]; //USB_IF_NUM];

static U8  EP0Buffer[8]; //USB_MAX_PACKET0];
USB_EP_DATA EP0Data;
USB_SETUP_PACKET SetupPacket;

//static USBC_CbGeneric gUSB_CbFeature   = NULL;
//static USBC_CbGeneric gUSB_CbConfigure = NULL;
//static USBC_CbGeneric gUSB_CbInterface = NULL;


/** @brief Resets USB Core
 *  @param None
 *  @return None
 */
void USB_ResetCore(void)
{
  gUSB_DeviceStatus  = USB_POWER;
  gUSB_DeviceAddress = 0;
  gUSB_Configuration = 0;
  gUSB_EndPointMask  = 0x00010001;
  gUSB_EndPointHalt  = 0x00000000;
}

/** @brief USB Request - Setup Stage
 *  @param None (global SetupPacket)
 *  @return None
 */
void USB_SetupStage(void)
{
  USB_EpRead(0x00, (U8 *)&SetupPacket);
}

/** @brief USB Request - Data In Stage
 *  @param None (global EP0Data)
 *  @return None
 */
void USB_DataInStage(void)
{
  U32 cnt;

  if (EP0Data.Count > USB_MAX_PACKET0)
  {
    cnt = USB_MAX_PACKET0;
  }
  else
  {
    cnt = EP0Data.Count;
  }
  cnt = USB_EpWrite(0x80, EP0Data.pData, cnt);
  EP0Data.pData += cnt;
  EP0Data.Count -= cnt;
}

/** @brief USB Request - Data Out Stage
 *  @param None (global EP0Data)
 *  @return None
 */
void USB_DataOutStage(void)
{
  U32 cnt;

  cnt = USB_EpRead(0x00, EP0Data.pData);
  EP0Data.pData += cnt;
  EP0Data.Count -= cnt;
}

/** @brief USB Request - Status In Stage
 *  @param None
 *  @return None
 */
void USB_StatusInStage(void)
{
  USB_EpWrite(0x80, NULL, 0);
}

/** @brief USB Request - Status Out Stage
 *  @param None
 *  @return None
 */
void USB_StatusOutStage(void)
{
  USB_EpRead(0x00, EP0Buffer);
}

/** @brief Get Status USB Request
 *  @param None (global SetupPacket)
 *  @return TRUE - Success, FALSE - Error
 */ //__inline
//U32 USB_GetStatus(void)
U32 usb_CtrlSetupReqStdGetStatus(void)
{
  U32 n, m, result = FALSE;

  switch (SetupPacket.bmRequestType.BM.Recipient)
  {
    case REQUEST_TO_DEVICE:
      EP0Data.pData = (U8 *)&gUSB_DeviceStatus;
      USB_DataInStage();
      result = TRUE;
      break;
    case REQUEST_TO_INTERFACE:
      if ((gUSB_Configuration != 0) &&
          (SetupPacket.wIndex.WB.L < gUSB_NumInterfaces))
      {
        *((__packed U16 *)EP0Buffer) = 0;
        EP0Data.pData = EP0Buffer;
        USB_DataInStage();
        result = TRUE;
      }
//      else
//      {
//        return (FALSE);
//      }
      break;
    case REQUEST_TO_ENDPOINT:
      n = SetupPacket.wIndex.WB.L & 0x8F;
      m = (n & 0x80) ? ((1 << 16) << (n & 0x0F)) : (1 << n);
      if (((gUSB_Configuration != 0) ||
          ((n & 0x0F) == 0)) && (gUSB_EndPointMask & m))
      {
        *((__packed U16 *)EP0Buffer) = (gUSB_EndPointHalt & m) ? 1 : 0;
        EP0Data.pData = EP0Buffer;
        USB_DataInStage();
        result = TRUE;
      }
//      else
//      {
//        return (FALSE);
//      }
      break;
    default:
//      return (FALSE);
      break;
  }
//  return (TRUE);
  return result;
}

/** @brief Set/Clear Feature USB Request
 *  @param aSetClear - 0 = Clear, 1 = Set
 *  @return TRUE - Success, FALSE - Error
 */ //__inline
U32 usb_CtrlSetupReqStdSetClrFeature(U32 aSetClear)
//{
//  if (!USB_SetClrFeature(0)) return FALSE;
//  
//  USB_StatusInStage();
//  if (NULL != gUSB_CbFeature) gUSB_CbFeature();
//  
//  return TRUE;
//}
//U32 usb_SetClrFeature(U32 aSetClear)
{
  U32 n, m, result = FALSE;

  switch (SetupPacket.bmRequestType.BM.Recipient)
  {
    case REQUEST_TO_DEVICE:
      if (SetupPacket.wValue.W == USB_FEATURE_REMOTE_WAKEUP)
      {
        if (aSetClear)
        {
          USB_WakeUpConfigure(TRUE);
          gUSB_DeviceStatus |=  USB_GETSTATUS_REMOTE_WAKEUP;
        }
        else
        {
          USB_WakeUpConfigure(FALSE);
          gUSB_DeviceStatus &= ~USB_GETSTATUS_REMOTE_WAKEUP;
        }
        result = TRUE;
      }
//      else
//      {
//        return (FALSE);
//      }
      break;
    case REQUEST_TO_INTERFACE:
//      return (FALSE);
      break;
    case REQUEST_TO_ENDPOINT:
      n = SetupPacket.wIndex.WB.L & 0x8F;
      m = (n & 0x80) ? ((1 << 16) << (n & 0x0F)) : (1 << n);
      if ((gUSB_Configuration != 0) &&
          ((n & 0x0F) != 0) && (gUSB_EndPointMask & m))
      {
        if (SetupPacket.wValue.W == USB_FEATURE_ENDPOINT_STALL)
        {
          if (aSetClear)
          {
            USB_EpSetStall(n);
            gUSB_EndPointHalt |=  m;
          }
          else
          {
            USB_EpClrStall(n);
            gUSB_EndPointHalt &= ~m;
          }
          result = TRUE;
        }
//        else
//        {
//          return (FALSE);
//        }
      }
//      else
//      {
//        return (FALSE);
//      }
      break;
    default:
//      return (FALSE);
      break;
  }
  
  if (TRUE == result)
  {
    USB_StatusInStage();
    //if (NULL != gUSB_CbFeature) gUSB_CbFeature();
    USB_CbFeature();
  }
//  return (TRUE);
  return result;
}


/** @brief Get Descriptor USB Request
 *  @param None (global SetupPacket)
 *  @return TRUE - Success, FALSE - Error
 */ //__inline
U32 usb_CtrlSetupReqStdGetDescriptor(void)
//U32 USB_GetDescriptor(void)
{
  U8 *pD;
  U32 len, n, result = FALSE;

  switch (SetupPacket.bmRequestType.BM.Recipient)
  {
    case REQUEST_TO_DEVICE:
      switch (SetupPacket.wValue.WB.H)
      {
        case USB_DEVICE_DESCRIPTOR_TYPE:
          EP0Data.pData = USB_GetDeviceDescriptor();
          len = USB_DEVICE_DESC_SIZE;
          result = TRUE;
          break;
        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
          pD = USB_GetConfigDescriptor();
          for (n = 0; n != SetupPacket.wValue.WB.L; n++)
          {
            if (((USB_CONFIGURATION_DESCRIPTOR *)pD)->bLength != 0)
            {
              pD += ((USB_CONFIGURATION_DESCRIPTOR *)pD)->wTotalLength;
            }
          }
          if (((USB_CONFIGURATION_DESCRIPTOR *)pD)->bLength != 0)
          {
//            return (FALSE);
//          }
            EP0Data.pData = pD;
            len = ((USB_CONFIGURATION_DESCRIPTOR *)pD)->wTotalLength;
            result = TRUE;
          }
          break;
        case USB_STRING_DESCRIPTOR_TYPE:
          EP0Data.pData = USB_GetStringDescriptor() + SetupPacket.wValue.WB.L;
          len = ((USB_STRING_DESCRIPTOR *)EP0Data.pData)->bLength;
          result = TRUE;
          break;
        default:
//          return (FALSE);
          break;
      }
      break;
    case REQUEST_TO_INTERFACE:
      result = USB_GetItrfaceDescriptor(SetupPacket.wIndex.WB.L,
                 SetupPacket.wValue.WB.H, EP0Data.pData, &len);
//      switch (SetupPacket.wValue.WB.H)
//      {
//#if USB_HID
//        case HID_HID_DESCRIPTOR_TYPE:
//          if (SetupPacket.wIndex.WB.L == USB_HID_IF_NUM)
//          {
////            return (FALSE);    /* Only Single HID Interface is supported */
////          }
//            EP0Data.pData = (U8 *)USB_ConfigDescriptor + HID_DESC_OFFSET;
//            len = HID_DESC_SIZE;
//            result = TRUE;
//          }
//          break;
//        case HID_REPORT_DESCRIPTOR_TYPE:
//          if (SetupPacket.wIndex.WB.L == USB_HID_IF_NUM)
//          {
////            return (FALSE);    /* Only Single HID Interface is supported */
////          }
//            EP0Data.pData = (BYTE *)HID_ReportDescriptor;
//            len = HID_ReportDescSize;
//            result = TRUE;
//          }
//          break;
//        case HID_PHYSICAL_DESCRIPTOR_TYPE:
////          return (FALSE);      /* HID Physical Descriptor is not supported */
//          break;
//#endif
//        default:
////          return (FALSE);
//          break;
//      }
//      break;
    default:
//      return (FALSE);
      break;
  }

  if (TRUE == result)
  {
    if (EP0Data.Count > len)
    {
      EP0Data.Count = len;
    }
    USB_DataInStage();
  }

//  return (TRUE);
  return result;
}




void usb_EpConfig(USB_ENDPOINT_DESCRIPTOR * pD)
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
      //if (USB_DBL_BUF_EP & (1 << num)) val |= USB_EP_KIND;
      break;
    case USB_ENDPOINT_TYPE_INTERRUPT:
      t = USB_EP_TYPE_INTERRUPT;
      break;
  }
  
  USB_EpConfigure(pD->bEndpointAddress, pD->wMaxPacketSize, t);
}


/** @brief Set Configuration USB Request
 *  @param None (global SetupPacket)
 *  @return TRUE - Success, FALSE - Error
 */ //__inline
U32 usb_CtrlSetupReqStdSetConfiguration(void)
//{
//  U32 result = FALSE;
//  
//  if (REQUEST_TO_DEVICE == SetupPacket.bmRequestType.BM.Recipient)
//  {
//    if (TRUE == USB_SetConfiguration())
//    {
//      USB_StatusInStage();
//      if (NULL != gUSB_CbConfigure) gUSB_CbConfigure();
//      result = TRUE;
//    }
//  }
//  
//  return result;
//}
//U32 USB_SetConfiguration(void)
{
  USB_COMMON_DESCRIPTOR *pD;
  U32                    alt, n, m, result = FALSE;

  if (REQUEST_TO_DEVICE == SetupPacket.bmRequestType.BM.Recipient)
  {
    if (SetupPacket.wValue.WB.L)
    {
      pD = (USB_COMMON_DESCRIPTOR *)USB_GetConfigDescriptor();
      while (pD->bLength)
      {
        switch (pD->bDescriptorType)
        {
          case USB_CONFIGURATION_DESCRIPTOR_TYPE:
            if (((USB_CONFIGURATION_DESCRIPTOR *)pD)->bConfigurationValue ==
                SetupPacket.wValue.WB.L)
            {
              gUSB_Configuration = SetupPacket.wValue.WB.L;
              gUSB_NumInterfaces =
                ((USB_CONFIGURATION_DESCRIPTOR *)pD)->bNumInterfaces;
              for (n = 0; n < USB_IF_NUM; n++)
              {
                gUSB_AltSetting[n] = 0;
              }
              for (n = 1; n < 16; n++)
              {
                if (gUSB_EndPointMask & (1 << n))
                {
                  USB_EpDisable(n);
                }
                if (gUSB_EndPointMask & ((1 << 16) << n))
                {
                  USB_EpDisable(n | 0x80);
                }
              }
              gUSB_EndPointMask = 0x00010001;
              gUSB_EndPointHalt = 0x00000000;
              USB_Configure(TRUE);
              if (((USB_CONFIGURATION_DESCRIPTOR *)pD)->bmAttributes &
                    USB_CONFIG_SELF_POWERED)
              {
                gUSB_DeviceStatus |=  USB_GETSTATUS_SELF_POWERED;
              }
              else
              {
                gUSB_DeviceStatus &= ~USB_GETSTATUS_SELF_POWERED;
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
              n = ((USB_ENDPOINT_DESCRIPTOR *)pD)->bEndpointAddress & 0x8F;
              m = (n & 0x80) ? ((1 << 16) << (n & 0x0F)) : (1 << n);
              gUSB_EndPointMask |= m;
              
              usb_EpConfig((USB_ENDPOINT_DESCRIPTOR *)pD);
              
  //            switch (((USB_ENDPOINT_DESCRIPTOR *)pD)->bmAttributes & USB_ENDPOINT_TYPE_MASK)
  //            {
  //              case USB_ENDPOINT_TYPE_CONTROL:
  //                t = USB_EP_TYPE_CONTROL;
  //                break;
  //              case USB_ENDPOINT_TYPE_ISOCHRONOUS:
  //                t = USB_EP_TYPE_ISOCHRONOUS;
  //                break;
  //              case USB_ENDPOINT_TYPE_BULK:
  //                t = USB_EP_TYPE_BULK;
  //                //if (USB_DBL_BUF_EP & (1 << num)) val |= USB_EP_KIND;
  //                break;
  //              case USB_ENDPOINT_TYPE_INTERRUPT:
  //                t = USB_EP_TYPE_INTERRUPT;
  //                break;
  //            }
  //            USB_EpConfigure
  //            (
  //              ((USB_ENDPOINT_DESCRIPTOR *)pD)->bEndpointAddress,
  //              ((USB_ENDPOINT_DESCRIPTOR *)pD)->wMaxPacketSize,
  //              t
  //            );
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
      gUSB_Configuration = 0;
      for (n = 1; n < 16; n++)
      {
        if (gUSB_EndPointMask & (1 << n))
        {
          USB_EpDisable(n);
        }
        if (gUSB_EndPointMask & ((1 << 16) << n))
        {
          USB_EpDisable(n | 0x80);
        }
      }
      gUSB_EndPointMask  = 0x00010001;
      gUSB_EndPointHalt  = 0x00000000;
      USB_Configure(FALSE);
    }

    if (gUSB_Configuration == SetupPacket.wValue.WB.L)
    {
//      return (TRUE);
      USB_StatusInStage();
      //if (NULL != gUSB_CbConfigure) gUSB_CbConfigure();
      USB_CbConfigure(gUSB_Configuration);
      result = TRUE;
    }
//    else
//    {
//      return (FALSE);
//    }
  }
  
  return result;
}

/** @brief Set Interface USB Request
 *  @param None (global SetupPacket)
 *  @return TRUE - Success, FALSE - Error
 */ //__inline
U32 usb_CtrlSetupReqStdSetInterface(void)
//{
//  U32 result = FALSE;
//  
//  if (REQUEST_TO_INTERFACE == SetupPacket.bmRequestType.BM.Recipient)
//  {
//    if (TRUE == USB_SetInterface())
//    {
//      USB_StatusInStage();
//      if (NULL != gUSB_CbInterface) gUSB_CbInterface();
//      result = TRUE;
//    }
//  }
//  
//  return result;
//}
//U32 USB_SetInterface(void)
{
  USB_COMMON_DESCRIPTOR *pD;
  U32                    ifn, alt, old, msk, n, m;
  U32                    result = FALSE;

  if (gUSB_Configuration == 0) return result;

  if (REQUEST_TO_INTERFACE == SetupPacket.bmRequestType.BM.Recipient)
  {
//    set = FALSE;
    pD  = (USB_COMMON_DESCRIPTOR *)USB_GetConfigDescriptor();
    while (pD->bLength)
    {
      switch (pD->bDescriptorType)
      {
        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
          if (((USB_CONFIGURATION_DESCRIPTOR *)pD)->bConfigurationValue !=
               gUSB_Configuration)
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
          if ((ifn == SetupPacket.wIndex.WB.L) &&
              (alt == SetupPacket.wValue.WB.L))
          {
            result = TRUE;
            old = gUSB_AltSetting[ifn];
            gUSB_AltSetting[ifn] = (U8)alt;
          }
          break;
        case USB_ENDPOINT_DESCRIPTOR_TYPE:
          if (ifn == SetupPacket.wIndex.WB.L)
          {
            n = ((USB_ENDPOINT_DESCRIPTOR *)pD)->bEndpointAddress & 0x8F;
            m = (n & 0x80) ? ((1 << 16) << (n & 0x0F)) : (1 << n);
            if (alt == SetupPacket.wValue.WB.L)
            {
              gUSB_EndPointMask |=  m;
              gUSB_EndPointHalt &= ~m;
              usb_EpConfig((USB_ENDPOINT_DESCRIPTOR *)pD);
              USB_EpEnable(n);
              USB_EpReset(n);
              msk |= m;
            }
            else if ((alt == old) && ((msk & m) == 0))
            {
              gUSB_EndPointMask &= ~m;
              gUSB_EndPointHalt &= ~m;
              USB_EpDisable(n);
            }
          }
          break;
      }
      pD = (USB_COMMON_DESCRIPTOR *)((U8 *)pD + pD->bLength);
    }
  }
  
  if (TRUE == result)
  {
    USB_StatusInStage();
    //if (NULL != gUSB_CbInterface) gUSB_CbInterface();
    USB_CbInterface();
  }
  
  return result;
}



//-----------------------------------------------------------------------------
//U32 usb_CtrlSetupReqStandard(void)
//{
//  return USB_GetStatus();
//}

//-----------------------------------------------------------------------------
//U32 usb_CtrlSetupReqClrFeature(void)
//{
//  if (!USB_SetClrFeature(0)) return FALSE;
//  
//  USB_StatusInStage();
//  if (NULL != gUSB_CbFeature) gUSB_CbFeature();
//  
//  return TRUE;
//}

//-----------------------------------------------------------------------------
//U32 usb_CtrlSetupReqSetFeature(void)
//{
//  if (!USB_SetClrFeature(1)) return FALSE;

//  USB_StatusInStage();
//  if (NULL != gUSB_CbFeature) gUSB_CbFeature();
//  
//  return TRUE;
//}

//-----------------------------------------------------------------------------
U32 usb_CtrlSetupReqStdSetAddress(void)
{
  U32 result = FALSE;
  
  if (REQUEST_TO_DEVICE == SetupPacket.bmRequestType.BM.Recipient)
  {
    gUSB_DeviceAddress = 0x80 | SetupPacket.wValue.WB.L;
    USB_StatusInStage();
    result = TRUE;
  }
  return result;
}

//-----------------------------------------------------------------------------
U32 usb_CtrlSetupReqStdGetConfiguration(void)
{
  U32 result = FALSE;
  
  if (REQUEST_TO_DEVICE == SetupPacket.bmRequestType.BM.Recipient)
  {
    EP0Data.pData = &gUSB_Configuration;
    USB_DataInStage();
    result = TRUE;
  }
  
  return result;
}

//-----------------------------------------------------------------------------
//U32  usb_CtrlSetupReqStdSetConfiguration(void)
//{
//  U32 result = FALSE;
//  
//  if (REQUEST_TO_DEVICE == SetupPacket.bmRequestType.BM.Recipient)
//  {
//    if (TRUE == USB_SetConfiguration())
//    {
//      USB_StatusInStage();
//      if (NULL != gUSB_CbConfigure) gUSB_CbConfigure();
//      result = TRUE;
//    }
//  }
//  
//  return result;
//}

//-----------------------------------------------------------------------------
U32 usb_CtrlSetupReqStdGetInterface(void)
{
  U32 result = FALSE;
  
  if (REQUEST_TO_INTERFACE == SetupPacket.bmRequestType.BM.Recipient)
  {
    if ((gUSB_Configuration != 0) && 
        (SetupPacket.wIndex.WB.L < gUSB_NumInterfaces))
    {
      EP0Data.pData = gUSB_AltSetting + SetupPacket.wIndex.WB.L;
      USB_DataInStage();
      result = TRUE;
    }
  }
  
  return result;
}

//-----------------------------------------------------------------------------
//U32 usb_CtrlSetupReqStdSetInterface(void)
//{
//  U32 result = FALSE;
//  
//  if (REQUEST_TO_INTERFACE == SetupPacket.bmRequestType.BM.Recipient)
//  {
//    if (TRUE == USB_SetInterface())
//    {
//      USB_StatusInStage();
//      if (NULL != gUSB_CbInterface) gUSB_CbInterface();
//      result = TRUE;
//    }
//  }
//  
//  return result;
//}



//-----------------------------------------------------------------------------
U32 usb_CtrlSetupReqClass(void)
{
  U32 result = FALSE;
  
  switch (SetupPacket.bmRequestType.BM.Recipient)
  {
    case REQUEST_TO_INTERFACE:
      result = USB_CtrlSetupReqItrface(SetupPacket.wIndex.WB.L,
                 SetupPacket.wIndex.WB.L, EP0Data.pData, EP0Data.Count);
//#if USB_HID
//      if (SetupPacket.wIndex.WB.L == USB_HID_IF_NUM)
//      {
//        switch (SetupPacket.bRequest)
//        {
//          case HID_REQUEST_GET_REPORT:
//            if (HID_GetReport())
//            {
//              EP0Data.pData = EP0Buffer;
//              USB_DataInStage();
//    //          break; //goto class_ok;
//              result = TRUE;
//            }
//            break;
//          case HID_REQUEST_SET_REPORT:
//            EP0Data.pData = EP0Buf;
//    //        break; //goto class_ok;
//            result = TRUE;
//          case HID_REQUEST_GET_IDLE:
//            if (HID_GetIdle())
//            {
//              EP0Data.pData = EP0Buf;
//              USB_DataInStage();
//    //          break; //goto class_ok;
//              result = TRUE;
//            }
//            break;
//          case HID_REQUEST_SET_IDLE:
//            if (HID_SetIdle())
//            {
//              USB_StatusInStage();
//    //          break; //goto class_ok;
//              result = TRUE;
//            }
//            break;
//          case HID_REQUEST_GET_PROTOCOL:
//            if (HID_GetProtocol())
//            {
//              EP0Data.pData = EP0Buf;
//              USB_DataInStage();
//    //          break; //goto class_ok;
//              result = TRUE;
//            }
//            break;
//          case HID_REQUEST_SET_PROTOCOL:
//            if (HID_SetProtocol())
//            {
//              USB_StatusInStage();
//    //          break; //goto class_ok;
//              result = TRUE;
//            }
//            break;
//        }
//      }
//#endif  /* USB_HID */
//#if USB_MSC
//      if (SetupPacket.wIndex.WB.L == USB_MSC_IF_NUM)
//      {
//        switch (SetupPacket.bRequest)
//        {
//          case MSC_REQUEST_RESET:
//            if (MSC_Reset())
//            {
//              USB_StatusInStage();
//    //          break; //goto class_ok;
//              result = TRUE;
//            }
//            break;
//          case MSC_REQUEST_GET_MAX_LUN:
//            if (MSC_GetMaxLUN())
//            {
//              EP0Data.pData = EP0Buffer;
//              USB_DataInStage();
//    //          break; //goto class_ok;
//              result = TRUE;
//            }
//            break;
//        }
//      }
//#endif  /* USB_MSC */
//#if USB_AUDIO
//      if ((SetupPacket.wIndex.WB.L == USB_ADC_CIF_NUM)  ||
//          (SetupPacket.wIndex.WB.L == USB_ADC_SIF1_NUM) ||
//          (SetupPacket.wIndex.WB.L == USB_ADC_SIF2_NUM))
//      {
//        if (SetupPacket.bmRequestType.BM.Dir)
//        {
//          if (ADC_IF_GetRequest())
//          {
//            EP0Data.pData = EP0Buf;
//            USB_DataInStage();
//    //        break; //goto class_ok;
//            result = TRUE;
//          }
//        }
//        else
//        {
//          EP0Data.pData = EP0Buf;
//    //      break; //goto class_ok;
//          result = TRUE;
//        }
//      }
//#endif  /* USB_AUDIO */
      break;
      
    case REQUEST_TO_ENDPOINT:
      result = USB_CtrlSetupReqEndpoint(SetupPacket.bmRequestType.BM.Dir,
                 EP0Data.pData, EP0Data.Count);
//#if USB_AUDIO
//      if (SetupPacket.bmRequestType.BM.Dir)
//      {
//        if (ADC_EP_GetRequest())
//        {
//          EP0Data.pData = EP0Buffer;
//          USB_DataInStage();
//    //      break; //goto class_ok;
//          result = TRUE;
//        }
//      }
//      else
//      {
//        EP0Data.pData = EP0Buffer;
//    //    break; //goto class_ok;
//        result = TRUE;
//      }
//    //  stall = TRUE; //goto stall_i;
//#endif
      break;
      
    default:
      break;
  }
//  stall = TRUE; //goto stall_i;
  return result;
}

//-----------------------------------------------------------------------------
// U32 usb_CtrlSetupReqClassEndpoint(void)
// {
  // U32 result = FALSE;
// #if USB_AUDIO
  // if (SetupPacket.bmRequestType.BM.Dir)
  // {
    // if (ADC_EP_GetRequest())
    // {
      // EP0Data.pData = EP0Buf;
      // USB_DataInStage();
// //      break; //goto class_ok;
      // result = TRUE;
    // }
  // }
  // else
  // {
    // EP0Data.pData = EP0Buffer;
// //    break; //goto class_ok;
    // result = TRUE;
  // }
// //  stall = TRUE; //goto stall_i;
// #endif
  // return result;
// }

//-----------------------------------------------------------------------------
U32 usb_CtrlOutReqClass(void)
{
  U32 result = FALSE;
  
  switch (SetupPacket.bmRequestType.BM.Recipient)
  {
    case REQUEST_TO_INTERFACE:
#if USB_HID
      if (SetupPacket.wIndex.WB.L == USB_HID_IF_NUM)
      {
        result = HID_SetReport();
    //    if (!HID_SetReport())
    //    {
    //      stall = TRUE; //goto stall_i;
    //      break;
    //    }
        break;
      }
#endif
#if USB_AUDIO
      if ((SetupPacket.wIndex.WB.L == USB_ADC_CIF_NUM)  ||
          (SetupPacket.wIndex.WB.L == USB_ADC_SIF1_NUM) ||
          (SetupPacket.wIndex.WB.L == USB_ADC_SIF2_NUM))
      {
        result = ADC_IF_SetRequest();
    //    if (!ADC_IF_SetRequest())
    //    {
    //      stall = TRUE; //goto stall_i;
    //      break;
    //    }
        break;
      }
#endif
      break;
      
    case REQUEST_TO_ENDPOINT:
#if USB_AUDIO
      result = ADC_EP_SetRequest();
#endif
      break;
      
    default:
      break;
  }
    
//  stall = TRUE; //goto stall_i;
  return result;
}

//-----------------------------------------------------------------------------
// U32 usb_CtrlOutReqClassEndpoint(void)
// {
// #if USB_AUDIO
  // return ADC_EP_SetRequest();
// #endif
  // //stall = TRUE; //goto stall_i;
// }




/** @brief USB Endpoint 0 Event Callback
 *  @param aEvent
 *  @return None
 */
void USB_EndPoint0(U32 aEvent)
{
  U32 result = FALSE;

  switch (aEvent)
  {
    case USB_EVNT_EP_SETUP:
      LOG("SETUP\r\n");
      USB_SetupStage();
      USB_EpDirCtrl(SetupPacket.bmRequestType.BM.Dir);
      EP0Data.Count = SetupPacket.wLength;
      result = FALSE;
      switch (SetupPacket.bmRequestType.BM.Type)
      {
        case REQUEST_STANDARD:
          switch (SetupPacket.bRequest)
          {
            case USB_REQUEST_GET_STATUS:
              LOG("StdGetStat\r\n");
              result = usb_CtrlSetupReqStdGetStatus();
//              if (!USB_GetStatus())
//              {
//                stall = TRUE;
//                break;
//              }
              break;

            case USB_REQUEST_CLEAR_FEATURE:
              result = usb_CtrlSetupReqStdSetClrFeature(FALSE);
//              if (!USB_SetClrFeature(0))
//              {
//                stall = TRUE;
//                break;
//              }
//              USB_StatusInStage();
//              if (NULL != gUSB_CbFeature) gUSB_CbFeature();
              break;

            case USB_REQUEST_SET_FEATURE:
              result = usb_CtrlSetupReqStdSetClrFeature(TRUE);
//              if (!USB_SetClrFeature(1))
//              {
//                stall = TRUE;
//                break;
//              }
//              USB_StatusInStage();
//              if (NULL != gUSB_CbFeature) gUSB_CbFeature();
              break;

            case USB_REQUEST_SET_ADDRESS:
              LOG("StdSetAddr\r\n");
              result = usb_CtrlSetupReqStdSetAddress();
//              switch (SetupPacket.bmRequestType.BM.Recipient)
//              {
//                case REQUEST_TO_DEVICE:
//                  gUSB_DeviceAddress = 0x80 | SetupPacket.wValue.WB.L;
//                  USB_StatusInStage();
//                  break;
//                default:
//                  stall = TRUE;
//                  break;
//              }
              break;

            case USB_REQUEST_GET_DESCRIPTOR:
              LOG("StdGerDscr\r\n");
              result = usb_CtrlSetupReqStdGetDescriptor();
//              if (!USB_GetDescriptor())
//              {
//                stall = TRUE;
//                break;
//              }
              break;

            case USB_REQUEST_SET_DESCRIPTOR:
/*stall_o:*/  //USB_EpSetStall(0x00);
              //EP0Data.Count = 0;
              //stall = TRUE;
              break;

            case USB_REQUEST_GET_CONFIGURATION:
              result = usb_CtrlSetupReqStdGetConfiguration();
//              switch (SetupPacket.bmRequestType.BM.Recipient)
//              {
//                case REQUEST_TO_DEVICE:
//                  EP0Data.pData = &gUSB_Configuration;
//                  USB_DataInStage();
//                  break;
//                default:
//                  stall = TRUE;
//                  break;
//              }
              break;

            case USB_REQUEST_SET_CONFIGURATION:
              result = usb_CtrlSetupReqStdSetConfiguration();
//              switch (SetupPacket.bmRequestType.BM.Recipient)
//              {
//                case REQUEST_TO_DEVICE:
//                  if (!USB_SetConfiguration())
//                  {
//                    stall = TRUE;
//                    break;
//                  }
//                  USB_StatusInStage();
//                  if (NULL != gUSB_CbConfigure) gUSB_CbConfigure();
//                  break;
//                default:
//                  stall = TRUE;
//                  break;
//              }
              break;

            case USB_REQUEST_GET_INTERFACE:
              result = usb_CtrlSetupReqStdGetInterface();
//              switch (SetupPacket.bmRequestType.BM.Recipient)
//              {
//                case REQUEST_TO_INTERFACE:
//                  if ((gUSB_Configuration != 0) &&
//                      (SetupPacket.wIndex.WB.L < gUSB_NumInterfaces))
//                  {
//                    EP0Data.pData = gUSB_AltSetting + SetupPacket.wIndex.WB.L;
//                    USB_DataInStage();
//                  }
//                  else
//                  {
//                    stall = TRUE;
//                  }
//                  break;
//                default:
//                  stall = TRUE;
//                  break;
//              }
              break;

            case USB_REQUEST_SET_INTERFACE:
              result = usb_CtrlSetupReqStdSetInterface();
//              switch (SetupPacket.bmRequestType.BM.Recipient)
//              {
//                case REQUEST_TO_INTERFACE:
//                  if (!USB_SetInterface())
//                  {
//                    stall = TRUE;
//                    break;
//                  }
//                  USB_StatusInStage();
//                  if (NULL != gUSB_CbInterface) gUSB_CbInterface();
//                  break;
//                default:
//                  stall = TRUE;
//                  break;
//              }
              break;

            default:
              //stall = TRUE;
              break;
          }
          break;

        case REQUEST_CLASS:
          result = usb_CtrlSetupReqClass();
//#if USB_CLASS
          // switch (SetupPacket.bmRequestType.BM.Recipient)
          // {
            // case REQUEST_TO_INTERFACE:
              // result = usb_CtrlSetupReqClassInterface();
//#if USB_HID
//              if (SetupPacket.wIndex.WB.L == USB_HID_IF_NUM)
//              {
//                switch (SetupPacket.bRequest)
//                {
//                  case HID_REQUEST_GET_REPORT:
//                    if (HID_GetReport())
//                    {
//                      EP0Data.pData = EP0Buf;
//                      USB_DataInStage();
//                      break; //goto class_ok;
//                    }
//                    break;
//                  case HID_REQUEST_SET_REPORT:
//                    EP0Data.pData = EP0Buf;
//                    break; //goto class_ok;
//                  case HID_REQUEST_GET_IDLE:
//                    if (HID_GetIdle())
//                    {
//                      EP0Data.pData = EP0Buf;
//                      USB_DataInStage();
//                      break; //goto class_ok;
//                    }
//                    break;
//                  case HID_REQUEST_SET_IDLE:
//                    if (HID_SetIdle())
//                    {
//                      USB_StatusInStage();
//                      break; //goto class_ok;
//                    }
//                    break;
//                  case HID_REQUEST_GET_PROTOCOL:
//                    if (HID_GetProtocol())
//                    {
//                      EP0Data.pData = EP0Buf;
//                      USB_DataInStage();
//                      break; //goto class_ok;
//                    }
//                    break;
//                  case HID_REQUEST_SET_PROTOCOL:
//                    if (HID_SetProtocol())
//                    {
//                      USB_StatusInStage();
//                      break; //goto class_ok;
//                    }
//                    break;
//                }
//              }
//#endif  /* USB_HID */
//#if USB_MSC
//              if (SetupPacket.wIndex.WB.L == USB_MSC_IF_NUM)
//              {
//                switch (SetupPacket.bRequest)
//                {
//                  case MSC_REQUEST_RESET:
//                    if (MSC_Reset())
//                    {
//                      USB_StatusInStage();
//                      break; //goto class_ok;
//                    }
//                    break;
//                  case MSC_REQUEST_GET_MAX_LUN:
//                    if (MSC_GetMaxLUN())
//                    {
//                      EP0Data.pData = EP0Buffer;
//                      USB_DataInStage();
//                      break; //goto class_ok;
//                    }
//                    break;
//                }
//              }
//#endif  /* USB_MSC */
//#if USB_AUDIO
//              if ((SetupPacket.wIndex.WB.L == USB_ADC_CIF_NUM)  ||
//                  (SetupPacket.wIndex.WB.L == USB_ADC_SIF1_NUM) ||
//                  (SetupPacket.wIndex.WB.L == USB_ADC_SIF2_NUM))
//              {
//                if (SetupPacket.bmRequestType.BM.Dir)
//                {
//                  if (ADC_IF_GetRequest())
//                  {
//                    EP0Data.pData = EP0Buf;
//                    USB_DataInStage();
//                    break; //goto class_ok;
//                  }
//                }
//                else
//                {
//                  EP0Data.pData = EP0Buf;
//                  break; //goto class_ok;
//                }
//              }
//#endif  /* USB_AUDIO */
//              stall = TRUE; //goto stall_i;
              // break;
// //#if USB_AUDIO
            // case REQUEST_TO_ENDPOINT:
              // result = usb_CtrlSetupReqClassEndpoint();
//              if (SetupPacket.bmRequestType.BM.Dir)
//              {
//                if (ADC_EP_GetRequest())
//                {
//                  EP0Data.pData = EP0Buf;
//                  USB_DataInStage();
//                  break; //goto class_ok;
//                }
//              }
//              else
//              {
//                EP0Data.pData = EP0Buf;
//                break; //goto class_ok;
//              }
//              stall = TRUE; //goto stall_i;
              // break;
// //#endif  /* USB_AUDIO */
            // default:
// //              stall = TRUE; //goto stall_i;
              // break;
          // }
//class_ok: break;
          break;
//#else
//          stall = TRUE; //goto stall_i;
//          break;
//#endif  /* USB_CLASS */

        case REQUEST_VENDOR:
//          stall = TRUE; //goto stall_i;
          break;

        default:
//stall_i:  USB_SetStallEP(0x80);
//          EP0Data.Count = 0;
//          stall = TRUE;
          break;

      }
      if (TRUE != result)
      {
        USB_EpSetStall(0x00);
        EP0Data.Count = 0;
      }
      break;

    case USB_EVNT_EP_OUT:
      if (SetupPacket.bmRequestType.BM.Dir == 0)
      {
        if (EP0Data.Count)
        {
          USB_DataOutStage();
          if (EP0Data.Count == 0)
          {
            result = FALSE;
            switch (SetupPacket.bmRequestType.BM.Type)
            {
              case REQUEST_STANDARD:
//                stall = TRUE; //goto stall_i;
                break;
//#if (USB_CLASS)
              case REQUEST_CLASS:
                result = usb_CtrlOutReqClass();
                // switch (SetupPacket.bmRequestType.BM.Recipient)
                // {
                  // case REQUEST_TO_INTERFACE:
                    // result = usb_CtrlOutReqClassInterface();
//#if USB_HID
//                    if (SetupPacket.wIndex.WB.L == USB_HID_IF_NUM)
//                    {
//                      if (!HID_SetReport())
//                      {
//                        stall = TRUE; //goto stall_i;
//                        break;
//                      }
//                      break;
//                    }
//#endif
//#if USB_AUDIO
//                    if ((SetupPacket.wIndex.WB.L == USB_ADC_CIF_NUM)  ||
//                        (SetupPacket.wIndex.WB.L == USB_ADC_SIF1_NUM) ||
//                        (SetupPacket.wIndex.WB.L == USB_ADC_SIF2_NUM)) {
//                      if (!ADC_IF_SetRequest())
//                      {
//                        stall = TRUE; //goto stall_i;
//                        break;
//                      }
//                      break;
//                    }
//#endif
//                    stall = TRUE; //goto stall_i;
                    // break;

                  // case REQUEST_TO_ENDPOINT:
                    // result = usb_CtrlOutReqClassEndpoint();
//#if USB_AUDIO
//                    if (ADC_EP_SetRequest()) break;
//#endif
//                    stall = TRUE; //goto stall_i;
                    // break;

                  // default:
// //                    stall = TRUE; //goto stall_i;
                    // break;
                // }
                break;
//#endif
              default:
//                stall = TRUE; //goto stall_i;
                break;
            }
            if (TRUE == result)
            {
              USB_StatusInStage();
            }
            else
            {
              USB_EpSetStall(0x00);
              EP0Data.Count = 0;
            }
          }
        }
      }
      else
      {
        USB_StatusOutStage();
      }
      break;

    case USB_EVNT_EP_IN:
      if (SetupPacket.bmRequestType.BM.Dir == 1)
      {
        USB_DataInStage();
      }
      else
      {
        if (gUSB_DeviceAddress & 0x80)
        {
          gUSB_DeviceAddress &= 0x7F;
          USB_SetAddress(gUSB_DeviceAddress);
        }
      }
      break;

    case USB_EVNT_EP_IN_STALL:
      USB_EpClrStall(0x80);
      break;

    case USB_EVNT_EP_OUT_STALL:
      USB_EpClrStall(0x00);
      break;
  }
  
//  if (TRUE == stall)
//  {
//    USB_EpSetStall(0x00);
//    EP0Data.Count = 0;
//  }
}
