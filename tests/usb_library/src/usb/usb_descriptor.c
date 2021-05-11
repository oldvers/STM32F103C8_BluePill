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








#include "types.h"
#include "usb_config.h"
#include "usb_definitions.h"
#include "usb_descriptor.h"
#include "usb_control.h"
#include "usb_device.h"
#include "usb_hid_definitions.h"

#include "hid.h"

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
  USB_INTERFACE_IDX_CNT
} USB_INTERFACE_IDX;

typedef enum
{
  USB_ENDPOINT_IDX_HID = 1,
  USB_ENDPOINT_IDX_CNT
} USB_ENDPOINT_IDX;

/* -------------------------------------------------------------------------- */
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
/* USB Standard Device Descriptor */
static const U8 USB_DeviceDescriptor[] =
{
  USB_DEVICE_DESC_SIZE,                  /* bLength */
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
  USB_CONFIGUARTION_DESC_SIZE,           /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE,     /* bDescriptorType */
  WBVAL((                                /* wTotalLength */
   USB_CONFIGUARTION_DESC_SIZE * (1)                               +
   USB_INTERFACE_DESC_SIZE     * (USB_INTERFACE_IDX_CNT)           +
   USB_ENDPOINT_DESC_SIZE      * (USB_ENDPOINT_IDX_CNT - 1)        +
   USB_HID_DESCRIPTOR_SIZE
  )),
  USB_INTERFACE_IDX_CNT,                 /* bNumInterfaces */
  0x01,                                  /* bConfigurationValue */
  0x00,                                  /* iConfiguration */
  USB_CONFIG_BUS_POWERED /*|*/           /* bmAttributes */
  /*USB_CONFIG_REMOTE_WAKEUP*/,
  USB_CONFIG_POWER_MA(100),              /* bMaxPower */
/* Interface 0, Alternate Setting 0, HID Class */
  USB_INTERFACE_DESC_SIZE,               /* bLength */
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
  USB_ENDPOINT_DESC_SIZE,                /* bLength */
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

typedef void (*USBD_CbGeneric)(void);
typedef void (*USBD_CbEndPoint)(U32 aEvent);
typedef USB_CTRL_STAGE (*USBD_CbControl)
(
  USB_SETUP_PACKET *pSetup,
  U8 **pData,
  U16 *pSize
);

typedef struct
{
  USBD_CbGeneric  CbInit;
  USBD_CbControl  CbCtrlSetup;
  USBD_CbControl  CbCtrlOut;
  USBD_CbGeneric  CbSOF;
  USBD_CbEndPoint CbEndPointI;
  USBD_CbEndPoint CbEndPointO;
  U8              EndPointI;
  U8              EndPointO;
} USB_INTERFACE_CALLBACKS_DESCRIPTOR;

const USB_INTERFACE_CALLBACKS_DESCRIPTOR
      USB_IfCbDescriptor[USB_INTERFACE_MAX_CNT] =
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

U8 USBD_HID_InEndPointWr(U8 *pData, U8 aSize)
{
  return USBD_EndPointWr(USB_ENDPOINT_I(USB_ENDPOINT_IDX_HID), pData, aSize);
}
