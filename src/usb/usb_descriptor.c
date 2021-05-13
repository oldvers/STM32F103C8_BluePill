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
  STR_DESCRIPTOR_IDX_SERIAL_NUMBER, /* iSerialNumber */
  STR_DESCRIPTOR_IDX_CDC,
  /* Strings count */
  STR_DESCRIPTOR_IDX_CNT
} STR_DESCRIPTOR_IDX;

typedef enum
{
  USB_INTERFACE_IDX_CDC_IRQ = 0,
  USB_INTERFACE_IDX_CDC_DATA,
  /* Interfaces count */
  USB_INTERFACE_IDX_CNT
} USB_INTERFACE_IDX;

typedef enum
{
  USB_ENDPOINT_IDX_CTRL = 0,
  USB_ENDPOINT_IDX_CDC_IRQ,
  USB_ENDPOINT_IDX_CDC_DATA,
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
  USB_DEVICE_CLASS_COMMUNICATIONS,       /* bDeviceClass */
  CDC_IF_SUBCLASS_ACM,                   /* bDeviceSubClass */
  CDC_IF_PROTOCOL_AT_CMD,                /* bDeviceProtocol */
  USB_CTRL_PACKET_SIZE,                  /* bMaxPacketSize0 */
  WBVAL(0xC251),                         /* idVendor */
  WBVAL(0xA004),                         /* idProduct */
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
   USB_ENDPOINT_DESCRIPTOR_SIZE       * (3)                              +
   CDC_FNC_DESC_SUM_SIZE              * (1)
  )),
  USB_INTERFACE_IDX_CNT,                 /* bNumInterfaces */
  0x01,                                  /* bConfigurationValue */
  0x00,                                  /* iConfiguration */
  USB_CONFIG_BUS_POWERED /*|*/           /* bmAttributes */
  /*USB_CONFIG_REMOTE_WAKEUP*/,
  USB_CONFIG_POWER_MA(100),              /* bMaxPower */
/* Interface 0, Alternate Setting 0, CDC Class */
  USB_INTERFACE_DESCRIPTOR_SIZE,         /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,         /* bDescriptorType */
  USB_INTERFACE_IDX_CDC_IRQ,             /* bInterfaceNumber */
  0x00,                                  /* bAlternateSetting */
  0x01,                                  /* bNumEndpoints */
  USB_DEVICE_CLASS_COMMUNICATIONS,       /* bInterfaceClass */
  CDC_IF_SUBCLASS_ACM,                   /* bInterfaceSubClass */
  CDC_IF_PROTOCOL_AT_CMD,                /* bInterfaceProtocol */
  STR_DESCRIPTOR_IDX_CDC,                /* iInterface */
/* Header Functional Descriptor */
  0x05,                                  /* bLength */
  CDC_CS_INTERFACE_DESC_TYPE,            /* bDescriptorType */
  CDC_HEADER_FNC_DESC_SUBTYPE,           /* bDescriptorSubtype */
  WBVAL(0x0110),                         /* bcdCDC: Spec Release Number */
/* Call Management Functional Descriptor */
  0x05,                                  /* bFunctionLength */
  CDC_CS_INTERFACE_DESC_TYPE,            /* bDescriptorType */
  CDC_CALL_MGMT_FNC_DESC_SUBTYPE,        /* bDescriptorSubtype */
  0x00,                                  /* bmCapabilities: D0+D1 */
  0x01,                                  /* bDataInterface */
/* ACM Functional Descriptor */
  0x04,                                  /* bFunctionLength */
  CDC_CS_INTERFACE_DESC_TYPE,            /* bDescriptorType */
  CDC_ACM_FNC_DESC_SUBTYPE,              /* bDescriptorSubtype */
  0x02,                                  /* bmCapabilities */
/* Union Functional Descriptor */
  0x05,                                  /* bFunctionLength */
  CDC_CS_INTERFACE_DESC_TYPE,            /* bDescriptorType */
  CDC_UNION_FNC_DESC_SUBTYPE,            /* bDescriptorSubtype */
  USB_INTERFACE_IDX_CDC_IRQ,             /* bMasterInterface */
  USB_INTERFACE_IDX_CDC_DATA,            /* bSlaveInterface0 */
/* Endpoint Interrupt Descriptor */
  USB_ENDPOINT_DESCRIPTOR_SIZE,          /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,          /* bDescriptorType */
  USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDC_IRQ), /* bEndpointAddress */
  USB_ENDPOINT_TYPE_INTERRUPT,           /* bmAttributes */
  WBVAL(USB_CDC_IRQ_PACKET_SIZE),        /* wMaxPacketSize */
  USB_CDC_IRQ_INTERVAL,                  /* bInterval */
