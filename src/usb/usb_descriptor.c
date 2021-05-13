#include "types.h"
#include "usb_config.h"
#include "usb_definitions.h"
#include "usb_descriptor.h"
#include "usb_device.h"
#include "usb_cdc_definitions.h"

#include "cdc.h"

#define WBVAL(x) (x & 0xFF),((x >> 8) & 0xFF)

typedef enum
{
  STR_DESCRIPTOR_IDX_LANG_ID = 0,   /* Language ID */
  STR_DESCRIPTOR_IDX_MANUFACTURER,  /* iManufacturer */
  STR_DESCRIPTOR_IDX_PRODUCT,       /* iProduct */
  STR_DESCRIPTOR_IDX_CDC,
  STR_DESCRIPTOR_IDX_CDD,
  STR_DESCRIPTOR_IDX_SERIAL_NUMBER, /* iSerialNumber */
  /* Strings count */
  STR_DESCRIPTOR_IDX_CNT
} STR_DESCRIPTOR_IDX;

typedef enum
{
  USB_INTERFACE_IDX_CDC = 0,
  USB_INTERFACE_IDX_CDD,
  /* Interfaces count */
  USB_INTERFACE_IDX_CNT
} USB_INTERFACE_IDX;

typedef enum
{
  USB_ENDPOINT_IDX_CTRL = 0,
  USB_ENDPOINT_IDX_CDC,
  USB_ENDPOINT_IDX_CDD,
  /* Endpoints count */
  USB_ENDPOINT_IDX_CNT
} USB_ENDPOINT_IDX;

/* -------------------------------------------------------------------------- */

/* USB Standard Device Descriptor */
static const U8 USB_DeviceDescriptor[] =
{
  USB_DEVICE_DESCRIPTOR_SIZE,            /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE,            /* bDescriptorType */
  WBVAL(0x0200), /* 2.00 */              /* bcdUSB */
  USB_DEVICE_CLASS_RESERVED,             /* bDeviceClass */
  0x00,                                  /* bDeviceSubClass */
  0x00,                                  /* bDeviceProtocol */
  USB_CTRL_PACKET_SIZE,                  /* bMaxPacketSize0 */
  WBVAL(0x10C4),                         /* idVendor */
  WBVAL(0xEA70),                         /* idProduct */
  WBVAL(0x0100), /* 1.00 */              /* bcdDevice */
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
   USB_ENDPOINT_DESCRIPTOR_SIZE       * (4)
  )),
  USB_INTERFACE_IDX_CNT,                 /* bNumInterfaces */
  0x01,                                  /* bConfigurationValue */
  0x00,                                  /* iConfiguration */
  USB_CONFIG_BUS_POWERED /*|*/           /* bmAttributes */
  /*USB_CONFIG_REMOTE_WAKEUP*/,
  USB_CONFIG_POWER_MA(500),              /* bMaxPower */
/* Interface 0, Alternate Setting 0, Vendor Specific Class */
  USB_INTERFACE_DESCRIPTOR_SIZE,         /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,         /* bDescriptorType */
  USB_INTERFACE_IDX_CDC,                 /* bInterfaceNumber */
  0x00,                                  /* bAlternateSetting */
  0x02,                                  /* bNumEndpoints */
  USB_DEVICE_CLASS_VENDOR_SPECIFIC,      /* bInterfaceClass */
  CDC_IF_SUBCLASS_NONE,                  /* bInterfaceSubClass */
  CDC_IF_PROTOCOL_NONE,                  /* bInterfaceProtocol */
  STR_DESCRIPTOR_IDX_CDC,                /* iInterface */
/* Bulk In Endpoint */
  USB_ENDPOINT_DESCRIPTOR_SIZE,          /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,          /* bDescriptorType */
  USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDC),  /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,                /* bmAttributes */
  WBVAL(USB_CDC_PACKET_SIZE),            /* wMaxPacketSize */
  0,                                     /* bInterval */
/* Bulk Out Endpoint */
  USB_ENDPOINT_DESCRIPTOR_SIZE,          /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,          /* bDescriptorType */
  USB_ENDPOINT_O(USB_ENDPOINT_IDX_CDC),  /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,                /* bmAttributes */
  WBVAL(USB_CDC_PACKET_SIZE),            /* wMaxPacketSize */
  0,                                     /* bInterval */
/* Interface 1, Alternate Setting 0, Vendor Specific Class */
  USB_INTERFACE_DESCRIPTOR_SIZE,         /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,         /* bDescriptorType */
  USB_INTERFACE_IDX_CDD,                 /* bInterfaceNumber */
  0x00,                                  /* bAlternateSetting */
  0x02,                                  /* bNumEndpoints */
  USB_DEVICE_CLASS_VENDOR_SPECIFIC,      /* bInterfaceClass */
  CDC_IF_SUBCLASS_NONE,                  /* bInterfaceSubClass */
  CDC_IF_PROTOCOL_NONE,                  /* bInterfaceProtocol */
  STR_DESCRIPTOR_IDX_CDD,                /* iInterface */
/* Bulk In Endpoint */
  USB_ENDPOINT_DESCRIPTOR_SIZE,          /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,          /* bDescriptorType */
  USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDD),  /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,                /* bmAttributes */
  WBVAL(USB_CDC_PACKET_SIZE),            /* wMaxPacketSize */
  0,                                     /* bInterval */
/* Bulk Out Endpoint */
  USB_ENDPOINT_DESCRIPTOR_SIZE,          /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,          /* bDescriptorType */
  USB_ENDPOINT_O(USB_ENDPOINT_IDX_CDD),  /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,                /* bmAttributes */
  WBVAL(USB_CDC_PACKET_SIZE),            /* wMaxPacketSize */
  0,                                     /* bInterval */
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
  0x1A,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  'S',0,
  'i',0,
  'l',0,
  'i',0,
  'c',0,
  'o',0,
  'n',0,
  ' ',0,
  'L',0,
  'a',0,
  'b',0,
  's',0,
};

