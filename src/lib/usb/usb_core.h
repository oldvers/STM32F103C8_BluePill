#ifndef __USB_CORE_H__
#define __USB_CORE_H__

///* USB Endpoint Data Structure */
//typedef struct _USB_EP_DATA
//{
//  U8 *pData;
//  U16 Count;
//} USB_EP_DATA;

/* Endpoint Processing Result */
typedef enum _USB_CTRL_EP_RESULT
{
  USB_CTRL_EP_RESULT_NONE = 0,
  USB_CTRL_EP_RESULT_I_STALL,
  USB_CTRL_EP_RESULT_O_STALL,
  USB_CTRL_EP_RESULT_I_DATA,
  USB_CTRL_EP_RESULT_O_DATA,
  USB_CTRL_EP_RESULT_I_STATUS,
  USB_CTRL_EP_RESULT_O_STATUS,
} USB_CTRL_EP_RESULT;

//typedef void (*USBC_CbGeneric)(void);

/* USB Core Global Variables */
//extern WORD  USB_DeviceStatus;
//extern BYTE  USB_DeviceAddress;
//extern BYTE  USB_Configuration;
//extern DWORD USB_EndPointMask;
//extern DWORD USB_EndPointHalt;
//extern BYTE  USB_AltSetting[USB_IF_NUM];

/* USB Endpoint 0 Buffer */
//extern BYTE  EP0Buf[USB_MAX_PACKET0];

/* USB Endpoint 0 Data Info */
//extern USB_EP_DATA EP0Data;

/* USB Setup Packet */
//extern USB_SETUP_PACKET SetupPacket;

/* USB Core Functions */
//extern void  USB_ResetCore (void);
void USB_ResetCore(void);
void USB_EndPoint0(U32 aEvent);

#endif  /* __USB_CORE_H__ */
