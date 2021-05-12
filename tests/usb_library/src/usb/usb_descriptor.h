#ifndef __USB_DESCRIPTOR_H__
#define __USB_DESCRIPTOR_H__

#include "usb_descriptor_definitions.h"

/* --- Class Specific Optional Function Prototypes -------------------------- */

U8 USBD_HID_InEndPointWr(U8 *pData, U8 aSize);

#endif  /* __USB_DESCRIPTOR_H__ */
