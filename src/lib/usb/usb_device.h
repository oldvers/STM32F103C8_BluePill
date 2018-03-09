#ifndef __USB_CLASS_H__
#define __USB_CLASS_H__

void USB_CbFeature(void);
void USB_CbConfigure(U8 aConfig);
void USB_CbInterface(void);
U32  USB_CtrlSetupReqItrface(U8 aItrface, U8 aReq, U8 * pData, U32 aSize);
U32  USB_CtrlSetupReqEndpoint(U8 aDir, U8 * pData, U32 aSize);

void USBD_Init(void);

#endif /* __USB_CLASS_H__ */
