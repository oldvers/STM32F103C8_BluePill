#ifndef __ICEMKII_H__
#define __ICEMKII_H__

/* Max ICE mkII Bulk In/Out Endpoint Packet Size */
#define USB_ICEMKII_PACKET_SIZE    (64)
/* Max ICE mkII Bulk In/Out Endpoint Interval */
#define USB_ICEMKII_BULK_INTERVAL  (10)

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
void ICEMKII_BulkIn(U32 aEvent);
void ICEMKII_BulkOut(U32 aEvent);

#endif /* __ICEMKII_H__ */
