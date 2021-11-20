#include <string.h>
#include "icemkii_message.h"
#include "debug.h"

#define ICEMKII_DEBUG

#ifdef ICEMKII_DEBUG
#  define ICEMKII_LOG    DBG
#else
#  define ICEMKII_LOG(...)
#endif

#define ICEMKII_MSG_START                   ( 0x1B )
#define ICEMKII_MSG_TOKEN                   ( 0x0E )
#define ICEMKII_MSG_HEADER_SIZE             ( 8 )
#define ICEMKII_MSG_WRAPPER_SIZE            ( ICEMKII_MSG_HEADER_SIZE + 2 )
#define ICEMKII_MSG_MODE_INPUT              ( 0 )
#define ICEMKII_MSG_MODE_OUTPUT             ( 1 )

#define ICEMKII_MSG_CHECK_LENGTH(a,m)       ((FW_BOOLEAN)((0 < a) && (a <= m)))
#define ICEMKII_MSG_POSITION(b,i)           (b[(i) - ICEMKII_MSG_HEADER_SIZE])

#define ICEMKII_MSG_STAGE_START(i)          (0 == i)
#define ICEMKII_MSG_STAGE_SEQNUML(i)        (1 == i)
#define ICEMKII_MSG_STAGE_SEQNUMH(i)        (2 == i)
#define ICEMKII_MSG_STAGE_SIZE0(i)          (3 == i)
#define ICEMKII_MSG_STAGE_SIZE1(i)          (4 == i)
#define ICEMKII_MSG_STAGE_SIZE2(i)          (5 == i)
#define ICEMKII_MSG_STAGE_SIZE3(i)          (6 == i)
#define ICEMKII_MSG_STAGE_TOKEN(i)          (7 == i)
#define ICEMKII_MSG_STAGE_BODY(i,s)         ((7 < i) && (i < (8 + s)))
#define ICEMKII_MSG_STAGE_CALC_CRC(i,s)     (i < (8 + s))
#define ICEMKII_MSG_STAGE_CRCL(i,s)         (i == (8 + s))
#define ICEMKII_MSG_STAGE_CRCH(i,s)         (i == (8 + s + 1))
#define ICEMKII_MSG_STAGE_COMPLETE(i,s)     (i == (8 + s + 2))

#define CRC16_INIT_VALUE                    ( 0xFFFF )

typedef struct ICEMKII_MSG_s
{
  U8            * Buffer;
  U32             MaxSize;
  U32             ActSize;
  U16             Index;
  U16             RCRC;
  U16             FCRC;
  U16             SeqNumber;
  U16             LastError;
  struct
  {
      U16         OK : 1;       /* Message correctness */
      U16         Mode : 1;     /* Message mode: in/out */
      U16         Complete : 1; /* Message completeness */
  };
} ICEMKII_MSG_t;

static const U16 CRC16_Table[256] =
{
    0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
    0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
    0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
    0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
    0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
    0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
    0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
    0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
    0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
    0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
    0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
    0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
    0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
    0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
    0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
    0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
    0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
    0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
    0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
    0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
    0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
    0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
    0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
    0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
    0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
    0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
    0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
    0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
    0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
    0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
    0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
    0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78,
};

//-----------------------------------------------------------------------------

static U16 CRC16_Get(U8 * pMsg, U32 size, U16 crc)
{
   for (U32 i = 0; i < size; i++)
   {
       crc = (crc >> 8) ^ CRC16_Table[(crc ^ pMsg[i]) & 0x00FF];
   }
   return crc;
}

//-----------------------------------------------------------------------------
/** @brief Inits the JTAG ICE mkII message
 *  @param[in] pHolder - Pointer to the holder for the instance
 *  @param[in] aSize - Size of the holder
 *  @param[in] pBuffer - Pointer to the the JTAG ICE mkII message data buffer
 *  @param[in] aBufferSize - Size of the buffer
 *  @return Pointer to the created JTAG ICE mkII component
 */

