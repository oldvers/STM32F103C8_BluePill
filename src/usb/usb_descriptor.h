#ifndef __USB_DESCRIPTOR_H__
#define __USB_DESCRIPTOR_H__

#include "usb_descriptor_definitions.h"

/* --- Class Specific Optional Function Prototypes -------------------------- */

U8  USBD_CDC_GetInterfaceNumber (void);
U32 USBD_CDC_IrqEndPointWr      (U8 *pData, U32 aSize);
U32 USBD_CDC_IEndPointWr        (U8 *pData, U32 aSize);
U32 USBD_CDC_OEndPointRd        (U8 *pData, U32 aSize);

#endif  /* __USB_DESCRIPTOR_H__ */
