#include "types.h"
#include "usb_cfg.h"
#include "usb_defs.h"
#include "usb_descr.h"
#include "usb_icemkii_defs.h"

#include "debug.h"

/* ------------------------------------------------------------------------- */

#define STR_DESC_IDX_LANG_ID        (0)  /* Language ID */
#define STR_DESC_IDX_MANUFACTURER   (1)  /* iManufacturer */
#define STR_DESC_IDX_PRODUCT        (2)  /* iProduct */
#define STR_DESC_IDX_SERIAL_NUMBER  (3)  /* iSerialNumber */
#define STR_DESC_IDX_CNT            (STR_DESC_IDX_SERIAL_NUMBER + USB_ICEMKII)

/* ------------------------------------------------------------------------- */
/* USB Standard Device Descriptor */
static const U8 USB_DeviceDescriptor[] =
{
  USB_DEVICE_DESC_SIZE,              /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0200), /* 2.00 */          /* bcdUSB */
  USB_DEVICE_CLASS_VENDOR_SPECIFIC,  /* bDeviceClass */
  0x00,                              /* bDeviceSubClass */
  0x00,                              /* bDeviceProtocol */
  USB_CTRL_PACKET_SIZE,              /* bMaxPacketSize0 */
  WBVAL(0x03EB),                     /* idVendor */
  WBVAL(0x2103),                     /* idProduct */
  WBVAL(0x0200), /* 2.00 */          /* bcdDevice */
  STR_DESC_IDX_MANUFACTURER,         /* iManufacturer */
  STR_DESC_IDX_PRODUCT,              /* iProduct */
  STR_DESC_IDX_SERIAL_NUMBER,        /* iSerialNumber */
  0x01                               /* bNumConfigurations */
};

/* ------------------------------------------------------------------------- */
/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
static const U8 USB_ConfigDescriptor[] =
{
/* Configuration 1 */
  USB_CONFIGUARTION_DESC_SIZE,       /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL((                            /* wTotalLength */
   USB_CONFIGUARTION_DESC_SIZE * (1)           +
   USB_INTERFACE_DESC_SIZE     * (USB_IF_CNT)  +
   USB_ENDPOINT_DESC_SIZE      * (USB_EP_CNT - 1)
  )),
  USB_IF_CNT,                        /* bNumInterfaces */
  0x01,                              /* bConfigurationValue */
  0x00,                              /* iConfiguration */
  USB_CONFIG_BUS_POWERED,            /* bmAttributes */
  USB_CONFIG_POWER_MA(400),          /* bMaxPower */

/* Interface 0, Alternate Setting 0, ICEMKII */
  USB_INTERFACE_DESC_SIZE,           /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
  USB_ICEMKII_IF_NUM,                /* bInterfaceNumber */
  0x00,                              /* bAlternateSetting */
  USB_ICEMKII_EP_CNT,                /* bNumEndpoints */
  USB_DEVICE_CLASS_VENDOR_SPECIFIC,  /* bInterfaceClass */
  0x00,                              /* bInterfaceSubClass */
  0x00,                              /* bInterfaceProtocol */
  0x00,                              /* iInterface */
/* Bulk In Endpoint */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_ICEMKII_EP_BULK_IN,            /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
  WBVAL(USB_ICEMKII_PACKET_SIZE),    /* wMaxPacketSize */
  10,                                /* bInterval */
/* Bulk Out Endpoint */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_ICEMKII_EP_BULK_OUT,           /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
  WBVAL(USB_ICEMKII_PACKET_SIZE),    /* wMaxPacketSize */
  10,                                /* bInterval */

/* Terminator */
  0                                  /* bLength */
};

/* ------------------------------------------------------------------------- */
/* USB String Descriptor (optional) */
static const U8 USB_StrDescLangId[] =
{
  0x04,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0409), /* US English */    /* wLANGID */
};

static const U8 USB_StrDescManufacturer[] =
{
  0x0C,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'A',0,
  'T',0,
  'M',0,
  'E',0,
  'L',0,
};

static const U8 USB_StrDescProduct[] =
{
  0x1A,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'J',0,
  'T',0,
  'A',0,
  'G',0,
  'I',0,
  'C',0,
  'E',0,
  ' ',0,
  'm',0,
  'k',0,
  'I',0,
  'I',0,
};

static const U8 USB_StrDescSerialNumber[] =
{
  0x1A,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'S',0,
  'T',0,
  'M',0,
  '3',0,
  '2',0,
  '1',0,
  '.',0,
  '0',0,
  '0',0,
  '.',0,
  '0',0,
  '0',0,
};

static const U8 * USB_StringDescriptor[STR_DESC_IDX_CNT] =
{
  USB_StrDescLangId,
  USB_StrDescManufacturer,
  USB_StrDescProduct,
  USB_StrDescSerialNumber,
};

/* ------------------------------------------------------------------------- */

U8 *USB_GetDeviceDescriptor(void)
{
  return (U8 *)USB_DeviceDescriptor;
};

/* ------------------------------------------------------------------------- */

U8 *USB_GetConfigDescriptor(void)
{
  return (U8 *)USB_ConfigDescriptor;
};

/* ------------------------------------------------------------------------- */

U8 *USB_GetStringDescriptor(U8 aIndex)
{
  return (U8 *)USB_StringDescriptor[aIndex];
}

/* ------------------------------------------------------------------------- */

U32 USB_GetItrfaceDescriptor(USB_SETUP_PACKET * pSetup, U8 **pData, U16 *pSize)
{
  U32 result = FALSE;

  return result;
}

/* ------------------------------------------------------------------------- */
