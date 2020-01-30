#include "icemkii_message.h"

// Test Packets
//#24 #05#00 #01#02#03#04#05 #42#01
//#24 #40#00 #01#02#03#04#05#06#07#08#09#10#11#12#13#14#15#16
//           #01#02#03#04#05#06#07#08#09#10#11#12#13#14#15#16
//           #01#02#03#04#05#06#07#08#09#10#11#12#13#14#15#16
//           #01#02#03#04#05#06#07#08#09#10#11#12#13#14#15#16 #42#00

//-----------------------------------------------------------------------------
/** @brief Puts Byte into JTAG ICE mkII message
 *  @param pPacket - Pointer to ICE mkII message structure
 *  @param aValue - Value that should be placed into message
 *  @return None
 ********  @note Calls OnComplete callback when the packet is collected successfuly.
 */
   
#define ICEMKII_MESSAGE_START               ( 0x1B )
#define ICEMKII_MESSAGE_TOKEN               ( 0x0E )
#define ICEMKII_MESSAGE_HEADER_SIZE         ( 8 )
   
#define ICEMKII_MESSAGE_STAGE_START(i)      (0 == i)
#define ICEMKII_MESSAGE_STAGE_SEQNUML(i)    (1 == i)
#define ICEMKII_MESSAGE_STAGE_SEQNUMH(i)    (2 == i)
#define ICEMKII_MESSAGE_STAGE_SIZE0(i)      (3 == i)
#define ICEMKII_MESSAGE_STAGE_SIZE1(i)      (4 == i)
#define ICEMKII_MESSAGE_STAGE_SIZE2(i)      (5 == i)
#define ICEMKII_MESSAGE_STAGE_SIZE3(i)      (6 == i)
#define ICEMKII_MESSAGE_STAGE_TOKEN(i)      (7 == i)
#define ICEMKII_MESSAGE_STAGE_BODY(i,s)     ((8 <= i) && (i < (8 + s)))
#define ICEMKII_MESSAGE_STAGE_CRCL(i,s)     (i == (8 + s))
#define ICEMKII_MESSAGE_STAGE_CRCH(i,s)     (i == (8 + s + 1))

   
void ICEMKII_MESSAGE_PutByte(ICEMKII_MESSAGE * pMsg, U8 aValue)
{
  pMsg->OK = TRUE;

  /* Message Body Stage */
  if ( ICEMKII_MESSAGE_STAGE_BODY(pMsg->Index, pMsg->ActSize) )
  {
    pMsg->Buffer[pMsg->Index - ICEMKII_MESSAGE_HEADER_SIZE] = aValue;
    //pState->CS ^= aValue;
  }
  /* Message Start Stage */
  else if ( ICEMKII_MESSAGE_STAGE_START(pMsg->Index) )
  {
    pMsg->OK = (aValue == ICEMKII_MESSAGE_START);
  }
  /* Message Sequence Number Stage */
  else if ( ICEMKII_MESSAGE_STAGE_SEQNUML(pMsg->Index) )
  {
    pMsg->SeqNumber = aValue;
  }
  else if ( ICEMKII_MESSAGE_STAGE_SEQNUMH(pMsg->Index) )
  {
    pMsg->SeqNumber = (pMsg->SeqNumber + (aValue << 8));
    //pMsg->OK = ((0 < pMsg->ActSize) || (pMsg->MaxSize >= pMsg->ActSize));
    //pMsg->CS = 0;
  }
  /* Message Size Stage */
  else if ( ICEMKII_MESSAGE_STAGE_SIZE0(pMsg->Index) )
  {
    pMsg->ActSize = aValue;
  }
  else if ( ICEMKII_MESSAGE_STAGE_SIZE1(pMsg->Index) )
  {
    pMsg->ActSize += (U32)(aValue << 8);
  }
  else if ( ICEMKII_MESSAGE_STAGE_SIZE2(pMsg->Index) )
  {
    pMsg->ActSize += (U32)(aValue << 16);
  }
  else if ( ICEMKII_MESSAGE_STAGE_SIZE3(pMsg->Index) )
  {
    pMsg->ActSize += (U32)(aValue << 24);
    pMsg->OK = ((0 < pMsg->ActSize) || (pMsg->MaxSize >= pMsg->ActSize));
    pMsg->CRC16 = 0;
  }
  /* Message Token Stage */
  else if ( ICEMKII_MESSAGE_STAGE_TOKEN(pMsg->Index) )
  {
    pMsg->OK = (aValue == ICEMKII_MESSAGE_TOKEN);
  }
  /* Message CRC Stage */
  else if ( ICEMKII_MESSAGE_STAGE_CRCL(pMsg->Index, pMsg->ActSize) )
  {
    pMsg->CRC16 = aValue;
  }
  else if ( ICEMKII_MESSAGE_STAGE_CRCH(pMsg->Index, pMsg->ActSize) )
  {
    pMsg->CRC16 += (aValue << 8);
    pMsg->OK = (0 == pMsg->CRC16);
  }
  //else if (pState->Index == (pMsg->ActSize + 4))
  //{
  //  pState->OK = (aValue == pState->CS);
  //}

  /* Packet Index Incrementing Stage */
  if (TRUE == pMsg->OK)
  {
    pMsg->Index++;
  }
  else
  {
    pMsg->Index = 0;
    pMsg->ActSize = 0;
  }
  
  /* Packet Completed Stage */
  if ((4 < pMsg->Index) && (pMsg->Index == (pMsg->ActSize + 5)))
  {
    /* Call Back */
    //if (NULL != pMsg->OnComplete) pMsg->OnComplete();
    /* Indicate Packet Completed */
    pMsg->Index = 0;
  }
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte from EAST packet
 *  @param pState - Pointer to EAST state structure
 *  @param pValue - Pointer to Value that should be gotten from packet
 *  @return TRUE - Value is available, FALSE - No Value available
 *  @note Should be called at least two times. The firts time - for getting
 *        values from packet. The last time - for calling OnComplete callback.
 */
//U32 EAST_GetByte(EAST_STATE * pState, U8 * pValue)
//{
//  pState->OK = TRUE;
//
//  /* Data Stage */
//  if ((2 < pState->Index) && (pState->Index < (pState->ActSize + 3)))
//  {
//    *pValue = pState->Buffer[pState->Index - 3];
//    pState->CS ^= *pValue;
//  }
//  /* Packet Start Byte Stage */
//  else if (0 == pState->Index)
//  {
//    *pValue = 0x24;
//    pState->ActSize = pState->MaxSize;
//  }
//  /* Packet Size Stage */
//  else if (1 == pState->Index)
//  {
//    *pValue = (pState->ActSize & 0xFF);
//  }
//  else if (2 == pState->Index)
//  {
//    *pValue = (pState->ActSize >> 8);
//    pState->CS = 0;
//  }
//  /* Packet Stop Byte Stage */
//  else if (pState->Index == (pState->ActSize + 3))
//  {
//    *pValue = 0x42;
//  }
//  /* Packet Control Sum Stage */
//  else if (pState->Index == (pState->ActSize + 4))
//  {
//    *pValue = pState->CS;
//  }
//  /* Packet Completed Stage */
//  else if (pState->Index == (pState->ActSize + 5))
//  {
//    /* Increase Index to call OnComplete at next iteration */
//    pState->Index++;
//    pState->OK = FALSE;
//  }
//  else if (pState->Index == (pState->ActSize + 6))
//  {
//    /* Call Back */
//    if (NULL != pState->OnComplete) pState->OnComplete();
//    /* Increase Index to avoid multiple call of OnComplete */
//    pState->Index++;
//    pState->OK = FALSE;
//  }
//  else
//  {
//    pState->OK = FALSE;
//  }
//
//  /* Packet Index Incrementing Stage */
//  if (TRUE == pState->OK)
//  {
//    pState->Index++;
//  }
//
//  return pState->OK;
//}