/* Interface 1, Data Class Interface Descriptor */
  USB_INTERFACE_DESCRIPTOR_SIZE,         /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,         /* bDescriptorType */
  USB_INTERFACE_IDX_CDC_DATA,            /* bInterfaceNumber */
  0x00,                                  /* bAlternateSetting */
  0x02,                                  /* bNumEndpoints */
  CDC_IF_CLASS_CDC,                      /* bInterfaceClass */
  CDC_IF_SUBCLASS_NONE,                  /* bInterfaceSubClass */
  CDC_IF_PROTOCOL_NONE,                  /* bInterfaceProtocol */
  0,                                     /* iInterface */
/* Bulk Out Endpoint */
  USB_ENDPOINT_DESCRIPTOR_SIZE,          /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,          /* bDescriptorType */
  USB_ENDPOINT_O(USB_ENDPOINT_IDX_CDC_DATA), /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,                /* bmAttributes */
  WBVAL(USB_CDC_PACKET_SIZE),            /* wMaxPacketSize */
  0,                                     /* bInterval */
/* Bulk In Endpoint */
  USB_ENDPOINT_DESCRIPTOR_SIZE,          /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,          /* bDescriptorType */
  USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDC_DATA), /* bEndpointAddress */
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
  0x1C,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  'K', 0,
  'e', 0,
  'i', 0,
  'l', 0,
  ' ', 0,
  'S', 0,
  'o', 0,
  'f', 0,
  't', 0,
  'w', 0,
  'a', 0,
  'r', 0,
  'e', 0,
};

static const U8 usbd_StrDescriptor_Product[] =
{
  0x34,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  'K', 0,
  'e', 0,
  'i', 0,
  'l', 0,
  ' ', 0,
  'C', 0,
  'o', 0,
  'm', 0,
  'm', 0,
  'u', 0,
  'n', 0,
  'i', 0,
  'c', 0,
  'a', 0,
  't', 0,
  'i', 0,
  'o', 0,
  'n', 0,
  ' ', 0,
  'D', 0,
  'e', 0,
  'v', 0,
  'i', 0,
  'c', 0,
  'e', 0,
};

static const U8 usbd_StrDescriptor_SerialNumber[] =
{
  0x1A,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  'S', 0,
  'T', 0,
  'M', 0,
  '3', 0,
  '2', 0,
  '1', 0,
  '.', 0,
  '0', 0,
  '0', 0,
  '.', 0,
  '0', 0,
  '0', 0,
};

static const U8 usbd_StrDescriptor_CDC[] =
{
  0x0E,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  'S', 0,
  'T', 0,
  'M', 0,
  'C', 0,
  'D', 0,
  'C', 0,
};

static const U8 * usbd_StrDescriptor[STR_DESCRIPTOR_IDX_CNT] =
{
  usbd_StrDescriptor_LanguageId,
  usbd_StrDescriptor_Manufacturer,
  usbd_StrDescriptor_Product,
  usbd_StrDescriptor_SerialNumber,
  usbd_StrDescriptor_CDC,
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
  [USB_INTERFACE_IDX_CDC_IRQ] =
  {
    .CbInit      = CDC_Init,
    .CbCtrlSetup = CDC_CtrlSetupReq,
    .CbCtrlOut   = CDC_CtrlOutReq,
    .CbSOF       = NULL,
    .CbEndPointI = CDC_InterruptIn,
    .CbEndPointO = NULL,
    .EndPointI   = USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDC_IRQ),
    .EndPointO   = 0,
  },
  [USB_INTERFACE_IDX_CDC_DATA] =
  {
    .CbInit      = NULL,
    .CbCtrlSetup = NULL,
    .CbCtrlOut   = NULL,
    .CbSOF       = NULL,
    .CbEndPointI = CDC_BulkIn,
    .CbEndPointO = CDC_BulkOut,
    .EndPointI   = USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDC_DATA),
    .EndPointO   = USB_ENDPOINT_O(USB_ENDPOINT_IDX_CDC_DATA),
  },
};

/* -------------------------------------------------------------------------- */

U8 USBD_CDC_GetInterfaceNumber(void)
{
  return USB_INTERFACE_IDX_CDC_IRQ;
}

/* -------------------------------------------------------------------------- */

U32 USBD_CDC_IrqEndPointWr(U8 *pData, U32 aSize)
{
  return USBD_EP_Wr(USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDC_IRQ), pData, aSize);
}

/* -------------------------------------------------------------------------- */

U32 USBD_CDC_IEndPointWr(U8 *pData, U32 aSize)
{
  return USBD_EP_Wr(USB_ENDPOINT_I(USB_ENDPOINT_IDX_CDC_DATA), pData, aSize);
}

/* -------------------------------------------------------------------------- */

U32 USBD_CDC_OEndPointRd(U8 *pData, U32 aSize)
{
  return USBD_EP_Rd(USB_ENDPOINT_O(USB_ENDPOINT_IDX_CDC_DATA), pData, aSize);
}

/* -------------------------------------------------------------------------- */
