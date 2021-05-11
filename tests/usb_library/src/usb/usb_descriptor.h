#ifndef __USB_DESCRIPTOR_H__
#define __USB_DESCRIPTOR_H__

#define WBVAL(x) (x & 0xFF),((x >> 8) & 0xFF)

#define USB_DEVICE_DESC_SIZE        (sizeof(USB_DEVICE_DESCRIPTOR))
#define USB_CONFIGUARTION_DESC_SIZE (sizeof(USB_CONFIGURATION_DESCRIPTOR))
#define USB_INTERFACE_DESC_SIZE     (sizeof(USB_INTERFACE_DESCRIPTOR))
#define USB_ENDPOINT_DESC_SIZE      (sizeof(USB_ENDPOINT_DESCRIPTOR))

U8         *USB_GetDeviceDescriptor(void);
U8         *USB_GetConfigDescriptor(void);
U8         *USB_GetStringDescriptor(U8 aIndex);
FW_BOOLEAN  USB_GetItrfaceDescriptor
            (
                USB_SETUP_PACKET * pSetup,
                U8 **pData,
                U16 *pSize
            );

U8 USBD_HID_InEndPointWr(U8 *pData, U8 aSize);

#endif  /* __USB_DESCRIPTOR_H__ */
