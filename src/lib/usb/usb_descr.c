#include "types.h"
#include "usb_cfg.h"
#include "usb_defs.h"
#include "usb_descr.h"
#include "msc_defs.h"
#include "cdc_defs.h"
#include "hid_defs.h"

#include "debug.h"

//-----------------------------------------------------------------------------
#if (USB_HID)
/* HID Report Descriptor */
const U8 HID_ReportDescriptor[] =
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

#define HID_REPORT_DESC_SIZE      (sizeof(HID_ReportDescriptor))
#endif

#define STR_DESC_IDX_LANG_ID        (0)  /* Language ID */
#define STR_DESC_IDX_MANUFACTURER   (1)  /* iManufacturer */
#define STR_DESC_IDX_PRODUCT        (2)  /* iProduct */
#define STR_DESC_IDX_SERIAL_NUMBER  (3)  /* iSerialNumber */
#define STR_DESC_IDX_MSC            (STR_DESC_IDX_SERIAL_NUMBER + USB_MSC)
#define STR_DESC_IDX_CDC            (STR_DESC_IDX_MSC + USB_CDC)
#define STR_DESC_IDX_HID            (STR_DESC_IDX_CDC + USB_HID)
#define STR_DESC_IDX_CNT            (STR_DESC_IDX_HID + 1)

//-----------------------------------------------------------------------------
/* USB Standard Device Descriptor */
static const U8 USB_DeviceDescriptor[] =
{
  USB_DEVICE_DESC_SIZE,              /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0200), /* 2.00 */          /* bcdUSB */

#if ((USB_MSC + USB_CDC + USB_HID) > 1)
  USB_DEVICE_CLASS_MISCELLANEOUS,    /* bDeviceClass */
  0x02,                              /* bDeviceSubClass */
  0x01,                              /* bDeviceProtocol */
#else
  USB_DEVICE_CLASS_RESERVED,         /* bDeviceClass */
  0x00,                              /* bDeviceSubClass */
  0x00,                              /* bDeviceProtocol */
#endif

  USB_CTRL_PACKET_SIZE,              /* bMaxPacketSize0 */
  WBVAL(0xC251),                     /* idVendor */
  WBVAL(0x1C04),                     /* idProduct */
  WBVAL(0x0100), /* 1.00 */          /* bcdDevice */
  STR_DESC_IDX_MANUFACTURER,         /* iManufacturer */
  STR_DESC_IDX_PRODUCT,              /* iProduct */
  STR_DESC_IDX_SERIAL_NUMBER,        /* iSerialNumber */
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
  WBVAL((                            /* wTotalLength */  //1*9+2*9+3*7+1*9
   USB_CONFIGUARTION_DESC_SIZE * (1)                               +
   USB_INTERFACE_DESC_SIZE     * (USB_IF_CNT)                      +
   USB_ENDPOINT_DESC_SIZE      * (USB_EP_CNT - 1)                  +
   USB_HID_DESC_SIZE           * (USB_HID)                         +
   USB_IF_ASSOC_DESC_SIZE      * (USB_MSC + USB_CDC + USB_HID - 1) +
   CDC_FNC_DESC_SUM_SIZE       * (USB_CDC)
  )),
  USB_IF_CNT,                        /* bNumInterfaces */
  0x01,                              /* bConfigurationValue */
  0x00,                              /* iConfiguration */
  USB_CONFIG_BUS_POWERED /*|*/       /* bmAttributes */
  /*USB_CONFIG_REMOTE_WAKEUP*/,
  USB_CONFIG_POWER_MA(100),          /* bMaxPower */

#if (USB_MSC)
/* Interface x, Alternate Setting 0, MSC Class */
  USB_INTERFACE_DESC_SIZE,           /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
  USB_MSC_IF_NUM,                    /* bInterfaceNumber */
  0x00,                              /* bAlternateSetting */
  0x02,                              /* bNumEndpoints */
  USB_DEVICE_CLASS_STORAGE,          /* bInterfaceClass */
  MSC_SUBCLASS_SCSI,                 /* bInterfaceSubClass */
  MSC_PROTOCOL_BULK_ONLY,            /* bInterfaceProtocol */
  STR_DESC_IDX_MSC,                  /* iInterface */
/* Bulk In Endpoint */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_MSC_EP_BULK_IN,                /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
  WBVAL(USB_MSC_PACKET_SIZE),        /* wMaxPacketSize */
  0,                                 /* bInterval */
/* Bulk Out Endpoint */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_MSC_EP_BULK_OUT,               /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
  WBVAL(USB_MSC_PACKET_SIZE),        /* wMaxPacketSize */
  0,                                 /* bInterval */
#endif

