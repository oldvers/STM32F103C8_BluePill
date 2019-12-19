#ifndef __FIFO_H__
#define __FIFO_H__

#include "types.h"
//#include "s25fl.h"

#define FIFO_SUCCESS     0
#define FIFO_IS_FULL     1
#define FIFO_IS_EMPTY    2

////Event Description Structure
//typedef __packed struct Event_s
//{
//  union
//  {
//    U16 Raw;
//    struct
//    {
//      U16 Average : 10; // First 8 + 2 
//      U8 Type   : 3; 
//      U8 AIn   : 2;
//      U8 Value : 1;
//      //Total 16 bit  or 2 bytes
//    } Bits;
//  } Field;
//} Event_t, * Event_p;

//#define MAX_EVENTS_IN_QUEUE    64   //previously  128

/* FIFO Description Structure */
typedef struct FIFO_s
{
  S32  I;            /* Input position */
  S32  O;            /* Output position */
  U32  S;            /* Size of FIFO */
  U8 * B;            /* FIFO buffer */
} FIFO_t, * FIFO_p;

void FIFO_Init(FIFO_p pFIFO, U8 * pBuffer, U32 aSize);
U32  FIFO_Put (FIFO_p pFIFO, U8 * pByte);
U32  FIFO_Get (FIFO_p pFIFO, U8 * pByte);
U32  FIFO_Free(FIFO_p pFIFO);
U32  FIFO_Size(FIFO_p pFIFO);

//U8   FIFO_Save(PCF85Time_p Time, Event_p Event);
//U8   FIFO_Load(PCF85Time_p Time, Event_p Event);
//U8   FIFO_MarkAsSended(void);

#endif /* __FIFO_H__ */
