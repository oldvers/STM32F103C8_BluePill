#ifndef __USB_DESCRIPTOR_H__
#define __USB_DESCRIPTOR_H__

#include "usb_descriptor_definitions.h"
#include "usb_device.h"

/* --- Class Specific Optional Function Prototypes -------------------------- */

U32 USBD_ICEMKII_IEndPointWrWsCb(USBD_CbByte pGetByteCb, U32 aSize);
U32 USBD_ICEMKII_OEndPointRdWsCb(USBD_CbByte pPutByteCb, U32 aSize);

#endif  /* __USB_DESCRIPTOR_H__ */