#if (1 < (USB_MSC + USB_CDC))
  USB_IF_ASSOC_DESC_SIZE,            /* bLength */
	USB_IF_ASSOC_DESCRIPTOR_TYPE,      /* bDescriptorType */
	USB_CDC_IF_NUM0,                   /* bFirstInterface */
	0x02,                              /* bInterfaceCount */
	USB_DEVICE_CLASS_COMMUNICATIONS,   /* bFunctionClass */
	CDC_IF_SUBCLASS_ACM,               /* bFunctionSubClass */
	CDC_IF_PROTOCOL_AT_CMD,            /* bFunctionProtocol */
	STR_DESC_IDX_CDC,                  /* iFunction (String descriptor index) */
#endif

#if (USB_CDC)
/* Interface x, Alternate Setting 0, CDC Class */
  USB_INTERFACE_DESC_SIZE,           /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
  USB_CDC_IF_NUM0,                   /* bInterfaceNumber */
  0x00,                              /* bAlternateSetting */
  0x01,                              /* bNumEndpoints */
  USB_DEVICE_CLASS_COMMUNICATIONS,   /* bInterfaceClass */
  CDC_IF_SUBCLASS_ACM,               /* bInterfaceSubClass */
  CDC_IF_PROTOCOL_AT_CMD,            /* bInterfaceProtocol */
  STR_DESC_IDX_CDC,                  /* iInterface */
/* Header Functional Descriptor */
  0x05,                              /* bLength */
  CDC_CS_INTERFACE_DESC_TYPE,        /* bDescriptorType */
  CDC_HEADER_FNC_DESC_SUBTYPE,       /* bDescriptorSubtype */
  WBVAL(0x0110),                     /* bcdCDC: Spec Release Number */
/* Call Management Functional Descriptor */
  0x05,                              /* bFunctionLength */
  CDC_CS_INTERFACE_DESC_TYPE,        /* bDescriptorType */
  CDC_CALL_MGMT_FNC_DESC_SUBTYPE,    /* bDescriptorSubtype */
  0x00,                              /* bmCapabilities: D0+D1 */
  0x01,                              /* bDataInterface */
/* ACM Functional Descriptor */
  0x04,                              /* bFunctionLength */
  CDC_CS_INTERFACE_DESC_TYPE,        /* bDescriptorType */
  CDC_ACM_FNC_DESC_SUBTYPE,          /* bDescriptorSubtype */
  0x02,                              /* bmCapabilities */
/* Union Functional Descriptor */
  0x05,                              /* bFunctionLength */
  CDC_CS_INTERFACE_DESC_TYPE,        /* bDescriptorType */
  CDC_UNION_FNC_DESC_SUBTYPE,        /* bDescriptorSubtype */
  USB_CDC_IF_NUM0,                   /* bMasterInterface */
  USB_CDC_IF_NUM1,                   /* bSlaveInterface0 */
/* Endpoint Interrupt Descriptor */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_CDC_EP_IRQ_IN,                 /* bEndpointAddress */
  USB_ENDPOINT_TYPE_INTERRUPT,       /* bmAttributes */
  WBVAL(USB_CDC_IRQ_PACKET_SIZE),    /* wMaxPacketSize */
  USB_CDC_IRQ_INTERVAL,              /* bInterval */
/* Data Class Interface Descriptor */
  USB_INTERFACE_DESC_SIZE,           /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
  USB_CDC_IF_NUM1,                   /* bInterfaceNumber */
  0x00,                              /* bAlternateSetting */
  0x02,                              /* bNumEndpoints */
  CDC_IF_CLASS_CDC,                  /* bInterfaceClass */
  CDC_IF_SUBCLASS_NONE,              /* bInterfaceSubClass */
  CDC_IF_PROTOCOL_NONE,              /* bInterfaceProtocol */
  0,                                 /* iInterface */
/* Bulk Out Endpoint */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_CDC_EP_BULK_OUT,               /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
  WBVAL(USB_CDC_PACKET_SIZE),        /* wMaxPacketSize */
  0,                                 /* bInterval */
/* Bulk In Endpoint */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_CDC_EP_BULK_IN,                /* bEndpointAddress */
  USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
  WBVAL(USB_CDC_PACKET_SIZE),        /* wMaxPacketSize */
  0,                                 /* bInterval */
#endif

#if (0 < (USB_MSC + USB_CDC))
  USB_IF_ASSOC_DESC_SIZE,            /* bLength */
	USB_IF_ASSOC_DESCRIPTOR_TYPE,      /* bDescriptorType */
	USB_HID_IF_NUM,                    /* bFirstInterface */
	0x01,                              /* bInterfaceCount */
	USB_DEVICE_CLASS_HUMAN_INTERFACE,  /* bFunctionClass */
	HID_SUBCLASS_NONE,                 /* bFunctionSubClass */
	HID_PROTOCOL_NONE,                 /* bFunctionProtocol */
	STR_DESC_IDX_HID,                  /* iFunction (String descriptor index) */
#endif