ICEMKII_MSG_p ICEMKII_MSG_Init
(
    U8 * pHolder,
    U32 aSize,
    U8 *pBuffer,
    U32 aBufferSize
)
{
    ICEMKII_MSG_p result = NULL;

    ICEMKII_LOG("-*- ICEMKII_Init() -*-\r\n");
    ICEMKII_LOG("--- Inputs\r\n");
    ICEMKII_LOG("  Holder    Addr = %08X\r\n", pHolder);
    ICEMKII_LOG("  Container Size = %d\r\n", aSize);
    ICEMKII_LOG("  Buffer Addr    = %08X\r\n", pBuffer);
    ICEMKII_LOG("  Buffer Size    = %d\r\n", aSize);
    ICEMKII_LOG("--- Internals\r\n");
    ICEMKII_LOG("  ICEMKII Size   = %d\r\n", sizeof(ICEMKII_MSG_t));

    /* Check the size of the holder */
    if ((NULL == pHolder) || (sizeof(ICEMKII_MSG_t) > aSize))
    {
        ICEMKII_LOG("  Input parameters error!\r\n");
        return NULL;
    }

    /* Initialization */
    result = (ICEMKII_MSG_p)pHolder;
    memset(result, 0, sizeof(ICEMKII_MSG_t));

    if ((NULL != pBuffer) && (0 != aBufferSize))
    {
        result->Buffer = pBuffer;
        result->MaxSize = aBufferSize;
        result->Mode = ICEMKII_MSG_MODE_OUTPUT;
    }

    ICEMKII_LOG("  ICEMKII Addr   = %08X\r\n", result);

    return result;
}

//-----------------------------------------------------------------------------

FW_RESULT ICEMKII_MSG_SetBuffer
(
    ICEMKII_MSG_p pMsg,
    U8 * pBuffer,
    U32 aSize
)
{
    U16 temp = 0;

    ICEMKII_LOG("-*- ICEMKII_SetBuffer() -*-\r\n");
    ICEMKII_LOG("--- Inputs\r\n");
    ICEMKII_LOG("  Buffer Address = %08X\r\n", pBuffer);
    ICEMKII_LOG("  Buffer Size    = %d\r\n", aSize);
    ICEMKII_LOG("--- Internals\r\n");

    if ((NULL == pMsg) || (NULL == pBuffer) || (0 == aSize))
    {
        ICEMKII_LOG("  Input parameters error!\r\n");
        return FW_ERROR;
    }

    /* Preserve the sequence number */
    temp = pMsg->SeqNumber;

    /* Reset the state */
    memset(pMsg, 0, sizeof(ICEMKII_MSG_t));

    /* Set the buffer */
    pMsg->Buffer = pBuffer;
    pMsg->MaxSize = aSize;
    pMsg->Mode = ICEMKII_MSG_MODE_OUTPUT;
    pMsg->SeqNumber = temp;

    ICEMKII_LOG("  ICEMKII Buffer = %08X\r\n", pBuffer);
    ICEMKII_LOG("  ICEMKII Buf Sz = %d\r\n", aSize);

    return FW_SUCCESS;
}

//-----------------------------------------------------------------------------
/** @brief Puts Byte into JTAG ICE mkII message
 *  @param pPacket - Pointer to ICE mkII message instance
 *  @param aValue - Value that should be placed into the message
 *  @return FW_INPROGRESS / FW_COMPLETE / FW_ERROR
 *  @note On the next call of this function after packet completion,
 *        collectiong of the new packet is started from the beginning. The
 *        previously collected packet is lost.
 */

