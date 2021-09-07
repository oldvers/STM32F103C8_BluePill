#ifndef __USB_DEFINITIONS_H__
#define __USB_DEFINITIONS_H__

typedef __packed union
{
  U16 W;
  __packed struct
  {
    U8 L;
    U8 H;
  } WB;
} WORD_BYTE;

/* bmRequestType.Dir */
#define REQUEST_HOST_TO_DEVICE     0
#define REQUEST_DEVICE_TO_HOST     1

/* bmRequestType.Type */
#define REQUEST_STANDARD           0
#define REQUEST_CLASS              1
#define REQUEST_VENDOR             2
#define REQUEST_RESERVED           3

/* bmRequestType.Recipient */
#define REQUEST_TO_DEVICE          0
#define REQUEST_TO_INTERFACE       1
#define REQUEST_TO_ENDPOINT        2
#define REQUEST_TO_OTHER           3

/* bmRequestType Definition */
typedef __packed union _REQUEST_TYPE
{
  __packed struct _BM
  {
    U8 Recipient : 5;
    U8 Type      : 2;
    U8 Dir       : 1;
  } BM;
  U8 B;
} REQUEST_TYPE;

/* USB Standard Request Codes */
#define USB_REQUEST_GET_STATUS                 0
#define USB_REQUEST_CLEAR_FEATURE              1
#define USB_REQUEST_SET_FEATURE                3
#define USB_REQUEST_SET_ADDRESS                5
#define USB_REQUEST_GET_DESCRIPTOR             6
#define USB_REQUEST_SET_DESCRIPTOR             7
#define USB_REQUEST_GET_CONFIGURATION          8
#define USB_REQUEST_SET_CONFIGURATION          9
#define USB_REQUEST_GET_INTERFACE              10
#define USB_REQUEST_SET_INTERFACE              11
#define USB_REQUEST_SYNC_FRAME                 12

/* USB GET_STATUS Bit Values */
#define USB_GETSTATUS_SELF_POWERED             0x01
#define USB_GETSTATUS_REMOTE_WAKEUP            0x02
#define USB_GETSTATUS_ENDPOINT_STALL           0x01

/* USB Standard Feature selectors */
#define USB_FEATURE_ENDPOINT_STALL             0
#define USB_FEATURE_REMOTE_WAKEUP              1

/* USB Default Control Pipe Setup Packet */
typedef __packed struct _USB_SETUP_PACKET
{
  REQUEST_TYPE bmRequestType;
  U8           bRequest;
  WORD_BYTE    wValue;
  WORD_BYTE    wIndex;
  U16          wLength;
} USB_SETUP_PACKET;

/* USB Descriptor Types */
#define USB_DEVICE_DESCRIPTOR_TYPE             0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE      0x02
#define USB_STRING_DESCRIPTOR_TYPE             0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE          0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE           0x05
#define USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE   0x06
#define USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE 0x07
#define USB_INTERFACE_POWER_DESCRIPTOR_TYPE    0x08
#define USB_IF_ASSOCIATION_DESCRIPTOR_TYPE     0x0B

/* USB Device Classes */
#define USB_DEVICE_CLASS_RESERVED              0x00
#define USB_DEVICE_CLASS_AUDIO                 0x01
#define USB_DEVICE_CLASS_COMMUNICATIONS        0x02
#define USB_DEVICE_CLASS_HUMAN_INTERFACE       0x03
#define USB_DEVICE_CLASS_MONITOR               0x04
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE    0x05
#define USB_DEVICE_CLASS_POWER                 0x06
#define USB_DEVICE_CLASS_PRINTER               0x07
#define USB_DEVICE_CLASS_STORAGE               0x08
#define USB_DEVICE_CLASS_HUB                   0x09
#define USB_DEVICE_CLASS_MISCELLANEOUS         0xEF
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC       0xFF

/* bmAttributes in Configuration Descriptor */
#define USB_CONFIG_POWERED_MASK                0xC0
#define USB_CONFIG_BUS_POWERED                 0x80
#define USB_CONFIG_SELF_POWERED                0xC0
#define USB_CONFIG_REMOTE_WAKEUP               0x20

/* bMaxPower in Configuration Descriptor */
#define USB_CONFIG_POWER_MA(mA)                ((mA)/2)

/* bEndpointAddress in Endpoint Descriptor */
#define USB_ENDPOINT_DIRECTION_MASK            0x80
#define USB_ENDPOINT_O(addr)                   ((addr) | 0x00)
#define USB_ENDPOINT_I(addr)                   ((addr) | 0x80)

