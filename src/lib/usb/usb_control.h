#ifndef __USB_CORE_H__
#define __USB_CORE_H__

/* Control Endpoint Processing Stage */
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
/* USB Core Events */
typedef struct _USB_CORE_EVENTS
{
  USBC_CbGeneral CbFeature;
  USBC_CbWsParam CbConfigure;
  USBC_CbGeneral CbInterface;
  USBC_CbCtrl    CtrlSetupReq;
  USBC_CbCtrl    CtrlOutReq;
} USB_CORE_EVENTS;

/* USB Core Functions */
void USBC_Init(const USB_CORE_EVENTS *pEvents);
void USBC_Reset(void);
void USBC_ControlInOut(U32 aEvent);

#endif  /* __USB_CORE_H__ */
