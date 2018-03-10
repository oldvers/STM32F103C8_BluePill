#ifndef __USB_CORE_H__
#define __USB_CORE_H__

///* USB Endpoint Data Structure */
//typedef struct _USB_EP_DATA
//{
//  U8 *pData;
//  U16 Count;
//} USB_EP_DATA;

/* Endpoint Processing Result */
typedef enum _USB_CTRL_STAGE
{
  USB_CTRL_STAGE_WAIT = 0,
  USB_CTRL_STAGE_DATA,
  USB_CTRL_STAGE_STATUS,
  USB_CTRL_STAGE_ERROR,
} USB_CTRL_STAGE;

typedef void (*USBC_CbGeneral)(void);
typedef void (*USBC_CbWsParam)(U8 aParam);
typedef USB_CTRL_STAGE (*USBC_CbCtrl)
(
  USB_SETUP_PACKET *pSetup,
  U8 **pData,
  U16 *pSize
);
typedef struct _USB_CORE_EVENTS
{
  USBC_CbGeneral CbFeature;
  USBC_CbWsParam CbConfigure;
  USBC_CbGeneral CbInterface;
  USBC_CbCtrl    CtrlSetupReq;
  USBC_CbCtrl    CtrlOutReq;
} USB_CORE_EVENTS;


//void USB_CbFeature(void);
//void USB_CbConfigure(U8 aConfig);
//void USB_CbInterface(void);
//USB_CTRL_STAGE USB_CtrlSetupReqItrface(USB_SETUP_PACKET *pSetup, U8 **pData, U16 *pSize);
//USB_CTRL_STAGE USB_CtrlSetupReqEndpoint(USB_SETUP_PACKET *pSetup, U8 **pData, U16 *pSize);
//USB_CTRL_STAGE USB_CtrlOutReqClassItrface(USB_SETUP_PACKET *pSetup, U8 **pData, U16 *pSize);
//USB_CTRL_STAGE USB_CtrlOutReqClassEndpoint(USB_SETUP_PACKET *pSetup, U8 **pData, U16 *pSize);

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
//extern void USB_ResetCore (void);
void USBC_Init(const USB_CORE_EVENTS *pEvents);
void USBC_Reset(void);
void USBC_ControlInOut(U32 aEvent);

#endif  /* __USB_CORE_H__ */
