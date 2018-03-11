#ifndef __HID_H__
#define __HID_H__

#include "types.h"
#include "usb_cfg.h"
#include "usb_core.h"

/* HID Number of Reports */
#define HID_REPORT_NUM      1

/* HID Global Variables */
//extern BYTE HID_Protocol;
//extern BYTE HID_IdleTime[HID_REPORT_NUM];

/* HID Requests Callback Functions */
//extern BOOL HID_GetReport   (void);
//extern BOOL HID_SetReport   (void);
//extern BOOL HID_GetIdle     (void);
//extern BOOL HID_SetIdle     (void);
//extern BOOL HID_GetProtocol (void);
//extern BOOL HID_SetProtocol (void);

void HID_Init(void);
USB_CTRL_STAGE HID_CtrlSetupReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
);
USB_CTRL_STAGE HID_CtrlOutReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
);
void HID_InterruptIn(U32 aEvent);

#endif  /* __HID_H__ */
