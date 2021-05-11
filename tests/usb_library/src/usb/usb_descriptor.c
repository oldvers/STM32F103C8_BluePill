//#include "types.h"
//#include "usb_cfg.h"
//#include "usb_defs.h"
//#include "usb_descr.h"
//#include "msc_defs.h"
//#include "cdc_defs.h"
//#include "hid_defs.h"
//
//#include "debug.h"
//
////-----------------------------------------------------------------------------
//
//#define STR_DESC_IDX_LANG_ID        (0)                     /* Language ID */
//#define STR_DESC_IDX_MANUFACTURER   (1)                     /* iManufacturer */
//#define STR_DESC_IDX_PRODUCT        (2)                     /* iProduct */
//#define STR_DESC_IDX_CDC            (STR_DESC_IDX_PRODUCT + USB_CDC)
//#define STR_DESC_IDX_CDD            (STR_DESC_IDX_CDC + USB_CDD)
//#define STR_DESC_IDX_SERIAL_NUMBER  (STR_DESC_IDX_CDD + 1)  /* iSerialNumber */
//#define STR_DESC_IDX_CNT            (STR_DESC_IDX_SERIAL_NUMBER + 1)
//
////-----------------------------------------------------------------------------
///* USB Standard Device Descriptor */
//static const U8 USB_DeviceDescriptor[] =
//{
//  USB_DEVICE_DESC_SIZE,              /* bLength */
//  USB_DEVICE_DESCRIPTOR_TYPE,        /* bDescriptorType */
//  WBVAL(0x0200), /* 2.00 */          /* bcdUSB */
//  USB_DEVICE_CLASS_RESERVED,         /* bDeviceClass */
//  0x00,                              /* bDeviceSubClass */
//  0x00,                              /* bDeviceProtocol */
//  USB_CTRL_PACKET_SIZE,              /* bMaxPacketSize0 */
//  WBVAL(0x10C4),                     /* idVendor */
//  WBVAL(0xEA70),                     /* idProduct */
//  WBVAL(0x0100), /* 1.00 */          /* bcdDevice */
//  STR_DESC_IDX_MANUFACTURER,         /* iManufacturer */
//  STR_DESC_IDX_PRODUCT,              /* iProduct */
//  STR_DESC_IDX_SERIAL_NUMBER,        /* iSerialNumber */
//  0x01                               /* bNumConfigurations */
//};
//
////-----------------------------------------------------------------------------
///* USB Configuration Descriptor */
///*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
////static
//const U8 USB_ConfigDescriptor[] =
//{
///* Configuration 1 */
//  USB_CONFIGUARTION_DESC_SIZE,       /* bLength */
//  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType */
//  WBVAL((                            /* wTotalLength */
//   USB_CONFIGUARTION_DESC_SIZE * (1)                                         +
//   USB_INTERFACE_DESC_SIZE     * (USB_IF_CNT)                                +
//   USB_ENDPOINT_DESC_SIZE      * (USB_EP_CNT - USB_CTRL_EP_CNT)
//  )),
//  USB_IF_CNT,                        /* bNumInterfaces */
//  0x01,                              /* bConfigurationValue */
//  0x00,                              /* iConfiguration */
//  USB_CONFIG_BUS_POWERED /*|*/       /* bmAttributes */
//  /*USB_CONFIG_REMOTE_WAKEUP*/,
//  USB_CONFIG_POWER_MA(500),          /* bMaxPower */
//
//#if (USB_CDC)
///* Interface 0, Alternate Setting 0, Vendor Specific Class */
//  USB_INTERFACE_DESC_SIZE,           /* bLength */
//  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
//  USB_CDC_IF_NUM,                    /* bInterfaceNumber */
//  0x00,                              /* bAlternateSetting */
//  USB_CDC_EP_CNT,                    /* bNumEndpoints */
//  USB_DEVICE_CLASS_VENDOR_SPECIFIC,  /* bInterfaceClass */
//  CDC_IF_SUBCLASS_NONE,              /* bInterfaceSubClass */
//  CDC_IF_PROTOCOL_NONE,              /* bInterfaceProtocol */
//  STR_DESC_IDX_CDC,                  /* iInterface */
///* Bulk In Endpoint */
//  USB_ENDPOINT_DESC_SIZE,            /* bLength */
//  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
//  USB_CDC_EP_BLK_I,                  /* bEndpointAddress */
//  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
//  WBVAL(USB_CDC_PACKET_SIZE),        /* wMaxPacketSize */
//  0,                                 /* bInterval */
///* Bulk Out Endpoint */
//  USB_ENDPOINT_DESC_SIZE,            /* bLength */
//  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
//  USB_CDC_EP_BLK_O,                  /* bEndpointAddress */
//  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
//  WBVAL(USB_CDC_PACKET_SIZE),        /* wMaxPacketSize */
//  0,                                 /* bInterval */
//#endif
//
//#if (USB_CDD)
//  /* Interface x, Alternate Setting 0, Vendor Specific Class */
//  USB_INTERFACE_DESC_SIZE,           /* bLength */
//  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
//  USB_CDD_IF_NUM,                    /* bInterfaceNumber */
//  0x00,                              /* bAlternateSetting */
//  USB_CDD_EP_CNT,                    /* bNumEndpoints */
//  USB_DEVICE_CLASS_VENDOR_SPECIFIC,  /* bInterfaceClass */
//  CDC_IF_SUBCLASS_NONE,              /* bInterfaceSubClass */
//  CDC_IF_PROTOCOL_NONE,              /* bInterfaceProtocol */
//  STR_DESC_IDX_CDD,                  /* iInterface */
///* Bulk In Endpoint */
//  USB_ENDPOINT_DESC_SIZE,            /* bLength */
//  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
//  USB_CDD_EP_BLK_I,                  /* bEndpointAddress */
//  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
//  WBVAL(USB_CDC_PACKET_SIZE),        /* wMaxPacketSize */
//  0,                                 /* bInterval */
///* Bulk Out Endpoint */
//  USB_ENDPOINT_DESC_SIZE,            /* bLength */
//  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
//  USB_CDD_EP_BLK_O,                  /* bEndpointAddress */
//  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
//  WBVAL(USB_CDC_PACKET_SIZE),        /* wMaxPacketSize */
//  0,                                 /* bInterval */
//#endif
//
///* Terminator */
//  0                                  /* bLength */
//};
//
////-----------------------------------------------------------------------------
///* USB String Descriptor (optional) */
//static const U8 USB_StrDescLangId[] =
//{
//  0x04,                              /* bLength */
//  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
//  WBVAL(0x0409), /* US English */    /* wLANGID */
//};
//
//static const U8 USB_StrDescManufacturer[] =
//{
//  0x1A,                              /* bLength */
//  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
//  'S',0,
//  'i',0,
//  'l',0,
//  'i',0,
//  'c',0,
//  'o',0,
//  'n',0,
//  ' ',0,
//  'L',0,
//  'a',0,
//  'b',0,
//  's',0,
//};
//
//static const U8 USB_StrDescProduct[] =
//{
//  0x54,                              /* bLength */
//  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
//  'D',0,
//  'u',0,
//  'a',0,
//  'l',0,
//  ' ',0,
//  'C',0,
//  'P',0,
//  '2',0,
//  '1',0,
//  '0',0,
//  '5',0,
//  ' ',0,
//  'U',0,
//  'S',0,
//  'B',0,
//  ' ',0,
//  't',0,
//  'o',0,
//  ' ',0,
//  'U',0,
//  'A',0,
//  'R',0,
//  'T',0,
//  ' ',0,
//  'B',0,
//  'r',0,
//  'i',0,
//  'd',0,
//  'g',0,
//  'e',0,
//  ' ',0,
//  'C',0,
//  'o',0,
//  'n',0,
//  't',0,
//  'r',0,
//  'o',0,
//  'l',0,
//  'l',0,
//  'e',0,
//  'r',0,
//};
//
//static const U8 USB_StrDescSerialNumber[] =
//{
//  0x0A,                              /* bLength */
//  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
//  '0',0,
//  '0',0,
//  '1',0,
//  '3',0,
//};
//
//#if (USB_CDC)
//static const U8 USB_StrDescCDC[] =
//{
//  0x24,                              /* bLength */
//  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
//  'E',0,
//  'n',0,
//  'h',0,
//  'a',0,
//  'n',0,
//  'c',0,
//  'e',0,
//  'd',0,
//  ' ',0,
//  'C',0,
//  'O',0,
//  'M',0,
//  ' ',0,
//  'P',0,
//  'o',0,
//  'r',0,
//  't',0,
//};
//#endif
//
//#if (USB_CDD)
//static const U8 USB_StrDescCDD[] =
//{
//  0x24,                              /* bLength */
//  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
//  'S',0,
//  't',0,
//  'a',0,
//  'n',0,
//  'd',0,
//  'a',0,
//  'r',0,
//  'd',0,
//  ' ',0,
//  'C',0,
//  'O',0,
//  'M',0,
//  ' ',0,
//  'P',0,
//  'o',0,
//  'r',0,
//  't',0,
//};
//#endif
//
//static const U8 * USB_StringDescriptor[STR_DESC_IDX_CNT] =
//{
//  USB_StrDescLangId,
//  USB_StrDescManufacturer,
//  USB_StrDescProduct,
//#if (USB_CDC)
//  USB_StrDescCDC,
//#endif
//#if (USB_CDD)
//  USB_StrDescCDD,
//#endif
//  USB_StrDescSerialNumber,
//};
//
////-----------------------------------------------------------------------------
//U8 *USB_GetDeviceDescriptor(void)
//{
//  return (U8 *)USB_DeviceDescriptor;
//};
//
////-----------------------------------------------------------------------------
//U8 *USB_GetConfigDescriptor(void)
//{
//  return (U8 *)USB_ConfigDescriptor;
//};
//
////-----------------------------------------------------------------------------
//U8 *USB_GetStringDescriptor(U8 aIndex)
//{
//  return (U8 *)USB_StringDescriptor[aIndex];
//}
//
////-----------------------------------------------------------------------------
//FW_BOOLEAN USB_GetItrfaceDescriptor
//(
//    USB_SETUP_PACKET * pSetup,
//    U8 **pData,
//    U16 *pSize
//)
//{
//  FW_BOOLEAN result = FW_FALSE;
//
//  return result;
//}