#if (USB_HID)
/* Interface x, Alternate Setting 0, HID Class */
  USB_INTERFACE_DESC_SIZE,           /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
  USB_HID_IF_NUM,                    /* bInterfaceNumber */
  0x00,                              /* bAlternateSetting */
  0x01,                              /* bNumEndpoints */
  USB_DEVICE_CLASS_HUMAN_INTERFACE,  /* bInterfaceClass */
  HID_SUBCLASS_NONE,                 /* bInterfaceSubClass */
  HID_PROTOCOL_NONE,                 /* bInterfaceProtocol */
  STR_DESC_IDX_HID,                  /* iInterface */
/* HID Class Descriptor */
  USB_HID_DESC_SIZE,                 /* bLength */
  HID_HID_DESCRIPTOR_TYPE,           /* bDescriptorType */
  WBVAL(0x0100), /* 1.00 */          /* bcdHID */
  0x00,                              /* bCountryCode */
  0x01,                              /* bNumDescriptors */
  HID_REPORT_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(HID_REPORT_DESC_SIZE),       /* wDescriptorLength */
/* Endpoint, HID Interrupt In */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_HID_EP_IRQ_IN,                 /* bEndpointAddress */
  USB_ENDPOINT_TYPE_INTERRUPT,       /* bmAttributes */
  WBVAL(USB_HID_PACKET_SIZE),        /* wMaxPacketSize */
  USB_HID_IRQ_INTERVAL,              /* bInterval */
#endif

/* Terminator */
  0                                  /* bLength */
};

//-----------------------------------------------------------------------------
/* USB String Descriptor (optional) */
static const U8 USB_StrDescLangId[] =
{
  0x04,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0409), /* US English */    /* wLANGID */
};

static const U8 USB_StrDescManufacturer[] =
{
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
};

static const U8 USB_StrDescProduct[] =
{
  0x2C,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'K',0,
  'e',0,
  'i',0,
  'l',0,
  ' ',0,
  'C',0,
  'o',0,
  'm',0,
  'p',0,
  'o',0,
  's',0,
  'i',0,
  't',0,
  'e',0,
  ' ',0,
  'D',0,
  'e',0,
  'v',0,
  'i',0,
  'c',0,
  'e',0,
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

#if (USB_MSC)
static const U8 USB_StrDescMSC[] =
{
  0x0E,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'S',0,
  'T',0,
  'M',0,
  'M',0,
  'S',0,
  'D',0,
};
#endif

#if (USB_CDC)
static const U8 USB_StrDescCDC[] =
{
  0x0E,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'S',0,
  'T',0,
  'M',0,
  'C',0,
  'D',0,
  'C',0,
};
#endif

#if (USB_HID)
static const U8 USB_StrDescHID[] =
{
  0x0E,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'S',0,
  'T',0,
  'M',0,
  'H',0,
  'I',0,
  'D',0,
};
#endif

static const U8 * USB_StringDescriptor[STR_DESC_IDX_CNT] =
{
  USB_StrDescLangId,
  USB_StrDescManufacturer,
  USB_StrDescProduct,
  USB_StrDescSerialNumber,
#if (USB_MSC)
  USB_StrDescMSC,
#endif
#if (USB_CDC)
  USB_StrDescCDC,
#endif
#if (USB_HID)
  USB_StrDescHID,
#endif
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
U8 *USB_GetStringDescriptor(U8 aIndex)
{
  return (U8 *)USB_StringDescriptor[aIndex];
}

//-----------------------------------------------------------------------------
U32 USB_GetItrfaceDescriptor(USB_SETUP_PACKET * pSetup, U8 **pData, U16 *pSize)
{
  U32 result = FALSE;

#if USB_HID
  switch (pSetup->wValue.WB.H)
  {
    case HID_HID_DESCRIPTOR_TYPE:
      if (USB_HID_IF_NUM == pSetup->wIndex.WB.L)
      {
        U8 * pD = (U8 *)USB_ConfigDescriptor;
        while (((USB_COMMON_DESCRIPTOR *)pD)->bLength)
        {
          if (HID_HID_DESCRIPTOR_TYPE == 
                 ((USB_COMMON_DESCRIPTOR *)pD)->bDescriptorType)
          {
//          *pData = (U8 *)USB_ConfigDescriptor + HID_DESC_OFFSET;
            *pData = pD;
            *pSize = USB_HID_DESC_SIZE;
            result = TRUE;
          }
        }
      }
      break;
    case HID_REPORT_DESCRIPTOR_TYPE:
      if (USB_HID_IF_NUM == pSetup->wIndex.WB.L)
      {
        *pData = (U8 *)HID_ReportDescriptor;
        *pSize = HID_REPORT_DESC_SIZE;
        result = TRUE;
      }
      break;
    case HID_PHYSICAL_DESCRIPTOR_TYPE:
      break;

    default:
      break;
  }
#endif
  return result;
}