/* bmAttributes in Endpoint Descriptor */
#define USB_ENDPOINT_TYPE_MASK                 0x03
#define USB_ENDPOINT_TYPE_CONTROL              0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS          0x01
#define USB_ENDPOINT_TYPE_BULK                 0x02
#define USB_ENDPOINT_TYPE_INTERRUPT            0x03
#define USB_ENDPOINT_SYNC_MASK                 0x0C
#define USB_ENDPOINT_SYNC_NO_SYNCHRONIZATION   0x00
#define USB_ENDPOINT_SYNC_ASYNCHRONOUS         0x04
#define USB_ENDPOINT_SYNC_ADAPTIVE             0x08
#define USB_ENDPOINT_SYNC_SYNCHRONOUS          0x0C
#define USB_ENDPOINT_USAGE_MASK                0x30
#define USB_ENDPOINT_USAGE_DATA                0x00
#define USB_ENDPOINT_USAGE_FEEDBACK            0x10
#define USB_ENDPOINT_USAGE_IMPLICIT_FEEDBACK   0x20
#define USB_ENDPOINT_USAGE_RESERVED            0x30

/* Control Endpoints */
#define EP0_I                                  USB_ENDPOINT_I(0)
#define EP0_O                                  USB_ENDPOINT_O(0)

/* USB Standard Device Descriptor */
typedef __packed struct _USB_DEVICE_DESCRIPTOR
{
  U8   bLength;
  U8   bDescriptorType;
  U16  bcdUSB;
  U8   bDeviceClass;
  U8   bDeviceSubClass;
  U8   bDeviceProtocol;
  U8   bMaxPacketSize0;
  U16  idVendor;
  U16  idProduct;
  U16  bcdDevice;
  U8   iManufacturer;
  U8   iProduct;
  U8   iSerialNumber;
  U8   bNumConfigurations;
} USB_DEVICE_DESCRIPTOR;

#define USB_DEVICE_DESCRIPTOR_SIZE (sizeof(USB_DEVICE_DESCRIPTOR))

/* USB 2.0 Device Qualifier Descriptor */
typedef __packed struct _USB_DEVICE_QUALIFIER_DESCRIPTOR
{
  U8   bLength;
  U8   bDescriptorType;
  U16  bcdUSB;
  U8   bDeviceClass;
  U8   bDeviceSubClass;
  U8   bDeviceProtocol;
  U8   bMaxPacketSize0;
  U8   bNumConfigurations;
  U8   bReserved;
} USB_DEVICE_QUALIFIER_DESCRIPTOR;

/* USB Standard Configuration Descriptor */
typedef __packed struct _USB_CONFIGURATION_DESCRIPTOR
{
  U8   bLength;
  U8   bDescriptorType;
  U16  wTotalLength;
  U8   bNumInterfaces;
  U8   bConfigurationValue;
  U8   iConfiguration;
  U8   bmAttributes;
  U8   bMaxPower;
} USB_CONFIGURATION_DESCRIPTOR;

#define USB_CONFIGURATION_DESCRIPTOR_SIZE (sizeof(USB_CONFIGURATION_DESCRIPTOR))

/* USB Standard Interface Descriptor */
typedef __packed struct _USB_INTERFACE_DESCRIPTOR
{
  U8   bLength;
  U8   bDescriptorType;
  U8   bInterfaceNumber;
  U8   bAlternateSetting;
  U8   bNumEndpoints;
  U8   bInterfaceClass;
  U8   bInterfaceSubClass;
  U8   bInterfaceProtocol;
  U8   iInterface;
} USB_INTERFACE_DESCRIPTOR;

#define USB_INTERFACE_DESCRIPTOR_SIZE (sizeof(USB_INTERFACE_DESCRIPTOR))

/* USB Standard Endpoint Descriptor */
typedef __packed struct _USB_ENDPOINT_DESCRIPTOR
{
  U8   bLength;
  U8   bDescriptorType;
  U8   bEndpointAddress;
  U8   bmAttributes;
  U16  wMaxPacketSize;
  U8   bInterval;
} USB_ENDPOINT_DESCRIPTOR;

#define USB_ENDPOINT_DESCRIPTOR_SIZE (sizeof(USB_ENDPOINT_DESCRIPTOR))

/* USB String Descriptor */
typedef __packed struct _USB_STRING_DESCRIPTOR
{
  U8   bLength;
  U8   bDescriptorType;
  U16  bString;
} USB_STRING_DESCRIPTOR;

/* USB Interface Association Descriptor */
typedef __packed struct _USB_INTERFACE_ASSOCIATION_DESCRIPTOR
{
  U8 bLength;           /* 8 Bytes */
  U8 bDescriptorType;
  U8 bFirstInterface;
  U8 bInterfaceCount;
  U8 bFunctionClass;
  U8 bFunctionSubClass;
  U8 bFunctionProtocol;
  U8 iFunction;
} USB_INTERFACE_ASSOCIATION_DESCRIPTOR;

#define USB_IF_ASSOCIATION_DESCRIPTOR_SIZE \
                                 (sizeof(USB_INTERFACE_ASSOCIATION_DESCRIPTOR))

/* USB Common Descriptor */
typedef __packed struct _USB_COMMON_DESCRIPTOR
{
  U8   bLength;
  U8   bDescriptorType;
} USB_COMMON_DESCRIPTOR;

#endif /* __USB_DEFINITIONS_H__ */