#include "east.h"

// Test Packets
//#24 #05#00 #01#02#03#04#05 #42#01
//#24 #40#00 #01#02#03#04#05#06#07#08#09#10#11#12#13#14#15#16
//           #01#02#03#04#05#06#07#08#09#10#11#12#13#14#15#16
//           #01#02#03#04#05#06#07#08#09#10#11#12#13#14#15#16
//           #01#02#03#04#05#06#07#08#09#10#11#12#13#14#15#16 #42#00

//-----------------------------------------------------------------------------
/** @brief Puts Byte into EAST packet
 *  @param pState - Pointer to EAST state structure
 *  @param aValue - Value that should be placed into packet
 *  @return None
 *  @note Calls OnComplete callback when the packet is collected successfuly.
 */
void EAST_PutByte(EAST_STATE * pState, U8 aValue)
{
  pState->OK = TRUE;

  /* Data Stage */
  if ((1 < pState->Index) && (pState->Index < (pState->ActSize + 2)))
  {
    pState->Buffer[pState->Index - 2] = aValue;
    pState->CS ^= aValue;
  }
  /* Packet Start Byte Stage */
  else if (0 == pState->Index)
  {
    pState->OK = (aValue == 0x24);
  }
  /* Packet Size Stage */
  else if (1 == pState->Index)
  {
    pState->ActSize = aValue;
//  }
//  else if (2 == pState->Index)
//  {
//    pState->ActSize = (pState->ActSize + (aValue << 8));
    pState->OK = ((0 < pState->ActSize) || 
                  (pState->MaxSize >= pState->ActSize));
    pState->CS = 0;
  }
  /* Packet Stop Byte Stage */
  else if (pState->Index == (pState->ActSize + 2))
  {
    pState->OK = (aValue == 0x42);
  }
  /* Packet Control Sum Stage */
  else if (pState->Index == (pState->ActSize + 3))
  {
    pState->OK = (aValue == pState->CS);
  }

  /* Packet Index Incrementing Stage */
  if (TRUE == pState->OK)
  {
    pState->Index++;
  }
  else
  {
    pState->Index = 0;
    pState->ActSize = 0;
  }
  
  /* Packet Completed Stage */
  if ((3 < pState->Index) && (pState->Index == (pState->ActSize + 4)))
  {
    /* Call Back */
    if (NULL != pState->OnComplete) pState->OnComplete();
    /* Indicate Packet Completed */
    pState->Index = 0;
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
U32 EAST_GetByte(EAST_STATE * pState, U8 * pValue)
{
  pState->OK = TRUE;

  /* Data Stage */
  if ((1 < pState->Index) && (pState->Index < (pState->ActSize + 2)))
  {
    *pValue = pState->Buffer[pState->Index - 2];
    pState->CS ^= *pValue;
  }
  /* Packet Start Byte Stage */
  else if (0 == pState->Index)
  {
    *pValue = 0x24;
    pState->ActSize = pState->MaxSize;
  }
  /* Packet Size Stage */
  else if (1 == pState->Index)
  {
    *pValue = (pState->ActSize & 0xFF);
//  }
//  else if (2 == pState->Index)
//  {
//    *pValue = (pState->ActSize >> 8);
    pState->CS = 0;
  }
  /* Packet Stop Byte Stage */
  else if (pState->Index == (pState->ActSize + 2))
  {
    *pValue = 0x42;
  }
  /* Packet Control Sum Stage */
  else if (pState->Index == (pState->ActSize + 3))
  {
    *pValue = pState->CS;
  }
  /* Packet Completed Stage */
  else if (pState->Index == (pState->ActSize + 4))
  {
    /* Increase Index to call OnComplete at next iteration */
    pState->Index++;
    pState->OK = FALSE;
  }
  else if (pState->Index == (pState->ActSize + 5))
  {
    /* Call Back */
    if (NULL != pState->OnComplete) pState->OnComplete();
    /* Increase Index to avoid multiple call of OnComplete */
    pState->Index++;
    pState->OK = FALSE;
  }
  else
  {
    pState->OK = FALSE;
  }

  /* Packet Index Incrementing Stage */
  if (TRUE == pState->OK)
  {
    pState->Index++;
  }

  return pState->OK;
}
