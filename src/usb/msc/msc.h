#ifndef __MSC_H__
#define __MSC_H__

#include "types.h"
#include "usb_definitions.h"
#include "usb_control.h"

/* --- Class Specific Definitions ------------------------------------------- */

/* MSC Endpoint Max Packet Size */
#define USB_MSC_PACKET_SIZE        (64)

void           MSC_Init            (void);
USB_CTRL_STAGE MSC_CtrlSetupReq    (
                                     USB_SETUP_PACKET * pSetup,
                                     U8 **pData,
                                     U16 *pSize
                                   );
void           MSC_BulkIn          (U32 aEvent);
void           MSC_BulkOut         (U32 aEvent);

#endif  /* __MSC_H__ */