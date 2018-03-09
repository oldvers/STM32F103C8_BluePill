#include "types.h"
#include "usb_cfg.h"
//#include "msc.h"
#include "usb_defs.h"
#include "usb_descr.h"

//-----------------------------------------------------------------------------
/* USB Standard Device Descriptor */
static const U8 USB_DeviceDescriptor[] =
{
  USB_DEVICE_DESC_SIZE,              /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0110), /* 1.10 */          /* bcdUSB */
  0x00,                              /* bDeviceClass */
  0x00,                              /* bDeviceSubClass */
  0x00,                              /* bDeviceProtocol */
  USB_MAX_PACKET0,                   /* bMaxPacketSize0 */
  WBVAL(0xC251),                     /* idVendor */
  WBVAL(0x1C03),                     /* idProduct */
  WBVAL(0x0100), /* 1.00 */          /* bcdDevice */
  0x04,                              /* iManufacturer */
  0x20,                              /* iProduct */
  0x4A,                              /* iSerialNumber */
  0x01                               /* bNumConfigurations */
};

//-----------------------------------------------------------------------------
/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
static const U8 USB_ConfigDescriptor[] =
{
/* Configuration 1 */
  USB_CONFIGUARTION_DESC_SIZE,       /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL((                            /* wTotalLength */
    1*USB_CONFIGUARTION_DESC_SIZE +
    1*USB_INTERFACE_DESC_SIZE     +
    2*USB_ENDPOINT_DESC_SIZE
  )),
  0x01,                              /* bNumInterfaces */
  0x01,                              /* bConfigurationValue */
  0x00,                              /* iConfiguration */
  USB_CONFIG_BUS_POWERED /*|*/       /* bmAttributes */
/*USB_CONFIG_REMOTE_WAKEUP*/,
  USB_CONFIG_POWER_MA(100),          /* bMaxPower */
/* Interface 0, Alternate Setting 0, MSC Class */
  USB_INTERFACE_DESC_SIZE,           /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
  0x00,                              /* bInterfaceNumber */
  0x00,                              /* bAlternateSetting */
  0x02,                              /* bNumEndpoints */
  USB_DEVICE_CLASS_STORAGE,          /* bInterfaceClass */
  0x06, //MSC_SUBCLASS_SCSI,                 /* bInterfaceSubClass */
  0x50, //MSC_PROTOCOL_BULK_ONLY,            /* bInterfaceProtocol */
  0x64,                              /* iInterface */
/* Bulk In Endpoint */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_ENDPOINT_IN(1),                /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
  WBVAL(0x0040),                     /* wMaxPacketSize */
  0,                                 /* bInterval */
/* Bulk Out Endpoint */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_ENDPOINT_OUT(2),               /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
  WBVAL(0x0040),                     /* wMaxPacketSize */
  0,                                 /* bInterval */
/* Terminator */
  0                                  /* bLength */
};

//-----------------------------------------------------------------------------
/* USB String Descriptor (optional) */
static const U8 USB_StringDescriptor[] =
{
/* Index 0x00: LANGID Codes */
  0x04,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0409), /* US English */    /* wLANGID */
/* Index 0x04: Manufacturer */
  0x1C,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'K',0,
  'e',0,
  'i',0,
  'l',0,
  ' ',0,
  'S',0,
  'o',0,
  'f',0,
  't',0,
  'w',0,
  'a',0,
  'r',0,
  'e',0,
/* Index 0x20: Product */
  0x2A,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'K',0,
  'e',0,
  'i',0,
  'l',0,
  ' ',0,
  'M',0,
  'C',0,
  'B',0,
  'S',0,
  'T',0,
  'M',0,
  '3',0,
  '2',0,
  ' ',0,
  'M',0,
  'e',0,
  'm',0,
  'o',0,
  'r',0,
  'y',0,
/* Index 0x4A: Serial Number */
  0x1A,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'D',0,
  'E',0,
  'M',0,
  'O',0,
  ' ',0,
  '0',0,
  '1',0,
  '.',0,
  '0',0,
  '0',0,
  ' ',0,
  ' ',0,
/* Index 0x64: Interface 0, Alternate Setting 0 */
  0x0E,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'M',0,
  'e',0,
  'm',0,
  'o',0,
  'r',0,
  'y',0,
};

//-----------------------------------------------------------------------------
U8 *USB_GetDeviceDescriptor(void)
{
  return (U8 *)USB_DeviceDescriptor;
};

//-----------------------------------------------------------------------------
U8 *USB_GetConfigDescriptor(void)
{
  return (U8 *)USB_ConfigDescriptor;
};

//-----------------------------------------------------------------------------
U8 *USB_GetStringDescriptor(void)
{
  return (U8 *)USB_StringDescriptor;
}

//-----------------------------------------------------------------------------
U32 USB_GetItrfaceDescriptor(U8 aItrface, U8 aType, U8 *pData, U32 *pSize)
{
  U32 result = FALSE;

  switch (aType)
  {
#if USB_HID
    case HID_HID_DESCRIPTOR_TYPE:
      if (aInterface == USB_HID_IF_NUM)
      {
        pData = (U8 *)USB_ConfigDescriptor + HID_DESC_OFFSET;
        *pSize = HID_DESC_SIZE;
        result = TRUE;
      }
      break;
    case HID_REPORT_DESCRIPTOR_TYPE:
      if (aInterface == USB_HID_IF_NUM)
      {
        pData = (U8 *)HID_ReportDescriptor;
        *pSize = HID_ReportDescSize;
        result = TRUE;
      }
      break;
    case HID_PHYSICAL_DESCRIPTOR_TYPE:
      break;
#endif
    default:
      break;
  }

  return result;
}
