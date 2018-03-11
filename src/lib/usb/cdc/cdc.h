#ifndef __CDC_H__
#define __CDC_H__

void CDC_Init(void);
USB_CTRL_STAGE CDC_CtrlSetupReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
);
USB_CTRL_STAGE CDC_CtrlOutReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
);
void CDC_InterruptIn(U32 aEvent);
void CDC_BulkIn(U32 aEvent);
void CDC_BulkOut(U32 aEvent);

#endif /* __CDC_H__ */
