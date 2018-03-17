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
#define USB_CTRL_PACKET_SIZE      (8)
/* Max MSC Bulk In/Out Endpoint Packet Size */
#define USB_MSC_PACKET_SIZE       (64)
/* Max CDC Interrupt Endpoint Packet Size */
#define USB_CDC_IRQ_PACKET_SIZE   (8)
/* Max CDC Bulk In/Out Endpoint Packet Size */
#define USB_CDC_PACKET_SIZE       (64)
/* Max HID Endpoint Packet Size */
#define USB_HID_PACKET_SIZE       (4)

/* HID Interrupt Endpoint Polling Interval (ms) */
#define USB_HID_IRQ_INTERVAL      (32)
/* CDC Interrupt Endpoint Polling Interval (ms) */
#define USB_CDC_IRQ_INTERVAL      (16)

/* USB Class Support */
/* Mass Storage Device Class (MSC) (0 - Disabled, 1 - Enabled) */
#define USB_MSC                   (0)
/* Communication Device Class (CDC) (0 - Disabled, 1 - Enabled) */
#define USB_CDC                   (1)
/* Human Interface Device (HID) (0 - Disabled, 1 - Enabled) */
#define USB_HID                   (0)

/* --- Calculations -------------------------------------------------------- */
/* USB Endpoint Direction */
#define USB_EP_I             (0x80)
#define USB_EP_O             (0x00)

/* MSC Interface Count */
#define USB_MSC_IF_CNT       (1)
/* MSC Interface Number */
#define USB_MSC_IF_NUM       (USB_MSC * USB_MSC_IF_CNT - 1)
/* MSC Endpoint Count */
#define USB_MSC_EP_CNT       (2)
/* MSC Endpoint Number */
#define USB_MSC_EP(n)        (USB_MSC * (USB_MSC_EP_CNT + n - 1))
#define USB_MSC_EP_BULK_IN   (USB_MSC_EP(0) + USB_EP_I)
#define USB_MSC_EP_BULK_OUT  (USB_MSC_EP(1) + USB_EP_O)

/* CDC Interface Count */
#define USB_CDC_IF_CNT       (2)
/* CDC Interface Number */
#define USB_CDC_IF_NUM0      (USB_MSC_IF_NUM + USB_CDC * USB_CDC_IF_CNT - 1)
#define USB_CDC_IF_NUM1      (USB_MSC_IF_NUM + USB_CDC * USB_CDC_IF_CNT)
/* CDC Endpoint Count */
#define USB_CDC_EP_CNT       (3)
/* CDC Endpoint Number */
#define USB_CDC_EP(n)        (USB_MSC_EP(1) + USB_CDC * USB_CDC_EP_CNT + n - 2)
#define USB_CDC_EP_BULK_OUT  (USB_CDC_EP(0) + USB_EP_O)
#define USB_CDC_EP_BULK_IN   (USB_CDC_EP(1) + USB_EP_I)
#define USB_CDC_EP_IRQ_IN    (USB_CDC_EP(2) + USB_EP_I)

/* Interface Count */
#define USB_HID_IF_CNT       (1)
/* HID Interface Number */
#define USB_HID_IF_NUM       (USB_CDC_IF_NUM1 + USB_HID * USB_HID_IF_CNT)
/* HID Endpoint Count */
#define USB_HID_EP_CNT       (1)
/* CDC Endpoint Number */
#define USB_HID_EP(n)        (USB_CDC_EP(2) + USB_HID * USB_HID_EP_CNT + n)
#define USB_HID_EP_IRQ_IN    (USB_HID_EP(0) + USB_EP_I)

/* Max Number of Interfaces <1-256> */
#define USB_IF_CNT           (USB_MSC * USB_MSC_IF_CNT + \
                              USB_CDC * USB_CDC_IF_CNT + \
                              USB_HID * USB_HID_IF_CNT)
/* Max Number of Bidirectional Endpoints  <1-16> */
#define USB_EP_CNT           (USB_MSC * USB_MSC_EP_CNT + \
                              USB_CDC * USB_CDC_EP_CNT + \
                              USB_HID * USB_HID_EP_CNT + 1)

#if (0 == (USB_MSC + USB_CDC + USB_HID))
#  error "At least one USB Device Class should be selected!"
#endif

#endif  /* __USB_CFG_H__ */
