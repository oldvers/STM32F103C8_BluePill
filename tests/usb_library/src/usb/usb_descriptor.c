#include "types.h"
#include "usb_config.h"
#include "usb_definitions.h"
#include "usb_descriptor.h"
#include "usb_device.h"
#include "usb_hid_definitions.h"

#include "hid.h"

#define WBVAL(x) (x & 0xFF),((x >> 8) & 0xFF)

typedef enum
{
  STR_DESCRIPTOR_IDX_LANG_ID = 0,   /* Language ID */
  STR_DESCRIPTOR_IDX_MANUFACTURER,  /* iManufacturer */
  STR_DESCRIPTOR_IDX_PRODUCT,       /* iProduct */
  STR_DESCRIPTOR_IDX_SERIAL_NUMBER, /* iSerialNumber */
  STR_DESCRIPTOR_IDX_HID,
  STR_DESCRIPTOR_IDX_CNT
} STR_DESCRIPTOR_IDX;

typedef enum
{
  USB_INTERFACE_IDX_HID = 0,
  /* Interfaces count */
  USB_INTERFACE_IDX_CNT
} USB_INTERFACE_IDX;

typedef enum
{
  USB_ENDPOINT_IDX_CTRL = 0,
  USB_ENDPOINT_IDX_HID,
  /* Endpoints count */
  USB_ENDPOINT_IDX_CNT
} USB_ENDPOINT_IDX;

/* --- Class Specific Definitions ------------------------------------------- */
/* HID Report Descriptor */
const U8 USB_HID_ReportDescriptor[] =
{
  HID_UsagePageVendor(0x00),
  HID_Usage(0x01),
  HID_Collection(HID_Application),
    HID_UsagePage(HID_USAGE_PAGE_BUTTON),
    HID_UsageMin(1),
    HID_UsageMax(2),
    HID_LogicalMin(0),
    HID_LogicalMax(1),
    HID_ReportCount(2),
    HID_ReportSize(1),
    HID_Input(HID_Data | HID_Variable | HID_Absolute),
    HID_ReportCount(1),
    HID_ReportSize(6),
    HID_Input(HID_Constant),
    HID_UsagePage(HID_USAGE_PAGE_LED),
    HID_Usage(HID_USAGE_LED_GENERIC_INDICATOR),
    HID_LogicalMin(0),
    HID_LogicalMax(1),
    HID_ReportCount(8),
    HID_ReportSize(1),
    HID_Output(HID_Data | HID_Variable | HID_Absolute),
  HID_EndCollection,
};

#define USB_HID_REPORT_DESCRIPTOR_SIZE   (sizeof(USB_HID_ReportDescriptor))
/* HID Endpoint Max Packet Size */
#define USB_HID_PACKET_SIZE              (4)
/* HID Interrupt Endpoint Polling Interval (ms) */
#define USB_HID_IRQ_INTERVAL             (32)

/* -------------------------------------------------------------------------- */

U8 USBD_HID_InEndPointWr(U8 *pData, U8 aSize)
{
  return USBD_EndPointWr(USB_ENDPOINT_I(USB_ENDPOINT_IDX_HID), pData, aSize);
}

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
  WBVAL(0xC251),                         /* idVendor */
  WBVAL(0x1C03),                         /* idProduct */
  WBVAL(0x0100), /* 1.00 */              /* bcdDevice */
  STR_DESCRIPTOR_IDX_MANUFACTURER,       /* iManufacturer */
  STR_DESCRIPTOR_IDX_PRODUCT,            /* iProduct */
  STR_DESCRIPTOR_IDX_SERIAL_NUMBER,      /* iSerialNumber */
  0x01                                   /* bNumConfigurations */
};

/* -------------------------------------------------------------------------- */
/* USB Configuration Descriptor */
/* All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
static const U8 USB_ConfigDescriptor[] =
{
/* Configuration 1 */
  USB_CONFIGURATION_DESCRIPTOR_SIZE,     /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE,     /* bDescriptorType */
  WBVAL((                                /* wTotalLength */
   USB_CONFIGURATION_DESCRIPTOR_SIZE * (1)                               +
   USB_INTERFACE_DESCRIPTOR_SIZE     * (USB_INTERFACE_IDX_CNT)           +
   USB_ENDPOINT_DESCRIPTOR_SIZE      * (USB_ENDPOINT_IDX_CNT - 1)        +
   USB_HID_DESCRIPTOR_SIZE
  )),
  USB_INTERFACE_IDX_CNT,                 /* bNumInterfaces */
  0x01,                                  /* bConfigurationValue */
  0x00,                                  /* iConfiguration */
  USB_CONFIG_BUS_POWERED /*|*/           /* bmAttributes */
  /*USB_CONFIG_REMOTE_WAKEUP*/,
  USB_CONFIG_POWER_MA(100),              /* bMaxPower */
/* Interface 0, Alternate Setting 0, HID Class */
  USB_INTERFACE_DESCRIPTOR_SIZE,         /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,         /* bDescriptorType */
  USB_INTERFACE_IDX_HID,                 /* bInterfaceNumber */
  0x00,                                  /* bAlternateSetting */
  0x01,                                  /* bNumEndpoints */
  USB_DEVICE_CLASS_HUMAN_INTERFACE,      /* bInterfaceClass */
  HID_SUBCLASS_NONE,                     /* bInterfaceSubClass */
  HID_PROTOCOL_NONE,                     /* bInterfaceProtocol */
  STR_DESCRIPTOR_IDX_HID,                /* iInterface */
