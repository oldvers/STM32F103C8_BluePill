#ifndef __ICEMKII_H__
#define __ICEMKII_H__

void ICEMKII_Init(void);
USB_CTRL_STAGE ICEMKII_CtrlSetupReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
);
USB_CTRL_STAGE ICEMKII_CtrlOutReq
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
);

#endif /* __ICEMKII_H__ */
