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
#define USB_SOF_EVENT             (1)
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

/* Count of Control Endpoints */
#define USB_CTRL_EP_CNT           (2)

/* Max Control Endpoint Packet Size (8, 16, 32 or 64 Bytes) */
#define USB_CTRL_PACKET_SIZE      (64)
/* Max CDC Bulk In/Out Endpoint Packet Size */
#define USB_CDC_PACKET_SIZE       (64)

/* USB Class Support */
/* Communication Device Class (CDC) (0 - Disabled, 1 - Enabled) */
#define USB_CDC                   (1)
/* Communication Device Class 2 (CDD) (0 - Disabled, 1 - Enabled) */
#define USB_CDD                   (1)

/* --- Calculations -------------------------------------------------------- */
/* USB Endpoint Direction */
#define USB_EP_I             (0x80)
#define USB_EP_O             (0x00)

/* CDC Interface Count */
#define USB_CDC_IF_CNT       (1)
/* CDC Interface Number */
#define USB_CDC_IF_NUM       (0)
/* CDC Endpoint Count */
#define USB_CDC_EP_CNT       (2)
/* CDC Endpoint Number */
#define USB_CDC_EP(n)        (USB_CDC * (n / 2 + 1))
#define USB_CDC_EP_BLK_O     (USB_CDC_EP(0) + USB_EP_O)
#define USB_CDC_EP_BLK_I     (USB_CDC_EP(1) + USB_EP_I)

/* CDD Interface Count */
#define USB_CDD_IF_CNT       (1)
/* CDD Interface Number */
#define USB_CDD_IF_NUM       (1)
/* CDD Endpoint Count */
#define USB_CDD_EP_CNT       (2)
/* CDD Endpoint Number */
#define USB_CDD_EP(n)        (USB_CDC_EP(1) + USB_CDD * (n / 2 + 1))
#define USB_CDD_EP_BLK_O     (USB_CDD_EP(0) + USB_EP_O)
#define USB_CDD_EP_BLK_I     (USB_CDD_EP(1) + USB_EP_I)


/* Max Number of Interfaces <1-256> */
#define USB_IF_CNT           (USB_CDC * USB_CDC_IF_CNT + \
                              USB_CDD * USB_CDD_IF_CNT)
/* Max Number of Bidirectional Endpoints  <1-8> */
#define USB_EP_CNT           (USB_CDC * USB_CDC_EP_CNT + \
                              USB_CDD * USB_CDD_EP_CNT + USB_CTRL_EP_CNT)

#if (0 == (USB_CDC + USB_CDD))
#  error "At least one USB Device Class should be selected!"
#endif
#if ((0 == USB_CDC) && (0 != USB_CDD))
#  error "The first CDC class (USB_CDC) must be defined instead!"
#endif

#endif  /* __USB_CFG_H__ */