/* HID Class Descriptor */
  USB_HID_DESCRIPTOR_SIZE,               /* bLength */
  HID_HID_DESCRIPTOR_TYPE,               /* bDescriptorType */
  WBVAL(0x0100), /* 1.00 */              /* bcdHID */
  0x00,                                  /* bCountryCode */
  0x01,                                  /* bNumDescriptors */
  HID_REPORT_DESCRIPTOR_TYPE,            /* bDescriptorType */
  WBVAL(USB_HID_REPORT_DESCRIPTOR_SIZE), /* wDescriptorLength */
/* Endpoint, HID Interrupt In */
  USB_ENDPOINT_DESCRIPTOR_SIZE,          /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,          /* bDescriptorType */
  USB_ENDPOINT_I(USB_ENDPOINT_IDX_HID),  /* bEndpointAddress */
  USB_ENDPOINT_TYPE_INTERRUPT,           /* bmAttributes */
  WBVAL(USB_HID_PACKET_SIZE),            /* wMaxPacketSize */
  USB_HID_IRQ_INTERVAL,                  /* bInterval */
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
  0x2C,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  'K', 0,
  'e', 0,
  'i', 0,
  'l', 0,
  ' ', 0,
  'C', 0,
  'o', 0,
  'm', 0,
  'p', 0,
  'o', 0,
  's', 0,
  'i', 0,
  't', 0,
  'e', 0,
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

static const U8 usbd_StrDescriptor_HID[] =
{
  0x0E,                                  /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,            /* bDescriptorType */
  'S', 0,
  'T', 0,
  'M', 0,
  'H', 0,
  'I', 0,
  'D', 0,
};

static const U8 * usbd_StrDescriptor[STR_DESCRIPTOR_IDX_CNT] =
{
  usbd_StrDescriptor_LanguageId,
  usbd_StrDescriptor_Manufacturer,
  usbd_StrDescriptor_Product,
  usbd_StrDescriptor_SerialNumber,
  usbd_StrDescriptor_HID,
};

/* -------------------------------------------------------------------------- */

U8 *USB_GetDeviceDescriptor(void)
{
  return (U8 *)USB_DeviceDescriptor;
};

/* -------------------------------------------------------------------------- */

U8 *USB_GetConfigDescriptor(void)
{
  return (U8 *)USB_ConfigDescriptor;
};

/* -------------------------------------------------------------------------- */

U8 *USB_GetStringDescriptor(U8 aIndex)
{
  return (U8 *)usbd_StrDescriptor[aIndex];
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN USB_GetItrfaceDescriptor
(
  USB_SETUP_PACKET * pSetup,
  U8 **pData,
  U16 *pSize
)
{
  FW_BOOLEAN result = FW_FALSE;

  switch (pSetup->wValue.WB.H)
  {
    case HID_HID_DESCRIPTOR_TYPE:
      if (USB_INTERFACE_IDX_HID == pSetup->wIndex.WB.L)
      {
        U8 * pD = (U8 *)USB_ConfigDescriptor;
        while (((USB_COMMON_DESCRIPTOR *)pD)->bLength)
        {
          if (HID_HID_DESCRIPTOR_TYPE ==
                 ((USB_COMMON_DESCRIPTOR *)pD)->bDescriptorType)
          {
            *pData = pD;
            *pSize = USB_HID_DESCRIPTOR_SIZE;
            result = FW_TRUE;
          }
        }
      }
      break;
    case HID_REPORT_DESCRIPTOR_TYPE:
      if (USB_INTERFACE_IDX_HID == pSetup->wIndex.WB.L)
      {
        *pData = (U8 *)USB_HID_ReportDescriptor;
        *pSize = USB_HID_REPORT_DESCRIPTOR_SIZE;
        result = FW_TRUE;
      }
      break;
    case HID_PHYSICAL_DESCRIPTOR_TYPE:
      break;

    default:
      break;
  }

  return result;
}

/* -------------------------------------------------------------------------- */

U8 USBD_GetItrfacesCount(void)
{
  return USB_INTERFACE_IDX_CNT;
}

/* --- Interfaces Callbacks Descriptor -------------------------------------- */

const USB_INTERFACE_CALLBACKS_DESCRIPTOR
      USB_IfCbDescriptor[USB_INTERFACE_IDX_CNT] =
{
  [USB_INTERFACE_IDX_HID] =
  {
    .CbInit      = HID_Init,
    .CbCtrlSetup = HID_CtrlSetupReq,
    .CbCtrlOut   = HID_CtrlOutReq,
    .CbSOF       = NULL,
    .CbEndPointI = HID_IrqInReq,
    .CbEndPointO = NULL,
    .EndPointI   = USB_ENDPOINT_I(USB_ENDPOINT_IDX_HID),
    .EndPointO   = 0,
  },
};

/* -------------------------------------------------------------------------- */