FW_RESULT ICEMKII_MSG_PutByte(ICEMKII_MSG_p pMsg, U8 aValue)
{
    FW_RESULT result = FW_INPROGRESS;
    pMsg->OK = FW_TRUE;

    ICEMKII_LOG("-*- ICEMKII_PutByte -*-\r\n");
    ICEMKII_LOG("--- Inputs\r\n");
    ICEMKII_LOG("  ICEMKII Addr   = %08X\r\n", pMsg);
    ICEMKII_LOG("  Value          = %02X\r\n", aValue);
    ICEMKII_LOG("--- Internals\r\n");

    if ((NULL == pMsg) || (NULL == pMsg->Buffer))
    {
        ICEMKII_LOG("  Input parameters error!\r\n");
        return FW_ERROR;
    }

    pMsg->Mode = ICEMKII_MSG_MODE_INPUT;
    pMsg->Complete = FW_FALSE;

    /* Message Body Stage */
    if ( ICEMKII_MSG_STAGE_BODY(pMsg->Index, pMsg->ActSize) )
    {
        ICEMKII_MSG_POSITION(pMsg->Buffer, pMsg->Index) = aValue;
    }
    /* Message Start Stage */
    else if ( ICEMKII_MSG_STAGE_START(pMsg->Index) )
    {
        pMsg->OK = (FW_BOOLEAN)(aValue == ICEMKII_MSG_START);
        pMsg->RCRC = CRC16_INIT_VALUE;
    }
    /* Message Sequence Number Stage */
    else if ( ICEMKII_MSG_STAGE_SEQNUML(pMsg->Index) )
    {
        pMsg->SeqNumber = aValue;
    }
    else if ( ICEMKII_MSG_STAGE_SEQNUMH(pMsg->Index) )
    {
        pMsg->SeqNumber = (pMsg->SeqNumber + (aValue << 8));

        ICEMKII_LOG("  Sequence       = %d\r\n", pMsg->SeqNumber);
    }
    /* Message Size Stage */
    else if ( ICEMKII_MSG_STAGE_SIZE0(pMsg->Index) )
    {
        pMsg->ActSize = aValue;
    }
    else if ( ICEMKII_MSG_STAGE_SIZE1(pMsg->Index) )
    {
        pMsg->ActSize += (U32)(aValue << 8);
    }
    else if ( ICEMKII_MSG_STAGE_SIZE2(pMsg->Index) )
    {
        pMsg->ActSize += (U32)(aValue << 16);
    }
    else if ( ICEMKII_MSG_STAGE_SIZE3(pMsg->Index) )
    {
        pMsg->ActSize += (U32)(aValue << 24);
        pMsg->OK  = (FW_BOOLEAN)(0 < pMsg->ActSize);
        pMsg->OK |= (FW_BOOLEAN)(pMsg->MaxSize >= pMsg->ActSize);

        ICEMKII_LOG("  Length         = %d\r\n", pMsg->ActSize);
    }
    /* Message Token Stage */
    else if ( ICEMKII_MSG_STAGE_TOKEN(pMsg->Index) )
    {
        pMsg->OK = (FW_BOOLEAN)(aValue == ICEMKII_MSG_TOKEN);
    }
    /* Message CRC Stage */
    else if ( ICEMKII_MSG_STAGE_CRCL(pMsg->Index, pMsg->ActSize) )
    {
        pMsg->FCRC = aValue;
    }
    else if ( ICEMKII_MSG_STAGE_CRCH(pMsg->Index, pMsg->ActSize) )
    {
        pMsg->FCRC += (aValue << 8);
        pMsg->OK = (FW_BOOLEAN)(pMsg->FCRC == pMsg->RCRC);

        ICEMKII_LOG("  Factual CRC    = %04X\r\n", pMsg->FCRC);
        ICEMKII_LOG("  Running CRC    = %04X\r\n", pMsg->RCRC);
    }

    /* Packet Index Incrementing Stage */
    if (FW_TRUE == pMsg->OK)
    {
        if ( ICEMKII_MSG_STAGE_CALC_CRC(pMsg->Index, pMsg->ActSize) )
        {
            pMsg->RCRC = CRC16_Get(&aValue, 1, pMsg->RCRC);
        }
        pMsg->Index++;
    }
    else
    {
        result = FW_ERROR;
        pMsg->LastError = pMsg->Index;
        pMsg->Index = 0;
        pMsg->ActSize = 0;

        ICEMKII_LOG("  Wrong packet format!\r\n");
    }

    /* Packet Completed Stage */
    if ( ICEMKII_MSG_STAGE_COMPLETE(pMsg->Index, pMsg->ActSize) )
    {
        /* Indicate Message Completed */
        result = FW_COMPLETE;
        pMsg->Complete = FW_TRUE;
        pMsg->Index = 0;

        ICEMKII_LOG("  Packet complete!\r\n");
    }

    return result;
}

