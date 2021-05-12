#include "types.h"
#include "usb_config.h"
#include "usb_definitions.h"
#include "usb_descriptor.h"
#include "usb_device.h"
#include "usb_icemkii_definitions.h"
#include "usb_icemkii.h"

#define WBVAL(x) (x & 0xFF),((x >> 8) & 0xFF)

typedef enum
{
  STR_DESCRIPTOR_IDX_LANG_ID = 0,   /* Language ID */
  STR_DESCRIPTOR_IDX_MANUFACTURER,  /* iManufacturer */
  STR_DESCRIPTOR_IDX_PRODUCT,       /* iProduct */
  STR_DESCRIPTOR_IDX_SERIAL_NUMBER, /* iSerialNumber */
  /* Strings count */
  STR_DESCRIPTOR_IDX_CNT
} STR_DESCRIPTOR_IDX;

typedef enum
{
  USB_INTERFACE_IDX_ICEMKII = 0,
  /* Interfaces count */
  USB_INTERFACE_IDX_CNT
} USB_INTERFACE_IDX;

typedef enum
{
  USB_ENDPOINT_IDX_CTRL = 0,
  USB_ENDPOINT_IDX_RESERVED,
  USB_ENDPOINT_IDX_ICEMKII,
  /* Endpoints count */
  USB_ENDPOINT_IDX_CNT
} USB_ENDPOINT_IDX;

/* -------------------------------------------------------------------------- */

U8 USBD_ICEMKII_IEndPointWrWsCb(USBD_CbByte pGetByteCb, U8 aSize)
{
  return USBD_EndPointWrWsCb
         (
           USB_ENDPOINT_I(USB_ENDPOINT_IDX_ICEMKII),
           pGetByteCb,
           aSize
         );
}

/* -------------------------------------------------------------------------- */

U8 USBD_ICEMKII_OEndPointRdWsCb(USBD_CbByte pPutByteCb, U8 aSize)
{
  return USBD_EndPointRdWsCb
         (
           USB_ENDPOINT_O(USB_ENDPOINT_IDX_ICEMKII),
           pPutByteCb,
           aSize
         );
}

/* -------------------------------------------------------------------------- */

/* USB Standard Device Descriptor */
static const U8 USB_DeviceDescriptor[] =
{
  USB_DEVICE_DESCRIPTOR_SIZE,            /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE,            /* bDescriptorType */
  WBVAL(0x0200), /* 2.00 */              /* bcdUSB */
  USB_DEVICE_CLASS_VENDOR_SPECIFIC,      /* bDeviceClass */
  0x00,                                  /* bDeviceSubClass */
  0x00,                                  /* bDeviceProtocol */
  USB_CTRL_PACKET_SIZE,                  /* bMaxPacketSize0 */
  WBVAL(0x03EB),                         /* idVendor */
  WBVAL(0x2103),                         /* idProduct */
  WBVAL(0x0200), /* 2.00 */              /* bcdDevice */
  STR_DESCRIPTOR_IDX_MANUFACTURER,       /* iManufacturer */
  STR_DESCRIPTOR_IDX_PRODUCT,            /* iProduct */
  STR_DESCRIPTOR_IDX_SERIAL_NUMBER,      /* iSerialNumber */
  0x01                                   /* bNumConfigurations */
};

