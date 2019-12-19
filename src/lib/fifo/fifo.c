#include "fifo.h"

/****************************************************************************************/
// Type Definitions
/****************************************************************************************/

/* Very simple queue
 * These are FIFO queues which discard the new data when full.
 *
 * Queue is empty when In == Out.
 * If In != Out, then 
 *  - items are placed into In before incrementing In
 *  - items are removed from Out before incrementing Out
 * Queue is full when In == (Out - 1 + MAX_QUEUE_SIZE) % MAX_QUEUE_SIZE;
 *
 * The queue will hold MAX_EVENTS_IN_QUEUE number of items before the
 * calls to QueuePut fail.
 */

// https://github.com/holodnak/stm32-fdsemu/blob/master/fifo.c
// https://github.com/vyacht/stm32/blob/master/vynmea/fifo.c
// http://mainloop.ru/c-language/simple-fifo.html


//#define MAX_QUEUE_SIZE    (MAX_EVENTS_IN_QUEUE + 1)

//Event Queue Description Structure
//typedef __packed struct Event_Queue_s
//{
//  S32 In;
//  S32 Out;
//  Event_t Event[MAX_QUEUE_SIZE]; //No time needed, time added when saved on S25FL256 chip
//} Event_Queue_t, * Event_Queue_p;

//Storage Event Format Structure Description
//typedef __packed struct Storage_Event_s
//{
//  U32  UnixTime; // 32 bit, 4 bytes, 32 bit, time
//  U16  Event; // 16 bit, 2 bytes, 16 bits, Event data
//  //Total 6 bytes (48 bits), 40 events per pages
//  
//} Storage_Event_t, * Storage_Event_p;

//typedef __packed struct Key_Flags_s
//{
//  U32  Flags[2]; // 32bit * 2, 4 bytes * 2 = 8 bytes = 64bit (64 flags)
//  U32  Rsvd1; // 32bit
//  U16  Rsvd2; // 16bit
//  U16  Key; //16bit (2 last bytes)
//   //total 16 bytes
//} Key_Flags_t, * Key_Flags_p;

/****************************************************************************************/
//  Private Variables
/****************************************************************************************/

//Event_Queue_t   eQueue;
//U32             LastSavedEventAddress;
//U32             LastSendedEventAddress;

/****************************************************************************************/
//*** Event Queue functions ***
/****************************************************************************************/
/****************************************************************************************/
//  Initialize Event Queue and Find LastSavedEventAddress and LasSendedEventAddress
/****************************************************************************************/

//void eQueue_FindLastSavedEventAddress(U32 * LSEA)
//{
//  U16 key = 0xFFFF;
//  U32 laddr, maddr, haddr, paddr;
//  Storage_Event_t sEvent;
//  
//  //Initialize Addresses in Memory
//  LastSavedEventAddress = 0;

//  //Find the Address of the Last not empty Page with Key
//  laddr = 0;
//  haddr = S25FL_GetFlashSize();
//  if(haddr == 0) return;
//  haddr /= S25FL_GetPageSize(); // C /= A, C = C/A

//  while(laddr < haddr)
//  {
//    maddr = laddr + ((haddr - laddr) >> 1);

//    paddr = (maddr * S25FL_GetPageSize()) + 254; // Last 2 bytes have the key
//    S25FL_ReadBuffer(paddr, (U8 *)&key, 2);
//    
//    if(key != 0xCAFE)
//      haddr = maddr;
//    else
//      laddr = maddr + 1;
//  }
//  //If Memory is Clean (ie. empty) -> laddr == 0
//  if(laddr == 0) laddr += 1;
//  
//  //Find the Address of the Last Saved Event in non empty Page
//  sEvent.Event = 0;
//  sEvent.UnixTime = 0;
//  paddr = (laddr - 1) * S25FL_GetPageSize();
//  if(S25FL_ReadBuffer(paddr, (U8 *)&sEvent, sizeof(sEvent)) != sizeof(sEvent)) return;
//  while((sEvent.Event != 0xFF) && (sEvent.UnixTime != 0xFFFFFFFF))
//  {
//    paddr += sizeof(sEvent);
//    if((paddr % S25FL_GetPageSize()) == 240) paddr += 16; // 240 + 16 = 256 ==> Go to next page.
//    if(S25FL_ReadBuffer(paddr, (U8 *)&sEvent, sizeof(sEvent)) != sizeof(sEvent)) return;
//  }
//  LastSavedEventAddress = paddr;
//  if(LSEA != NULL) *LSEA = paddr;
//}
////OK

//void eQueue_FindLastSendedEventAddress(U32 * LSEA)
//{
//  U32 laddr, maddr, haddr, paddr, flags[2] = {0xFFFFFFFF, 0xFFFFFFFF};
//  U8 ecount, fword;
//  
//  //Initialize Addresses in Memory
//  LastSendedEventAddress = 0;

