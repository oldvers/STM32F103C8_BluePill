#ifndef __USB_DESCRIPTOR_H__
#define __USB_DESCRIPTOR_H__

#include "usb_descriptor_definitions.h"

/* --- Class Specific Optional Function Prototypes -------------------------- */

U8   USBD_MSC_IEndPointWr        (U8 *pData, U8 aSize);
U8   USBD_MSC_OEndPointRd        (U8 *pData, U8 aSize);
void USBD_MSC_OEndPointSetStall  (void);
void USBD_MSC_IEndPointSetStall  (void);
U8   USBD_CDC_GetInterfaceNumber (void);
U8   USBD_CDC_IrqEndPointWr      (U8 *pData, U8 aSize);
U8   USBD_CDC_IEndPointWr        (U8 *pData, U8 aSize);
U8   USBD_CDC_OEndPointRd        (U8 *pData, U8 aSize);
U8   USBD_HID_InEndPointWr       (U8 *pData, U8 aSize);

#endif  /* __USB_DESCRIPTOR_H__ */