/* -------------------------------------------------------------------------- */
/* USB Configuration Descriptor */
/* All Descriptors (Configuration, Interface, Endpoint, Class, Vendor) */
static const U8 USB_ConfigDescriptor[] =
{
/* Configuration 1 */
  USB_CONFIGURATION_DESCRIPTOR_SIZE,     /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE,     /* bDescriptorType */
  WBVAL((                                /* wTotalLength */
   USB_CONFIGURATION_DESCRIPTOR_SIZE  * (1)                              +
   USB_INTERFACE_DESCRIPTOR_SIZE      * (USB_INTERFACE_IDX_CNT)          +
   USB_ENDPOINT_DESCRIPTOR_SIZE       * (USB_ENDPOINT_IDX_CNT - 1)
  )),
  USB_INTERFACE_IDX_CNT,                 /* bNumInterfaces */
  0x01,                                  /* bConfigurationValue */
  0x00,                                  /* iConfiguration */
  USB_CONFIG_BUS_POWERED /*|*/           /* bmAttributes */
  /*USB_CONFIG_REMOTE_WAKEUP*/,
  USB_CONFIG_POWER_MA(400),              /* bMaxPower */
/* Interface 0, Alternate Setting 0, ICEMKII */
  USB_INTERFACE_DESCRIPTOR_SIZE,         /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,         /* bDescriptorType */
  USB_INTERFACE_IDX_ICEMKII,             /* bInterfaceNumber */
  0x00,                                  /* bAlternateSetting */
  0x02,                                  /* bNumEndpoints */
  USB_DEVICE_CLASS_VENDOR_SPECIFIC,      /* bInterfaceClass */
  0x00,                                  /* bInterfaceSubClass */
  0x00,                                  /* bInterfaceProtocol */
  0x00,                                  /* iInterface */
/* Bulk In Endpoint */
  USB_ENDPOINT_DESCRIPTOR_SIZE,          /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,          /* bDescriptorType */
  USB_ENDPOINT_I(USB_ENDPOINT_IDX_ICEMKII), /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,                /* bmAttributes */
  WBVAL(USB_ICEMKII_PACKET_SIZE),        /* wMaxPacketSize */
  USB_ICEMKII_BULK_INTERVAL,             /* bInterval */
/* Bulk Out Endpoint */
  USB_ENDPOINT_DESCRIPTOR_SIZE,          /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,          /* bDescriptorType */
  USB_ENDPOINT_O(USB_ENDPOINT_IDX_ICEMKII), /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,                /* bmAttributes */
  WBVAL(USB_ICEMKII_PACKET_SIZE),        /* wMaxPacketSize */
  USB_ICEMKII_BULK_INTERVAL,             /* bInterval */
/* Terminator */
  0                                      /* bTerminator */
};

/* -------------------------------------------------------------------------- */
/* USB String Descriptor (optional) */
static const U8 usbd_StrDescriptor_LanguageId[] =
{
  0x04,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  WBVAL(0x0409), /* US English */        /* wLANGID */
};

static const U8 usbd_StrDescriptor_Manufacturer[] =
{
  0x0C,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  'A',0,
  'T',0,
  'M',0,
  'E',0,
  'L',0,
};

static const U8 usbd_StrDescriptor_Product[] =
{
  0x1A,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
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

static const U8 usbd_StrDescriptor_SerialNumber[] =
{
  0x1A,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
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

static const U8 * usbd_StrDescriptor[STR_DESCRIPTOR_IDX_CNT] =
{
  usbd_StrDescriptor_LanguageId,
  usbd_StrDescriptor_Manufacturer,
  usbd_StrDescriptor_Product,
  usbd_StrDescriptor_SerialNumber,
};

/* -------------------------------------------------------------------------- */

U8 *USBD_GetDeviceDescriptor(void)
{
  return (U8 *)USB_DeviceDescriptor;
};

/* -------------------------------------------------------------------------- */

U8 *USBD_GetConfigDescriptor(void)
{
  return (U8 *)USB_ConfigDescriptor;
};

/* -------------------------------------------------------------------------- */

U8 *USBD_GetStringDescriptor(U8 aIndex)
{
  if (STR_DESCRIPTOR_IDX_CNT > aIndex)
  {
    return (U8 *)usbd_StrDescriptor[aIndex];
  }
  else
  {
    return NULL;
  }
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN USBD_GetItrfaceDescriptor
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
)
{
  FW_BOOLEAN result = FW_FALSE;

  return result;
}

/* -------------------------------------------------------------------------- */

U8 USBD_GetItrfacesCount(void)
{
  return USB_INTERFACE_IDX_CNT;
}

/* --- Interfaces Callbacks Descriptor -------------------------------------- */

const USBD_INTERFACE_CALLBACKS_DESCRIPTOR
      USBD_IfCbDescriptor[USB_INTERFACE_IDX_CNT] =
{
  [USB_INTERFACE_IDX_ICEMKII] =
  {
    .CbInit      = ICEMKII_Init,
    .CbCtrlSetup = ICEMKII_CtrlSetupReq,
    .CbCtrlOut   = ICEMKII_CtrlOutReq,
    .CbSOF       = NULL,
    .CbEndPointI = ICEMKII_BulkIn,
    .CbEndPointO = ICEMKII_BulkOut,
    .EndPointI   = USB_ENDPOINT_I(USB_ENDPOINT_IDX_ICEMKII),
    .EndPointO   = USB_ENDPOINT_O(USB_ENDPOINT_IDX_ICEMKII),
  },
};

/* -------------------------------------------------------------------------- */