static const U8 usbd_StrDescriptor_Product[] =
{
  0x54,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  'D',0,
  'u',0,
  'a',0,
  'l',0,
  ' ',0,
  'C',0,
  'P',0,
  '2',0,
  '1',0,
  '0',0,
  '5',0,
  ' ',0,
  'U',0,
  'S',0,
  'B',0,
  ' ',0,
  't',0,
  'o',0,
  ' ',0,
  'U',0,
  'A',0,
  'R',0,
  'T',0,
  ' ',0,
  'B',0,
  'r',0,
  'i',0,
  'd',0,
  'g',0,
  'e',0,
  ' ',0,
  'C',0,
  'o',0,
  'n',0,
  't',0,
  'r',0,
  'o',0,
  'l',0,
  'l',0,
  'e',0,
  'r',0,
};

static const U8 usbd_StrDescriptor_SerialNumber[] =
{
  0x0A,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  '0',0,
  '0',0,
  '1',0,
  '3',0,
};

static const U8 usbd_StrDescriptor_CDC[] =
{
  0x24,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  'E',0,
  'n',0,
  'h',0,
  'a',0,
  'n',0,
  'c',0,
  'e',0,
  'd',0,
  ' ',0,
  'C',0,
  'O',0,
  'M',0,
  ' ',0,
  'P',0,
  'o',0,
  'r',0,
  't',0,
};

static const U8 usbd_StrDescriptor_CDD[] =
{
  0x24,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  'S',0,
  't',0,
  'a',0,
  'n',0,
  'd',0,
  'a',0,
  'r',0,
  'd',0,
  ' ',0,
  'C',0,
  'O',0,
  'M',0,
  ' ',0,
  'P',0,
  'o',0,
  'r',0,
  't',0,
};

static const U8 * usbd_StrDescriptor[STR_DESCRIPTOR_IDX_CNT] =
{
  usbd_StrDescriptor_LanguageId,
  usbd_StrDescriptor_Manufacturer,
  usbd_StrDescriptor_Product,
  usbd_StrDescriptor_CDC,
  usbd_StrDescriptor_CDD,
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
  [USB_INTERFACE_IDX_CDC] =
  {
    .CbInit      = CDC_Init,
    .CbCtrlSetup = CDC_CtrlSetupReq,
    .CbCtrlOut   = CDC_CtrlOutReq,
    .CbSOF       = CDC_SOF,
    .CbEndPointI = CDC_BulkAIn,
    .CbEndPointO = CDC_BulkAOut,
    .EndPointI   = USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDC),
    .EndPointO   = USB_ENDPOINT_O(USB_ENDPOINT_IDX_CDC),
  },
  [USB_INTERFACE_IDX_CDD] =
  {
    .CbInit      = NULL,
    .CbCtrlSetup = CDC_CtrlSetupReq,
    .CbCtrlOut   = CDC_CtrlOutReq,
    .CbSOF       = NULL,
    .CbEndPointI = CDC_BulkBIn,
    .CbEndPointO = CDC_BulkBOut,
    .EndPointI   = USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDD),
    .EndPointO   = USB_ENDPOINT_O(USB_ENDPOINT_IDX_CDD),
  },
};

/* -------------------------------------------------------------------------- */

U8 USBD_CDC_GetInterfaceNumber(void)
{
  return USB_INTERFACE_IDX_CDC;
}

/* -------------------------------------------------------------------------- */

U8 USBD_CDD_GetInterfaceNumber(void)
{
  return USB_INTERFACE_IDX_CDD;
}

/* -------------------------------------------------------------------------- */

U32 USBD_CDC_IEndPointWrWsCb(USBD_CbByte pGetByteCb, U32 aSize)
{
  return USBD_EP_WrWsCb
         (
           USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDC),
           pGetByteCb,
           aSize
         );
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN USBD_CDC_IEndPointIsTxEmpty(void)
{
  return USBD_EP_IsTxEmpty(USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDC));
}

/* -------------------------------------------------------------------------- */

U32 USBD_CDC_OEndPointRdWsCb(USBD_CbByte pPutByteCb, U32 aSize)
{
  return USBD_EP_RdWsCb
         (
           USB_ENDPOINT_O(USB_ENDPOINT_IDX_CDC),
           pPutByteCb,
           aSize
         );
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN USBD_CDC_OEndPointIsRxEmpty(void)
{
  return USBD_EP_IsRxEmpty(USB_ENDPOINT_O(USB_ENDPOINT_IDX_CDC));
}

/* -------------------------------------------------------------------------- */

U32 USBD_CDD_IEndPointWrWsCb(USBD_CbByte pGetByteCb, U32 aSize)
{
  return USBD_EP_WrWsCb
         (
           USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDD),
           pGetByteCb,
           aSize
         );
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN USBD_CDD_IEndPointIsTxEmpty(void)
{
  return USBD_EP_IsTxEmpty(USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDD));
}

/* -------------------------------------------------------------------------- */
U32 USBD_CDD_OEndPointRdWsCb(USBD_CbByte pPutByteCb, U32 aSize)
{
  return USBD_EP_RdWsCb
         (
           USB_ENDPOINT_O(USB_ENDPOINT_IDX_CDD),
           pPutByteCb,
           aSize
         );
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN USBD_CDD_OEndPointIsRxEmpty(void)
{
  return USBD_EP_IsRxEmpty(USB_ENDPOINT_O(USB_ENDPOINT_IDX_CDD));
}

/* -------------------------------------------------------------------------- */

