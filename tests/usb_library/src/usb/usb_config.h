#ifndef __USB_CONFIG_H__
#define __USB_CONFIG_H__

/* --- Configuration ------------------------------------------------------- */

/* USB Device Events */
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

/* Max Control Endpoint Packet Size (8, 16, 32 or 64 Bytes) */
#define USB_CTRL_PACKET_SIZE      (64)

/* Max Count Of Interfaces */
#define USB_INTERFACE_MAX_CNT     (8)

#endif  /* __USB_CONFIG_H__ */
