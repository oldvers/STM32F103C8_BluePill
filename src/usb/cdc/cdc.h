#ifndef __CDC_H__
#define __CDC_H__

/* --- Class Specific Definitions ------------------------------------------- */

/* CDC Interrupt Endpoint Max Packet Size */
#define USB_CDC_IRQ_PACKET_SIZE    (8)
/* CDC Interrupt Endpoint Polling Interval (ms) */
#define USB_CDC_IRQ_INTERVAL       (16)
/* CDC Bulk Endpoint Max Packet Size */
#define USB_CDC_PACKET_SIZE        (64)

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
void CDC_SOF(void);
void CDC_InterruptIn(U32 aEvent);
void CDC_BulkIn(U32 aEvent);
void CDC_BulkOut(U32 aEvent);

#endif /* __CDC_H__ */