//-----------------------------------------------------------------------------
/** @brief Sets the Sequence Number of the JTAG ICE mkII message
 *  @param[in] pMsg - Pointer to the ICE mkII message instance
 *  @param[in] aValue - The Value of the Sequence Number
 *  @return None
 */

void ICEMKII_MSG_SetSequenceNumber(ICEMKII_MSG_p pMsg, U16 aValue)
{
    if (NULL != pMsg)
    {
        pMsg->SeqNumber = aValue;
    }
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte from the JTAG ICE mkII message
 *  @param[in] pMsg - Pointer to the ICE mkII message instance
 *  @param[out] pValue - Pointer to the placeholder for the Value
 *  @return FW_INPROGRESS / FW_COMPLETE
 *  @note On the next call of this function after packet completion,
 *        producing of the new packet is started from the beginning. The
 *        previously produced packet is left as is.
 */

FW_RESULT ICEMKII_MSG_GetByte(ICEMKII_MSG_p pMsg, U8 * pValue)
{
    FW_RESULT result = FW_INPROGRESS;
    pMsg->OK = FW_TRUE;

    pMsg->Mode = ICEMKII_MSG_MODE_OUTPUT;
    pMsg->Complete = FW_FALSE;

    ICEMKII_LOG("-*- ICEMKII_GetByte -*-\r\n");
    ICEMKII_LOG("--- Inputs\r\n");
    ICEMKII_LOG("  ICEMKII Addr   = %08X\r\n", pMsg);
    ICEMKII_LOG("  pValue         = %08X\r\n", pValue);
    ICEMKII_LOG("--- Internals\r\n");

    if ((NULL == pMsg) || (NULL == pMsg->Buffer))
    {
        ICEMKII_LOG("  Input parameters error!\r\n");
        return FW_ERROR;
    }

    /* Message Body Stage */
    if ( ICEMKII_MSG_STAGE_BODY(pMsg->Index, pMsg->ActSize) )
    {
        *pValue = ICEMKII_MSG_POSITION(pMsg->Buffer, pMsg->Index);
    }
    /* Message Start Stage */
    else if ( ICEMKII_MSG_STAGE_START(pMsg->Index) )
    {
        *pValue = ICEMKII_MSG_START;
        pMsg->ActSize = pMsg->MaxSize;
        pMsg->RCRC = CRC16_INIT_VALUE;
    }
    /* Message Sequence Number Stage */
    else if ( ICEMKII_MSG_STAGE_SEQNUML(pMsg->Index) )
    {
        *pValue = (pMsg->SeqNumber & 0xFF);
    }
    else if ( ICEMKII_MSG_STAGE_SEQNUMH(pMsg->Index) )
    {
        *pValue = ((pMsg->SeqNumber >> 8) & 0xFF);

        ICEMKII_LOG("  Sequence       = %d\r\n", pMsg->SeqNumber);
    }
    /* Message Size Stage */
    else if ( ICEMKII_MSG_STAGE_SIZE0(pMsg->Index) )
    {
        *pValue = (pMsg->ActSize & 0xFF);
    }
    else if ( ICEMKII_MSG_STAGE_SIZE1(pMsg->Index) )
    {
        *pValue = ((pMsg->ActSize >> 8) & 0xFF);
    }
    else if ( ICEMKII_MSG_STAGE_SIZE2(pMsg->Index) )
    {
        *pValue = ((pMsg->ActSize >> 16) & 0xFF);
    }
    else if ( ICEMKII_MSG_STAGE_SIZE3(pMsg->Index) )
    {
        *pValue = ((pMsg->ActSize >> 24) & 0xFF);

        ICEMKII_LOG("  Length         = %d\r\n", pMsg->ActSize);
    }
    /* Message Token Stage */
    else if ( ICEMKII_MSG_STAGE_TOKEN(pMsg->Index) )
    {
        *pValue = ICEMKII_MSG_TOKEN;
    }
    /* Message CRC Stage */
    else if ( ICEMKII_MSG_STAGE_CRCL(pMsg->Index, pMsg->ActSize) )
    {
      *pValue = (pMsg->RCRC & 0xFF);
    }
    else if ( ICEMKII_MSG_STAGE_CRCH(pMsg->Index, pMsg->ActSize) )
    {
        *pValue = ((pMsg->RCRC >> 8) & 0xFF);

        ICEMKII_LOG("  Factual CRC    = %04X\r\n", pMsg->FCRC);
        ICEMKII_LOG("  Running CRC    = %04X\r\n", pMsg->RCRC);
    }

    /* Packet Index Incrementing Stage */
    if (FW_TRUE == pMsg->OK)
    {
        if ( ICEMKII_MSG_STAGE_CALC_CRC(pMsg->Index, pMsg->ActSize) )
        {
            pMsg->RCRC = CRC16_Get(pValue, 1, pMsg->RCRC);
        }
        pMsg->Index++;
    }
    else
    {
        result = FW_ERROR;
        pMsg->LastError = pMsg->Index;
        pMsg->Index = 0;
        pMsg->ActSize = 0;

        ICEMKII_LOG("  Wrong packet format!\r\n");
    }

    /* Packet Completed Stage */
    if ( ICEMKII_MSG_STAGE_COMPLETE(pMsg->Index, pMsg->ActSize) )
    {
        /* Indicate Message Completed */
        pMsg->Complete = FW_TRUE;
        result = FW_COMPLETE;
        pMsg->Index = 0;

        ICEMKII_LOG("  Packet complete!\r\n");
    }
    else
    {
        ICEMKII_LOG("  *pValue        = %02X\r\n", *pValue);
    }

    return result;
}

//-----------------------------------------------------------------------------
/** @brief Gets the size of the currently collected data
 *  @param[in] pMsg - Pointer to the JTAG ICE mkII message instance
 *  @return Packet size
 */

U16 ICEMKII_MSG_GetDataSize(ICEMKII_MSG_p pMsg)
{
    U16 result = 0;

    if (ICEMKII_MSG_MODE_INPUT == pMsg->Mode)
    {
        if (FW_TRUE == pMsg->Complete)
        {
            result = pMsg->ActSize;
        }
    }
    else
    {
        result = pMsg->MaxSize;
    }

    return result;
}

//-----------------------------------------------------------------------------
/** @brief Gets the size of the currently collected data
 *  @param[in] pMsg - Pointer to the JTAG ICE mkII message instance
 *  @return Packet size
 */

U16 ICEMKII_MSG_GetPacketSize(ICEMKII_MSG_p pMsg)
{
    U16 result = 0;

    if (ICEMKII_MSG_MODE_INPUT == pMsg->Mode)
    {
        if (FW_TRUE == pMsg->Complete)
        {
            result = (ICEMKII_MSG_WRAPPER_SIZE + pMsg->ActSize);
        }
    }
    else
    {
        if (FW_FALSE == pMsg->Complete)
        {
            result = (ICEMKII_MSG_WRAPPER_SIZE + pMsg->MaxSize - pMsg->Index);
        }
    }

    return result;
}
