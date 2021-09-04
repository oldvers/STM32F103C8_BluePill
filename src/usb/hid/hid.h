#ifndef __HID_H__
#define __HID_H__

#include "types.h"
#include "usb_config.h"
#include "usb_control.h"

/* HID Number of Reports */
#define HID_REPORT_NUM      1

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
void HID_IrqInReq(U32 aEvent);

#endif  /* __HID_H__ */