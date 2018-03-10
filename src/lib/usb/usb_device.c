#include "types.h"

#include "usb.h"
#include "usb_cfg.h"
#include "usb_defs.h"
#include "usb_core.h"
#include "usb_device.h"
#include "msc.h"
#include "msc_defs.h"
//#include "memory.h"
  
//USB_CTRL_STAGE USB_CtrlSetupReqItrface(USB_SETUP_PACKET * pSetup, U8 **pData, U16 *pSize)
//{
//  //U32 result = FALSE;
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
////  if (pSetup->wIndex.WB.L == USB_MSC_IF_NUM)
////  {
//    return MSC_CtrlSetupReqItrface(pSetup, pData, pSize);
////    switch (aReq)
////        {
////          case MSC_REQUEST_RESET:
////            if (MSC_Reset())
////            {
//////              USB_StatusInStage();
//////    //          break; //goto class_ok;
//////              result = TRUE;
////            }
////            break;
////          case MSC_REQUEST_GET_MAX_LUN:
////            if (MSC_GetMaxLUN(pData, *pSize))
////            {
//////              EP0Data.pData = EP0Buffer;
//////              USB_DataInStage();
//////    //          break; //goto class_ok;
//////              result = TRUE;
////            }
////            break;
////        }
////      }
////  }
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
////  return result;
////  return USB_CTRL_STAGE_ERROR;
//}

//USB_CTRL_STAGE USB_CtrlSetupReqEndpoint(USB_SETUP_PACKET * pSetup, U8 **pData, U16 *pSize)
//{
//  //U32 result = FALSE;
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
//  return USB_CTRL_STAGE_ERROR;
//}

//USB_CTRL_STAGE USB_CtrlOutReqClassItrface(USB_SETUP_PACKET *pSetup, U8 **pData, U16 *pSize)
//{
//  return USB_CTRL_STAGE_ERROR;
//}

//USB_CTRL_STAGE USB_CtrlOutReqClassEndpoint(USB_SETUP_PACKET *pSetup, U8 **pData, U16 *pSize)
//{
//  return USB_CTRL_STAGE_ERROR;
//}

/*
 *  USB Power Event Callback
 *   Called automatically on USB Power Event
 *    Parameter:       power: On(TRUE)/Off(FALSE)
 */
//void USB_Power_Event(BOOL  power)
//{
//#if USB_POWER_EVENT
//  //
//#endif
//}

/*
 *  USB Reset Event Callback
 *   Called automatically on USB Reset Event
 */
#if USB_RESET_EVENT
static void usb_CbReset(void)
{
  USBC_Reset();
//  GPIOB->ODR &= ~LED_CFG;	        /* Turn Off Cfg LED */
}
#endif


/*
 *  USB Suspend Event Callback
 *   Called automatically on USB Suspend Event
 */

#if USB_SUSPEND_EVENT
static void usb_CbSuspend(void)
{
//  GPIOB->ODR |= LED_SUSP;            /* Turn On Suspend LED */
}
#endif


/*
 *  USB Resume Event Callback
 *   Called automatically on USB Resume Event
 */

#if USB_RESUME_EVENT
static void usb_CbResume(void)
{
  //
}
#endif

/*
 *  USB Remote Wakeup Event Callback
 *   Called automatically on USB Remote Wakeup Event
 */
#if USB_WAKEUP_EVENT
static void usb_CbWakeUp(void)
{
  //  GPIOB->ODR &= ~LED_SUSP;           /* Turn Off Suspend LED */
}
#endif

/*
 *  USB Start of Frame Event Callback
 *   Called automatically on USB Start of Frame Event
 */
#if USB_SOF_EVENT
static void usb_CbSOF(void)
{
  //
}
#endif


/*
 *  USB Error Event Callback
 *   Called automatically on USB Error Event
 *    Parameter:       error: Error Code
 */
#if USB_ERROR_EVENT
static void usb_CbError(U32 aError)
{
  //
}
#endif

/*
 *  USB Set Configuration Event Callback
 *   Called automatically on USB Set Configuration Request
 */
void USB_CbConfigure(U8 aConfig)
{
#if USB_CONFIGURE_EVENT
  /* Check if USB is configured */
  if (aConfig)
  {
//    GPIOB->ODR |=  LED_CFG;          /* Turn On Cfg LED */
  }
  else
  {
//    GPIOB->ODR &= ~LED_CFG;          /* Turn Off Cfg LED */
  }
#endif
}

/*
 *  USB Set Interface Event Callback
 *   Called automatically on USB Set Interface Request
 */
void USB_CbInterface(void)
{
#if USB_INTERFACE_EVENT
  //
#endif
}

/*
 *  USB Set/Clear Feature Event Callback
 *   Called automatically on USB Set/Clear Feature Request
 */
void USB_CbFeature(void)
{
#if USB_FEATURE_EVENT
  //
#endif
}

/*
 *  USB Endpoint 1 Event Callback
 *   Called automatically on USB Endpoint 1 Event
 *    Parameter:       event
 */

//void USB_EndPoint1(U32 aEvent)
//{
//  //MSC_BulkIn();
//}

/*
 *  USB Endpoint 2 Event Callback
 *   Called automatically on USB Endpoint 2 Event
 *    Parameter:       event
 */
//void USB_EndPoint2(U32 event)
//{
//  //MSC_BulkOut();
//}

const USB_CORE_EVENTS USBC_Events =
{
  USB_CbFeature,
  USB_CbConfigure,
  USB_CbInterface,
  MSC_CtrlSetupReq,
  NULL,
};

void USBD_Init(void)
{
#if USB_RESET_EVENT
  USB_SetCb_Reset(usb_CbReset);
#endif
#if USB_SUSPEND_EVENT
  USB_SetCb_Suspend(usb_CbSuspend);
#endif
//#if USB_RESUME_EVENT
//static void usb_CbResume(void)
//#endif
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
  USB_Init(3, USB_MAX_PACKET0);
  /* Register Callback for Control Endpoint */
  USB_SetCb_Ep(0, USBC_ControlInOut);
  /* Init Mass Storage Device */
  MSC_Init();
  USB_SetCb_Ep(MSC_EP_IN,  MSC_BulkIn);
  USB_SetCb_Ep(MSC_EP_OUT, MSC_BulkOut);
  /* Init Core */
  USBC_Init(&USBC_Events);
  /* Connect USB */
  USB_Connect(TRUE);
}
