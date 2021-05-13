#ifndef __USB_DEVICE_H__
#define __USB_DEVICE_H__

typedef void (*USBD_CbByte)(U8 * pByte);

void USBD_Init              (void);
void USBD_DeInit            (void);
U32  USBD_EndPointRd        (U8 aNumber, U8 *pData, U32 aSize);
U32  USBD_EndPointWr        (U8 aNumber, U8 *pData, U32 aSize);
U32  USBD_EndPointRdWsCb    (U8 aNumber, USBD_CbByte pPutByteCb, U32 aSize);
U32  USBD_EndPointWrWsCb    (U8 aNumber, USBD_CbByte pGetByteCb, U32 aSize);
void USBD_EndPointSetStall  (U8 aNumber);
void USBD_EndPointClrStall  (U8 aNumber);

#endif /* __USB_DEVICE_H__ */