//  //Find the Address of the Last not empty Page with All Flags High
//  laddr = 0;
//  haddr = LastSavedEventAddress; //LastSavedEventAddress needed.
//  if(S25FL_GetPageSize() == 0) return;
//  haddr /= S25FL_GetPageSize();
//  haddr += 1;  //Page where LastSavedEventAddress is (TBC)

//  while(laddr < haddr)
//  {
//    maddr = laddr + ((haddr - laddr) >> 1);

//    paddr = (maddr * S25FL_GetPageSize()) + 240;
//    S25FL_ReadBuffer(paddr, (U8 *)flags, 8);
//    
//    if((flags[0] == 0xFFFFFFFF) && (flags[1] == 0xFFFFFFFF))
//      haddr = maddr;
//    else
//      laddr = maddr + 1;
//  }
//  //If Memory is Clean -> laddr == 0
//  if(laddr == 0) laddr += 1;
//  
//  //Find the Address of the Last Sended Event in non empty Page
//  paddr = (laddr - 1) * S25FL_GetPageSize();
//  S25FL_ReadBuffer((paddr + 240), (U8 *)flags, 8);
//  
//  //Find how many Events Sended in current Page
//  if(flags[0] == 0)
//  {
//    ecount = 32; //bit in flag[0], sended = 0
//    fword = 1;
//  }
//  else
//  {
//    ecount = 0;
//    fword = 0;
//  }
//  
//  for(U8 i = 0; i < 32; i++)
//  {
//    if((flags[fword] & 1) == 0)
//      ecount++;
//    else
//      break;
//    flags[fword] >>= 1;
//  }
//  paddr += ecount * sizeof(Storage_Event_t);

//  LastSendedEventAddress = paddr;
//  if(LSEA != NULL) *LSEA = paddr;
//}
////OK

/****************************************************************************************/
// Event Queue Initialization
/****************************************************************************************/

void FIFO_Init(FIFO_p pFIFO, U8 * pBuffer, U32 aSize)
{
  /* Initialize FIFO */
  pFIFO->I = 0;
  pFIFO->O = 0;
  pFIFO->S = aSize;
  pFIFO->B = pBuffer;
  for(U8 i = 0; i < aSize; i++) pFIFO->B[i] = 0;
}

/****************************************************************************************/
// Put Event To Queue
/****************************************************************************************/

U32 FIFO_Put(FIFO_p pFIFO, U8 * pByte)
{
  if (pFIFO->I == ((pFIFO->O - 1 + pFIFO->S) % pFIFO->S))
  {
    return FIFO_IS_FULL;
  }

  pFIFO->B[pFIFO->I] = *pByte;

  pFIFO->I = (pFIFO->I + 1) % pFIFO->S;

  return FIFO_SUCCESS;
}

/****************************************************************************************/
//  Get Event from Queue
/****************************************************************************************/

U32 FIFO_Get(FIFO_p pFIFO, U8 * pByte)
{
  if (pFIFO->I == pFIFO->O)
  {
    return FIFO_IS_EMPTY;
  }

  *pByte = pFIFO->B[pFIFO->O];

  pFIFO->O = (pFIFO->O + 1) % pFIFO->S;

  return FIFO_SUCCESS;
}

//Before
/****************************************************************************************/
//  All Events are saved in memory page by page.
//  Page format is next:
//    0x0000...0x00F4 (  0...239) = Events Data (5 bytes per Event, 48 Events)
//    0x00F5...0x00FF (240...255) = Key + Event Transmitted Flags (128 bits, 16 Bytes)
//      (112 bits Flags, 16 bits Key)
/****************************************************************************************/

//After
/****************************************************************************************/
//  All Events are saved in memory page by page.
//  Page format is next:
//    0x0000...0x00F4 (  0...239) = Events Data (6 bytes per Event, 40 Events)
//    0x00F5...0x00FF (240...255) = Key + Event Transmitted Flags (128 bits, 16 Bytes)
//      (112 bits Flags, 16 bits Key)
/****************************************************************************************/

