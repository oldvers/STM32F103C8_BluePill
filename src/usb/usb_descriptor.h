#ifndef __USB_DESCRIPTOR_H__
#define __USB_DESCRIPTOR_H__

#include "usb_descriptor_definitions.h"
#include "usb_device.h"

/* --- Class Specific Optional Function Prototypes -------------------------- */

U8 USBD_ICEMKII_IEndPointWrWsCb(USBD_CbByte pGetByteCb, U8 aSize);
U8 USBD_ICEMKII_OEndPointRdWsCb(USBD_CbByte pPutByteCb, U8 aSize);

#endif  /* __USB_DESCRIPTOR_H__ */
