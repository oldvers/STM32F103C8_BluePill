#ifndef __USB_DEVICE_H__
#define __USB_DEVICE_H__

#include "usb_defs.h"
#include "usb_core.h"

//void USB_CbFeature(void);
//void USB_CbConfigure(U8 aConfig);
//void USB_CbInterface(void);
//USB_CTRL_STAGE USB_CtrlSetupReqItrface(USB_SETUP_PACKET *pSetup, U8 **pData, U16 *pSize);
//USB_CTRL_STAGE USB_CtrlSetupReqEndpoint(USB_SETUP_PACKET *pSetup, U8 **pData, U16 *pSize);
//USB_CTRL_STAGE USB_CtrlOutReqClassItrface(USB_SETUP_PACKET *pSetup, U8 **pData, U16 *pSize);
//USB_CTRL_STAGE USB_CtrlOutReqClassEndpoint(USB_SETUP_PACKET *pSetup, U8 **pData, U16 *pSize);

void USBD_Init(void);

#endif /* __USB_DEVICE_H__ */