//U8 eQueue_Save(PCF85Time_p Time, Event_p Event)
//{
//  Storage_Event_t sEvent;
//  Key_Flags_t KeyAndFlags;
//  U32 size, addr;
//  
//  //Fill Event Structure to Save
//  sEvent.UnixTime = Time->Unix;
//  sEvent.Event = Event->Field.Raw;
//  //sEvent.Average = Event->Field.Bits.Average;
//  //sEvent.Edge = Event->Field.Bits.Edge;
//  
//  //Fill Flags and Key
//  for(U8 i = 0; i < 2; i++) KeyAndFlags.Flags[i] = 0xFFFFFFFF;
//  KeyAndFlags.Rsvd1 = 0xFFFFFFFF;
//  KeyAndFlags.Rsvd2 = 0xFFFF;
//  KeyAndFlags.Key = 0xCAFE;
//  
//  //Save Event to LastSavedEventAddress in Memory
//  addr = LastSavedEventAddress;
//  size = S25FL_WriteBuffer(addr, (U8 *)&sEvent, sizeof(sEvent));
//  if(size != sizeof(sEvent)) return 0;
//  
//  //Save KeyAndFlags to the End of Current Page
//  addr = (LastSavedEventAddress & (~(S25FL_GetPageSize() - 1))) + 240;
//  size = S25FL_WriteBuffer(addr, (U8 *)&KeyAndFlags, sizeof(KeyAndFlags));
//  if(size != sizeof(KeyAndFlags)) return 0;
//  
//  //Increment LastSavedEventAddress
//  LastSavedEventAddress += sizeof(sEvent);
//  if((LastSavedEventAddress % S25FL_GetPageSize()) == 240) LastSavedEventAddress += 16;
//  
//  return 1;
//}
////OK

//U8 eQueue_Load(PCF85Time_p Time, Event_p Event)
//{
//  Storage_Event_t sEvent;
//  U32 size;
//  //U32 sizeofsevent;
//  
//  //Load Event from LastSendedEventAddress in Memory
//  size = S25FL_ReadBuffer(LastSendedEventAddress, (U8 *)&sEvent, sizeof(sEvent));
//  //sizeofsevent = sizeof(sEvent);
//  if(size != sizeof(sEvent)) return 0;
//  
//  //If Event is Empty - Return
//  if((sEvent.UnixTime == 0xFFFFFFFF) && (sEvent.Event == 0xFF)) return 0;
//  
//  //Fill Time
//  Time->Unix = sEvent.UnixTime;
//  PCF85_UnixToTime(Time->Unix, Time);
//  PCF85_TimeToString(Time, Time->String);
//  
//  //Fill Event
//  Event->Field.Raw = sEvent.Event;
//  //Event->Field.Bits.Edge = sEvent.Event;
//  //Event->Field.Bits.Average = sEvent.Average;
//  
//  return 1;
//}

//U8 eQueue_MarkAsSended(void)
//{
//  U32 paddr, bitmask, flags[2] = {0xFFFFFFFF, 0xFFFFFFFF};
//  U8 bitnum, wordnum;
//  
//  paddr = (LastSendedEventAddress & (~(S25FL_GetPageSize() - 1)));
//  bitnum = (LastSendedEventAddress - paddr) / sizeof(Storage_Event_t);
//  
//  wordnum = 0;
//  if(bitnum > 31)
//  {
//    wordnum = 1;
//    bitnum -= 32;
//  }
//  bitmask = (~(1 << bitnum));
//  flags[wordnum] &= bitmask;
//  
//  paddr += 240;
//  bitmask = S25FL_WriteBuffer(paddr, (U8 *)flags, 8);
//  if(bitmask != 8) return 0;
//  
//  //Increment LastSendedEventAddress
//  LastSendedEventAddress += sizeof(Storage_Event_t);
//  if((LastSendedEventAddress % S25FL_GetPageSize()) == 240) LastSendedEventAddress += 16;
//  
//  return 1;
//}
////OK

/*
void resetLastSavedSentEventAddresses (void)
{
//Reset values after ED
LastSavedEventAddress = 0;
LastSendedEventAddress = 0;
}*/

U32 FIFO_Free(FIFO_p pFIFO)
{
  return (pFIFO->S + 1) - (pFIFO->I - pFIFO->O);
}

U32 FIFO_Size(FIFO_p pFIFO)
{
  return (pFIFO->S - 1);
}

//unsigned int fifo_len(fifo_t * fifo)
//{
//    // two usigned will not make this negative 
//    // even if in or out overflow as long as in is ahead of out
//    return fifo->in - fifo->out;
//}

//int fifo_is_empty(fifo_t * fifo)
//{
//     return fifo->in == fifo->out;
//  if (pFIFO->I == pFIFO->O)
//  {
//    return FIFO_IS_EMPTY;
//  }
//}

//int fifo_is_full(fifo_t * fifo)
//{
//     return fifo_len(fifo) > fifo->mask;
//   if (pFIFO->I == ((pFIFO->O - 1 + pFIFO->S) % pFIFO->S))
//  {
//    return FIFO_IS_FULL;
//  }
//}

//unsigned int fifo_free(fifo_t * fifo)
//{
//     return fifo_unused(fifo);
//}
