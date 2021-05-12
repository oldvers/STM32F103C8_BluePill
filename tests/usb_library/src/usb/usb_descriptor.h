#ifndef __USB_DESCRIPTOR_H__
#define __USB_DESCRIPTOR_H__

#include "usb_control.h"

typedef void (*USBD_CbGeneric)(void);
typedef void (*USBD_CbEndPoint)(U32 aEvent);
typedef USB_CTRL_STAGE (*USBD_CbControl)
(
  USB_SETUP_PACKET *pSetup,
  U8 **pData,
  U16 *pSize
);

typedef struct
{
  USBD_CbGeneric  CbInit;
  USBD_CbControl  CbCtrlSetup;
  USBD_CbControl  CbCtrlOut;
  USBD_CbGeneric  CbSOF;
  USBD_CbEndPoint CbEndPointI;
  USBD_CbEndPoint CbEndPointO;
  U8              EndPointI;
  U8              EndPointO;
} USBD_INTERFACE_CALLBACKS_DESCRIPTOR;

extern const USBD_INTERFACE_CALLBACKS_DESCRIPTOR USBD_IfCbDescriptor[];

/* --- Common Mandatory Function Prototypes --------------------------------- */

U8         *USBD_GetDeviceDescriptor(void);
U8         *USBD_GetConfigDescriptor(void);
U8         *USBD_GetStringDescriptor(U8 aIndex);
FW_BOOLEAN  USBD_GetItrfaceDescriptor
            (
                USB_SETUP_PACKET * pSetup,
                U8 **pData,
                U16 *pSize
            );
U8          USBD_GetItrfacesCount(void);

/* --- Class Specific Optional Function Prototypes -------------------------- */

U8 USBD_HID_InEndPointWr(U8 *pData, U8 aSize);

#endif  /* __USB_DESCRIPTOR_H__ */
