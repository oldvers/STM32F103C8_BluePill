#ifndef __USB_H__
#define __USB_H__

/* Endpoint Direction */
#define USB_EP_ADDR_DIR_MASK             (0x80)
/* Endpoint Type */
typedef enum USB_EP_TYPE_E
{
  USB_EP_TYPE_BULK        = 0,
  USB_EP_TYPE_CONTROL     = 1,
  USB_EP_TYPE_ISOCHRONOUS = 2,
  USB_EP_TYPE_INTERRUPT   = 3,
} USB_EP_TYPE;

///* Endpoint Callback Events */
//#define USB_EVNT_EP_SETUP                (1)
//#define USB_EVNT_EP_OUT                  (2)
//#define USB_EVNT_EP_IN                   (3)

/* Endpoint Callback Events */
/* Setup Packet */
#define USB_EVNT_EP_SETUP                (1)
/* OUT Packet */
#define USB_EVNT_EP_OUT                  (2)
/*  IN Packet */
#define USB_EVNT_EP_IN                   (3)
/* OUT Packet - Not Acknowledged */
#define USB_EVNT_EP_OUT_NAK              (4)
/*  IN Packet - Not Acknowledged */
#define USB_EVNT_EP_IN_NAK               (5)
/* OUT Packet - Stalled */
#define USB_EVNT_EP_OUT_STALL            (6)
/*  IN Packet - Stalled */
#define USB_EVNT_EP_IN_STALL             (7)
/* DMA OUT EP - End of Transfer */
#define USB_EVNT_EP_OUT_DMA_EOT          (8)
/* DMA  IN EP - End of Transfer */
#define USB_EVNT_EP_IN_DMA_EOT           (9)
/* DMA OUT EP - New Descriptor Request */
#define USB_EVNT_EP_OUT_DMA_NDR          (10)
/* DMA  IN EP - New Descriptor Request */
#define USB_EVNT_EP_IN_DMA_NDR           (11)
/* DMA OUT EP - Error */
#define USB_EVNT_EP_OUT_DMA_ERR          (12)
/* DMA  IN EP - Error */
#define USB_EVNT_EP_IN_DMA_ERR           (13)

/* Callback Function Declarations */
typedef void (*USB_CbGeneric)(void);
typedef void (*USB_CbError)(U32 aError);
typedef void (*USB_CbEp)(U32 aEvent);
/* Function Declarations */
void USB_SetCb_Reset    (USB_CbGeneric pCbReset);
void USB_SetCb_Suspend  (USB_CbGeneric pCbSuspend);
void USB_SetCb_WakeUp   (USB_CbGeneric pCbWakeUp);
void USB_SetCb_SOF      (USB_CbGeneric pCbSOF);
void USB_SetCb_Error    (USB_CbError pCbError);
void USB_SetCb_Ep       (U32 aNumber, USB_CbEp pCbEp);
void USB_Init           (U32 aMaxEpCount, U32 aCtrlEpMaxPacketSize);
void USB_Connect        (U32 aConnnect);
void USB_Reset          (void);
void USB_Suspend        (void);
void USB_Resume         (void);
void USB_WakeUp         (void);
void USB_WakeUpConfigure(U32 aConfig);
void USB_SetAddress     (U32 aAddress);
void USB_Configure      (U32 aConfig);
void USB_EpConfigure    (U8 aAddress, U16 aMaxPacketSize, USB_EP_TYPE aType);
void USB_EpDirCtrl      (U32 aDirection);
void USB_EpEnable       (U32 aNumber);
void USB_EpDisable      (U32 aNumber);
void USB_EpReset        (U32 aNumber);
void USB_EpSetStall     (U32 aNumber);
void USB_EpClrStall     (U32 aNumber);
U32  USB_EpRead         (U32 aNumber, U8 *pData);
U32  USB_EpWrite        (U32 aNumber, U8 *pData, U32 aSize);
U32  USB_GetFrame       (void);
/* Interrupt Handler Declaration */
void USB_IRQHandler  (void);

#endif  /* __USBHW_H__ */
