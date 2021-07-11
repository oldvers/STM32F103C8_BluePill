#ifndef __USB_DESCRIPTOR_H__
#define __USB_DESCRIPTOR_H__

#include "usb_descriptor_definitions.h"
#include "usb_device.h"

/* --- Class Specific Optional Function Prototypes -------------------------- */

U8         USBD_CDC_UART_GetInterfaceNumber (void);
U32        USBD_CDC_UART_IrqEndPointWr      (U8 *pData, U32 aSize);
U32        USBD_CDC_UART_IEndPointWrWsCb    (USBD_CbByte pGetByteCb, U32 aSize);
FW_BOOLEAN USBD_CDC_UART_IEndPointIsTxEmpty (void);
U32        USBD_CDC_UART_OEndPointRdWsCb    (USBD_CbByte pPutByteCb, U32 aSize);
FW_BOOLEAN USBD_CDC_UART_OEndPointIsRxEmpty (void);

U8         USBD_CDC_I2C_GetInterfaceNumber (void);
U32        USBD_CDC_I2C_IrqEndPointWr      (U8 *pData, U32 aSize);
U32        USBD_CDC_I2C_IEndPointWrWsCb    (USBD_CbByte pGetByteCb, U32 aSize);
FW_BOOLEAN USBD_CDC_I2C_IEndPointIsTxEmpty (void);
U32        USBD_CDC_I2C_OEndPointRdWsCb    (USBD_CbByte pPutByteCb, U32 aSize);
FW_BOOLEAN USBD_CDC_I2C_OEndPointIsRxEmpty (void);

U8         USBD_CDC_SPI_GetInterfaceNumber (void);
U32        USBD_CDC_SPI_IrqEndPointWr      (U8 *pData, U32 aSize);
U32        USBD_CDC_SPI_IEndPointWrWsCb    (USBD_CbByte pGetByteCb, U32 aSize);
FW_BOOLEAN USBD_CDC_SPI_IEndPointIsTxEmpty (void);
U32        USBD_CDC_SPI_OEndPointRdWsCb    (USBD_CbByte pPutByteCb, U32 aSize);
FW_BOOLEAN USBD_CDC_SPI_OEndPointIsRxEmpty (void);

#endif  /* __USB_DESCRIPTOR_H__ */
