#include "east.h"
#include "debug.h"

// Test Packets
//#24 #05#00 #01#02#03#04#05 #42#01
//#24 #40#00 #01#02#03#04#05#06#07#08#09#10#11#12#13#14#15#16
//           #01#02#03#04#05#06#07#08#09#10#11#12#13#14#15#16
//           #01#02#03#04#05#06#07#08#09#10#11#12#13#14#15#16
//           #01#02#03#04#05#06#07#08#09#10#11#12#13#14#15#16 #42#00

//-----------------------------------------------------------------------------

#define EAST_DEBUG

#ifdef EAST_DEBUG
#  define EAST_LOG    LOG
#else
#  define EAST_LOG(...)
#endif

//-----------------------------------------------------------------------------

typedef struct EAST_s
{
    U16             MaxSize;
    U16             ActSize;
    U16             Index;
    U8              CS;
    U8              OK;
    U8            * Buffer;
    //EAST_Completed  OnComplete;
} EAST_t;

//-----------------------------------------------------------------------------

EAST_p EAST_Init(U8 * pContainer, U32 aSize)
{
    EAST_p result = NULL;

    EAST_LOG("- EAST_Init() -\r\n");
    EAST_LOG("--- Inputs\r\n");
    EAST_LOG("  Container Addr = %08X\r\n", pContainer);
    EAST_LOG("  Container Size = %d\r\n", aSize);
    EAST_LOG("--- Internals\r\n");

    /* Check the size of container */
    if ((NULL == pContainer) || (sizeof(EAST_t) > aSize))
    {
        EAST_LOG("  Wrong pointer or not enough size\r\n");
        return NULL;
    }

    /* Initialization */
    result = (EAST_p)pContainer;
    result->MaxSize = 0;
    result->ActSize = 0;
    result->Index = 0;
    result->CS = 0;
    result->OK = 0;
    result->Buffer = NULL;

    EAST_LOG("  EAST Address   = %08X\r\n", result);
    EAST_LOG("  EAST Size      = %d\r\n", sizeof(EAST_t));

    return result;
}

//-----------------------------------------------------------------------------

FW_BOOLEAN EAST_IsCompleted(EAST_p pEAST)
{
    return FW_FALSE;
}

//-----------------------------------------------------------------------------

FW_RESULT EAST_SetBuffer(U8 * pBuffer, U32 aSize)
{
    //if (0 == )
    return FW_ERROR;
}

//-----------------------------------------------------------------------------
/** @brief Puts Byte into EAST packet
 *  @param pEAST - Pointer to EAST state structure
 *  @param aValue - Value that should be placed into packet
 *  @return None
 *  @note Calls OnComplete callback when the packet is collected successfuly.
 */

FW_RESULT EAST_PutByte(EAST_p pEAST, U8 aValue)
//void EAST_PutByte(EAST_STATE * pEAST, U8 aValue)
{
  pEAST->OK = FW_TRUE;

  /* Data Stage */
  if ((1 < pEAST->Index) && (pEAST->Index < (pEAST->ActSize + 2)))
  {
    pEAST->Buffer[pEAST->Index - 2] = aValue;
    pEAST->CS ^= aValue;
  }
  /* Packet Start Byte Stage */
  else if (0 == pEAST->Index)
  {
    pEAST->OK = (aValue == 0x24);
  }
  /* Packet Size Stage */
  else if (1 == pEAST->Index)
  {
    pEAST->ActSize = aValue;
//  }
//  else if (2 == pEAST->Index)
//  {
//    pEAST->ActSize = (pEAST->ActSize + (aValue << 8));
    pEAST->OK = ((0 < pEAST->ActSize) ||
                  (pEAST->MaxSize >= pEAST->ActSize));
    pEAST->CS = 0;
  }
  /* Packet Stop Byte Stage */
  else if (pEAST->Index == (pEAST->ActSize + 2))
  {
    pEAST->OK = (aValue == 0x42);
  }
  /* Packet Control Sum Stage */
  else if (pEAST->Index == (pEAST->ActSize + 3))
  {
    pEAST->OK = (aValue == pEAST->CS);
  }

  /* Packet Index Incrementing Stage */
  if (FW_TRUE == pEAST->OK)
  {
    pEAST->Index++;
  }
  else
  {
    pEAST->Index = 0;
    pEAST->ActSize = 0;
  }

  /* Packet Completed Stage */
  if ((3 < pEAST->Index) && (pEAST->Index == (pEAST->ActSize + 4)))
  {
    /* Call Back */
    //if (NULL != pEAST->OnComplete) pEAST->OnComplete();
    /* Indicate Packet Completed */
    pEAST->Index = 0;
  }
  return FW_SUCCESS;
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte from EAST packet
 *  @param pEAST - Pointer to EAST state structure
 *  @param pValue - Pointer to Value that should be gotten from packet
 *  @return TRUE - Value is available, FALSE - No Value available
 *  @note Should be called at least two times. The firts time - for getting
 *        values from packet. The last time - for calling OnComplete callback.
 */

FW_RESULT EAST_GetByte(EAST_p pEAST, U8 * pValue)
//U32 EAST_GetByte(EAST_STATE * pEAST, U8 * pValue)
{
  pEAST->OK = FW_TRUE;

  /* Data Stage */
  if ((1 < pEAST->Index) && (pEAST->Index < (pEAST->ActSize + 2)))
  {
    *pValue = pEAST->Buffer[pEAST->Index - 2];
    pEAST->CS ^= *pValue;
  }
  /* Packet Start Byte Stage */
  else if (0 == pEAST->Index)
  {
    *pValue = 0x24;
    pEAST->ActSize = pEAST->MaxSize;
  }
  /* Packet Size Stage */
  else if (1 == pEAST->Index)
  {
    *pValue = (pEAST->ActSize & 0xFF);
//  }
//  else if (2 == pEAST->Index)
//  {
//    *pValue = (pEAST->ActSize >> 8);
    pEAST->CS = 0;
  }
  /* Packet Stop Byte Stage */
  else if (pEAST->Index == (pEAST->ActSize + 2))
  {
    *pValue = 0x42;
  }
  /* Packet Control Sum Stage */
  else if (pEAST->Index == (pEAST->ActSize + 3))
  {
    *pValue = pEAST->CS;
  }
  /* Packet Completed Stage */
  else if (pEAST->Index == (pEAST->ActSize + 4))
  {
    /* Increase Index to call OnComplete at next iteration */
    pEAST->Index++;
    pEAST->OK = FW_FALSE;
  }
  else if (pEAST->Index == (pEAST->ActSize + 5))
  {
    /* Call Back */
    //if (NULL != pEAST->OnComplete) pEAST->OnComplete();
    /* Increase Index to avoid multiple call of OnComplete */
    pEAST->Index++;
    pEAST->OK = FW_FALSE;
  }
  else
  {
    pEAST->OK = FW_FALSE;
  }

  /* Packet Index Incrementing Stage */
  if (FW_TRUE == pEAST->OK)
  {
    pEAST->Index++;
  }

  return (FW_RESULT)pEAST->OK;
}
