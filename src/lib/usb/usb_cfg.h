#ifndef __USB_CFG_H__
#define __USB_CFG_H__

/* --- Configuration ------------------------------------------------------- */

/* USB Device Event Handlers */
/* Power Event */
#define USB_POWER_EVENT           (0)
/* Reset Event */
#define USB_RESET_EVENT           (1)
/* Suspend Event */
#define USB_SUSPEND_EVENT         (1)
/* Resume Event */
#define USB_RESUME_EVENT          (0)
/* Remote Wakeup Event */
#define USB_WAKEUP_EVENT          (1)
/* Start of Frame Event */
#define USB_SOF_EVENT             (0)
/* Error Event */
#define USB_ERROR_EVENT           (0)

/* USB Core Events */
/* Set Configuration Event */
#define USB_CONFIGURE_EVENT       (1)
/* Set Interface Event */
#define USB_INTERFACE_EVENT       (0)
/* Set/Clear Feature Event */
#define USB_FEATURE_EVENT         (0)

/* USB Power - Default Power Setting (0 - Bus-powered, 1 - Self-powered) */
#define USB_POWER                 (0)

/* Max Control Endpoint Packet Size (8, 16, 32 or 64 Bytes) */
#define USB_CTRL_PACKET_SIZE      (16)
/* Max MSC Bulk In/Out Endpoint Packet Size */
#define USB_ICEMKII_PACKET_SIZE   (64)

/* USB Class Support */
/* AVR JTAG ICE MKII (ICEMKII) (0 - Disabled, 1 - Enabled) */
#define USB_ICEMKII               (1)

/* --- Calculations -------------------------------------------------------- */

/* USB Endpoint Direction */
#define USB_EP_I                  (0x80)
#define USB_EP_O                  (0x00)

/* ICEMKII Interface Count */
#define USB_ICEMKII_IF_CNT        (1)
/* ICEMKII Interface Number */
#define USB_ICEMKII_IF_NUM        (USB_ICEMKII * USB_ICEMKII_IF_CNT - 1)
/* ICEMKII Endpoint Count */
#define USB_ICEMKII_EP_CNT        (2)
/* ICEMKII Endpoint Number */
#define USB_ICEMKII_EP_BULK_IN    (0x02 + USB_EP_I)
#define USB_ICEMKII_EP_BULK_OUT   (0x02 + USB_EP_O)

/* Max Number of Interfaces <1-256> */
#define USB_IF_CNT                (USB_ICEMKII * USB_ICEMKII_IF_CNT)
/* Max Number of Bidirectional Endpoints  <1-8> */
#define USB_EP_CNT                (USB_ICEMKII * USB_ICEMKII_EP_CNT + 1)

#if (0 == (USB_ICEMKII))
#  error "At least one USB Device Class should be selected!"
#endif

#endif  /* __USB_CFG_H__ */
