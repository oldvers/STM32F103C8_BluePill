#ifndef __USB_DESCR_H__
#define __USB_DESCR_H__

#define WBVAL(x) (x & 0xFF),((x >> 8) & 0xFF)

#define USB_DEVICE_DESC_SIZE        (sizeof(USB_DEVICE_DESCRIPTOR))
#define USB_CONFIGUARTION_DESC_SIZE (sizeof(USB_CONFIGURATION_DESCRIPTOR))
#define USB_INTERFACE_DESC_SIZE     (sizeof(USB_INTERFACE_DESCRIPTOR))
#define USB_ENDPOINT_DESC_SIZE      (sizeof(USB_ENDPOINT_DESCRIPTOR))

const U8 USB_DeviceDescriptor[];
const U8 USB_ConfigDescriptor[];
const U8 USB_StringDescriptor[];

#endif  /* __USB_DESCR_H__ */
