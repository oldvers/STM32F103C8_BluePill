#ifndef __MSC_H__
#define __MSC_H__

#include "types.h"
#include "usb_cfg.h"
#include "usb_defs.h"
#include "usb_core.h"

void MSC_Init(void);
USB_CTRL_STAGE MSC_CtrlSetupReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
);

#endif  /* __MSC_H__ */
