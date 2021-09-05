#ifndef __CDC_PRIVATE_H__
#define __CDC_PRIVATE_H__

#define CDC_CTRL_LINE_STATE_DTR   (0)
#define CDC_CTRL_LINE_STATE_RTS   (1)

/* Line Coding Structure */
typedef __packed struct _CDC_LINE_CODING
{
  U32 dwBaudRate;       /* Number Data terminal rate, in bits per second */
  U8  bCharFormat;      /* Number of Stop bits */
                        /*   0 - 1 Stop bit    *
                         *   1 - 1.5 Stop bits *
                         *   2 - 2 Stop bits   */
  U8  bParityType;      /* Number Parity */
                        /*   0 - None    *
                         *   1 - Odd     *
                         *   2 - Even    *
                         *   3 - Mark    *
                         *   4 - Space   */
  U8  bDataBits;        /* Number Data Bits (5, 6, 7, 8 or 16) */
} CDC_LINE_CODING;

/* Serial State Notification Structure */
typedef __packed struct _CDC_SERIAL_STATE
{
  REQUEST_TYPE bmRequestType;
  U8  bNotification;
  U16 wValue;
  U16 wIndex;
  U16 wLength;
  __packed union
  {
    U16 Raw;
    __packed struct
    {
      U16 bRxCarrier : 1;
      U16 bTxCarrier : 1;
      U16 bBreak : 1;
      U16 bRingSignal : 1;
      U16 bFraming : 1;
      U16 bParity : 1;
      U16 bOverRun : 1;
    } Bit;
  } Data;
} CDC_SERIAL_STATE;

typedef void (*CDC_OPEN_FUNCTION)(void);
typedef U32 (*CDC_EP_IRQ_FUNCTION)(U8 *pData, U32 aSize);
typedef void (*CDC_SET_CTRL_FUNCTION)(U16 aValue);

/* CDC Port Context */
typedef struct _CDC_PORT
{
  CDC_OPEN_FUNCTION      fpOpen;
  CDC_EP_IRQ_FUNCTION    fpEpIIrqWr;
  CDC_SET_CTRL_FUNCTION  fpSetCtrlLine;
  U8                    *irqBuff;
  CDC_LINE_CODING       *lineCoding;
  CDC_SERIAL_STATE      *notification;
  U8                     irqBuffLen;
  FW_BOOLEAN             ready;
} CDC_PORT;

#endif /* __CDC_PRIVATE_H__ */
