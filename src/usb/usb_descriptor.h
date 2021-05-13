#ifndef __USB_DESCRIPTOR_H__
#define __USB_DESCRIPTOR_H__

#include "usb_descriptor_definitions.h"
#include "usb_device.h"

/* --- Class Specific Optional Function Prototypes -------------------------- */

U8         USBD_CDC_GetInterfaceNumber (void);
U32        USBD_CDC_IEndPointWrWsCb    (USBD_CbByte pGetByteCb, U32 aSize);
FW_BOOLEAN USBD_CDC_IEndPointIsTxEmpty (void);
U32        USBD_CDC_OEndPointRdWsCb    (USBD_CbByte pPutByteCb, U32 aSize);
FW_BOOLEAN USBD_CDC_OEndPointIsRxEmpty (void);

U8         USBD_CDD_GetInterfaceNumber (void);
U32        USBD_CDD_IEndPointWrWsCb    (USBD_CbByte pGetByteCb, U32 aSize);
FW_BOOLEAN USBD_CDD_IEndPointIsTxEmpty (void);
U32        USBD_CDD_OEndPointRdWsCb    (USBD_CbByte pPutByteCb, U32 aSize);
FW_BOOLEAN USBD_CDD_OEndPointIsRxEmpty (void);

#endif  /* __USB_DESCRIPTOR_H__ */
