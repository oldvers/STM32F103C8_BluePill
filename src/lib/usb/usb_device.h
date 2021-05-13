#ifndef __USB_DEVICE_H__
#define __USB_DEVICE_H__

typedef void (*USBD_CbByte)(U8 * pByte);

/* --- Functions ------------------------------------------------------------ */

void      USBD_Init         (void);
void      USBD_DeInit       (void);

/* --- Wrappers ------------------------------------------------------------- */

U32        USBD_EP_Rd        (U8 aNumber, U8 *pData, U32 aSize);
U32        USBD_EP_Wr        (U8 aNumber, U8 *pData, U32 aSize);
U32        USBD_EP_RdWsCb    (U8 aNumber, USBD_CbByte pPutByteCb, U32 aSize);
U32        USBD_EP_WrWsCb    (U8 aNumber, USBD_CbByte pGetByteCb, U32 aSize);
void       USBD_EP_SetStall  (U8 aNumber);
void       USBD_EP_ClrStall  (U8 aNumber);
FW_BOOLEAN USBD_EP_IsRxEmpty (U8 aNumber);
FW_BOOLEAN USBD_EP_IsTxEmpty (U8 aNumber);

#endif /* __USB_DEVICE_H__ */
