#ifndef __MSC_H__
#define __MSC_H__

#include "types.h"
#include "usb_defs.h"
#include "usb_core.h"

/* MSC Disk Image Definitions */
#define MSC_ImageSize   0x00001000

/* Mass Storage Memory Layout */
#define MSC_MemorySize  8192
#define MSC_BlockSize   512
#define MSC_BlockCount  (MSC_MemorySize / MSC_BlockSize)

/* Max In/Out Packet Size */
#define MSC_MAX_PACKET  64

/* MSC In/Out Endpoint Address */
#define MSC_EP_IN       0x81
#define MSC_EP_OUT      0x02

/* MSC Requests Callback Functions */
void MSC_Init(void);
//USB_CTRL_STAGE MSC_CtrlSetupReqItrface(U8 aReq, U8 **pData, U16 *pSize);
USB_CTRL_STAGE MSC_CtrlSetupReq(USB_SETUP_PACKET * pSetup, U8 **pData, U16 *pSize);
//U32  MSC_Reset(void);
//U32  MSC_GetMaxLUN(U8 * pData, U32 aSize);

/* MSC Bulk Callback Functions */
//extern void MSC_GetCBW (void);
//extern void MSC_SetCSW (void);
void MSC_BulkIn (U32 aEvent);
void MSC_BulkOut(U32 aEvent);

#endif  /* __MSC_H__ */
