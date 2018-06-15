// Functions to manage the nRF24L01+ transceiver

#include "nrf24.h"
#include "debug.h"
#include "FreeRTOS.h"
#include "semphr.h"

static SemaphoreHandle_t gNrfSemIrq = NULL;

//-----------------------------------------------------------------------------
/** @brief Reads a register
 *  @param Number of register to read
 *  @return Register's value
 */
static U8 nRF24_RdRegister(U8 aNumber)
{
  U8 buffer[2] = {aNumber & nRF24_MASK_REG_MAP, nRF24_CMD_NOP};

  nRF24L01P_CSN_Lo();
  nRF24L01P_Exchange(buffer, buffer, 2);
  nRF24L01P_CSN_Hi();

  return buffer[1];
}

//-----------------------------------------------------------------------------
/** @brief Writes a new value to register
 *  @param Number of register to write
 *  @param Value to write
 *  @return None
 */
static void nRF24_WrRegister(U8 aNumber, U8 aValue)
{
  U8 buffer[2], length = 1;

  nRF24L01P_CSN_Lo();

  if ( nRF24_CMD_W_REGISTER > aNumber )
  {
    /* This is a register access */
    buffer[0] = nRF24_CMD_W_REGISTER | (aNumber & nRF24_MASK_REG_MAP);
    buffer[1] = aValue;
    length++;
  }
  else
  {
    /* This is a single byte command or future command/register */
    buffer[0] = aNumber;
    if ((nRF24_CMD_FLUSH_TX != aNumber) &&
        (nRF24_CMD_FLUSH_RX != aNumber) &&
        (nRF24_CMD_REUSE_TX_PL != aNumber) &&
        (nRF24_CMD_NOP != aNumber))
    {
      /* Send register value */
      buffer[1] = aValue;
      length++;
    }
  }
  nRF24L01P_Exchange(buffer, NULL, length);
  nRF24L01P_CSN_Hi();
}

//-----------------------------------------------------------------------------
/** @brief Clears any pending IRQ flags
 *  @param None
 *  @return Current device status
 */
static U8 nRF24_ClearIrqFlags(void)
{
  U8 result, value = nRF24_CMD_W_REGISTER | nRF24_REG_STATUS;

  nRF24L01P_CSN_Lo();
  nRF24L01P_Exchange(&value, &result, 1);
  /* Set RX_DR, TX_DS and MAX_RT bits of the STATUS register to clear them */
  value = result | nRF24_MASK_STATUS_IRQ;
  nRF24L01P_Exchange(&value, NULL, 1);
  nRF24L01P_CSN_Hi();

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Reads a multi-byte register
 *  @param Number of register to read
 *  @param Pointer to the buffer for register data
 *  @param Number of bytes to read
 *  @return None
 */
static void nRF24_RdMultiRegister(U8 aNumber, U8 *pBuffer, U8 aSize)
{
  nRF24L01P_CSN_Lo();
  nRF24L01P_Exchange(&aNumber, NULL, 1);
  nRF24L01P_Exchange(NULL, pBuffer, aSize);
  nRF24L01P_CSN_Hi();
}

//-----------------------------------------------------------------------------
/** @brief Writes a multi-byte register
 *  @param Number of register to write
 *  @param Pointer to the buffer with data to write
 *  @param Number of bytes to write
 *  @return None
 */
static void nRF24_WrMultiRegister(U8 aNumber, U8 *pBuffer, U8 aSize)
{
  nRF24L01P_CSN_Lo();
  nRF24L01P_Exchange(&aNumber, NULL, 1);
  nRF24L01P_Exchange(pBuffer, NULL, aSize);
  nRF24L01P_CSN_Hi();
}

//-----------------------------------------------------------------------------
/** @brief Checks if the nRF24L01 is present
 *  @param None
 *  @return 1 - nRF24L01 is online and responding
 *          0 - received sequence differs from original
 */
static U8 nRF24_Check(void)
{
  U8 rxbuf[5], i, *ptr = (U8 *)nRF24_TEST_ADDR;

  /* Write test TX address and read TX_ADDR register */
  nRF24_WrMultiRegister(nRF24_CMD_W_REGISTER | nRF24_REG_TX_ADDR, ptr, 5);
  nRF24_RdMultiRegister(nRF24_CMD_R_REGISTER | nRF24_REG_TX_ADDR, rxbuf, 5);

  /* Compare buffers, return error on first mismatch */
  for ( i = 0; i < 5; i++ )
  {
    if (rxbuf[i] != *ptr++) return 0;
  }

  return 1;
}

//-----------------------------------------------------------------------------
/** @brief IRQ pin falling edge callback function
 *  @param None
 *  @return None
 */
static void nRF24_CbIrqPin(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(gNrfSemIrq, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

//-----------------------------------------------------------------------------
/** @brief Sets transceiver to it's initial state
 *  @param None
 *  @return TRUE - init succsess, FALSE - in other cases
 *  @note RX/TX pipe addresses remains untouched
 */
U8 nRF24_Init(void)
{
  /* Create Semaphores/Mutex for VCP */
  gNrfSemIrq = xSemaphoreCreateBinary();
  
  nRF24L01P_Init(nRF24_CbIrqPin);
  
  if ( 0 == nRF24_Check() ) return FALSE;

  /* Write to registers their initial values */
  nRF24_WrRegister(nRF24_REG_CONFIG, 0x08);
  nRF24_WrRegister(nRF24_REG_EN_AA, 0x3F);
  nRF24_WrRegister(nRF24_REG_EN_RXADDR, 0x03);
  nRF24_WrRegister(nRF24_REG_SETUP_AW, 0x03);
  nRF24_WrRegister(nRF24_REG_SETUP_RETR, 0x03);
  nRF24_WrRegister(nRF24_REG_RF_CH, 0x02);
  nRF24_WrRegister(nRF24_REG_RF_SETUP, 0x0E);
  nRF24_WrRegister(nRF24_REG_STATUS, 0x00);
  nRF24_WrRegister(nRF24_REG_RX_PW_P0, 0x00);
  nRF24_WrRegister(nRF24_REG_RX_PW_P1, 0x00);
  nRF24_WrRegister(nRF24_REG_RX_PW_P2, 0x00);
  nRF24_WrRegister(nRF24_REG_RX_PW_P3, 0x00);
  nRF24_WrRegister(nRF24_REG_RX_PW_P4, 0x00);
  nRF24_WrRegister(nRF24_REG_RX_PW_P5, 0x00);
  nRF24_WrRegister(nRF24_REG_DYNPD, 0x00);
  nRF24_WrRegister(nRF24_REG_FEATURE, 0x00);

  /* Clear the FIFO's */
  nRF24_FlushRx();
  nRF24_FlushTx();

  /* Clear any pending interrupt flags */
  (void)nRF24_ClearIrqFlags();

  /* Deassert CSN pin (chip release) */
  nRF24L01P_CSN_Hi();

  return TRUE;
}

//-----------------------------------------------------------------------------
/** @brief Controls transceiver power mode
 *  @param New state of power mode, one of nRF24_PWR_xx values
 *  @return None
 */
void nRF24_SetPowerMode(U8 aMode)
{
  U8 value = nRF24_RdRegister(nRF24_REG_CONFIG);

  if ( aMode == nRF24_PWR_UP )
  {
    /* Set the PWR_UP bit of CONFIG register to wake the transceiver
     * It goes into Stanby-I mode with consumption about 26uA */
    value |= nRF24_CONFIG_PWR_UP;
  }
  else
  {
    /* Clear the PWR_UP bit of CONFIG register to put the transceiver
     * into power down mode with consumption about 900nA */
    value &= ~nRF24_CONFIG_PWR_UP;
  }
  nRF24_WrRegister(nRF24_REG_CONFIG, value);
}

//-----------------------------------------------------------------------------
/** @brief Sets transceiver operational mode
 *  @param Operational mode, one of nRF24_MODE_xx values
 *  @return None
 */
void nRF24_SetOperationalMode(U8 aMode)
{
  U8 value = nRF24_RdRegister(nRF24_REG_CONFIG);

  /* Configure PRIM_RX bit of the CONFIG register */
  value &= ~nRF24_CONFIG_PRIM_RX;
  value |= (aMode & nRF24_CONFIG_PRIM_RX);
  nRF24_WrRegister(nRF24_REG_CONFIG, value);
}

//-----------------------------------------------------------------------------
/** @brief Configures transceiver CRC scheme
 *  @param CRC scheme, one of nRF24_CRC_xx values
 *  @return None
 *  @note Transceiver will forcibly turn on the CRC in case if auto
 *        acknowledgment enabled for at least one RX pipe
 */
void nRF24_SetCrcScheme(U8 aScheme)
{
  U8 value = nRF24_RdRegister(nRF24_REG_CONFIG);

  /* Configure EN_CRC[3] and CRCO[2] bits of the CONFIG register */
  value &= ~nRF24_MASK_CRC;
  value |= (aScheme & nRF24_MASK_CRC);
  nRF24_WrRegister(nRF24_REG_CONFIG, value);
}

//-----------------------------------------------------------------------------
/** @brief Sets frequency channel
 *  @param Radio frequency channel, value from 0 to 127
 *  @return None
 *  @note Frequency will be (2400 + channel) MHz
 *  @note PLOS_CNT[7:4] bits of the OBSERVER_TX register will be reset
 */
void nRF24_SetRfChannel(U8 aChannel)
{
  nRF24_WrRegister(nRF24_REG_RF_CH, aChannel);
}

//-----------------------------------------------------------------------------
/** @brief Sets automatic retransmission parameters
 *  @param Auto retransmit delay, one of nRF24_ARD_xx values
 *  @param Count of auto retransmits, value form 0 to 15
 *  @return None
 *  @note Zero arc value means that the automatic retransmission disabled
 */
void nRF24_SetAutoRetr(U8 aARD, U8 aARC)
{
  /* Sets auto retransmit settings (SETUP_RETR register) */
  nRF24_WrRegister(nRF24_REG_SETUP_RETR, 
    (U8)((aARD << 4) | (aARC & nRF24_MASK_RETR_ARC)));
}

//-----------------------------------------------------------------------------
/** @brief Sets of address widths
 *  @param RX/TX address field width, value from 3 to 5
 *  @return None
 *  @note This setting is common for all pipes
 */
void nRF24_SetAddrWidth(U8 aAddrWidth)
{
  nRF24_WrRegister(nRF24_REG_SETUP_AW, aAddrWidth - 2);
}

//-----------------------------------------------------------------------------
/** @brief Sets static RX address for a specified pipe
 *  @param Pipe to configure address, one of nRF24_PIPEx values
 *  @param Pointer to the buffer with address
 *  @return None
 *  @note Pipe can be a number from 0 to 5 (RX pipes) and 6 (TX pipe)
 *  @note Buffer length must be equal to current address width of transceiver
 *  @note For pipes[2..5] only first byte of address will be written because
 *        other bytes of address equals to pipe1
 *  @note For pipes[2..5] only first byte of address will be written because
 *        pipes 1-5 share the four most significant address bytes
 */
void nRF24_SetAddr(U8 aPipe, const U8 *pAddr)
{
  U8 aBuffer[8], aAddrWidth, i;

  /* RX_ADDR_Px register */
  switch ( aPipe )
  {
    case nRF24_PIPETX:
    case nRF24_PIPE0:
    case nRF24_PIPE1:
      /* Get address width */
      aAddrWidth = nRF24_RdRegister(nRF24_REG_SETUP_AW) + 2;
      /* Write address in reverse order (LSByte first) */
      pAddr += (aAddrWidth - 1);
      aBuffer[0] = nRF24_CMD_W_REGISTER | nRF24_ADDR_REGS[aPipe];
      for (i = 0; i < aAddrWidth; i++)
      {
        aBuffer[i + 1] = *pAddr--;
      }
      nRF24L01P_CSN_Lo();
      nRF24L01P_Exchange(aBuffer, NULL, aAddrWidth + 1);
      nRF24L01P_CSN_Hi();
      break;
    case nRF24_PIPE2:
    case nRF24_PIPE3:
    case nRF24_PIPE4:
    case nRF24_PIPE5:
      /* Write address LSBbyte (only first byte from the addr buffer) */
      nRF24_WrRegister(nRF24_ADDR_REGS[aPipe], *pAddr);
      break;
    default:
      /* Incorrect pipe number -> do nothing */
      break;
  }
}

//-----------------------------------------------------------------------------
/** @brief Configures RF output power in TX mode
 *  @param RF output power, one of nRF24_TXPWR_xx values
 *  @return None
 */
void nRF24_SetTxPower(U8 aTxPwr)
{
  U8 value = nRF24_RdRegister(nRF24_REG_RF_SETUP);

  /* Configure RF_PWR[2:1] bits of the RF_SETUP register */
  value &= ~nRF24_MASK_RF_PWR;
  value |= aTxPwr;
  nRF24_WrRegister(nRF24_REG_RF_SETUP, value);
}

//-----------------------------------------------------------------------------
/** @brief Configures transceiver data rate
 *  @param Data rate, one of nRF24_DR_xx values
 *  @return None
 */
void nRF24_SetDataRate(U8 aDataRate)
{
  U8 value = nRF24_RdRegister(nRF24_REG_RF_SETUP);

  /* Configure RF_DR_LOW[5] and RF_DR_HIGH[3] bits of the RF_SETUP register */
  value &= ~nRF24_MASK_DATARATE;
  value |= aDataRate;
  nRF24_WrRegister(nRF24_REG_RF_SETUP, value);
}

//-----------------------------------------------------------------------------
/** @brief Configures a specified RX pipe
 *  @param Number of the RX pipe, value from 0 to 5
 *  @param State of auto acknowledgment, one of nRF24_AA_xx values
 *  @param Payload length in bytes
 *  @return None
 */
void nRF24_SetRxPipe(U8 aPipe, U8 aAaState, U8 aPayloadLen)
{
  U8 value = nRF24_RdRegister(nRF24_REG_EN_RXADDR);

  /* Enable the specified pipe (EN_RXADDR register) */
  value |= (1 << aPipe);
  value &= nRF24_MASK_EN_RX;
  nRF24_WrRegister(nRF24_REG_EN_RXADDR, value);

  /* Set RX payload length (RX_PW_Px register) */
  nRF24_WrRegister(nRF24_RX_PW_PIPE[aPipe], aPayloadLen & nRF24_MASK_RX_PW);

  /* Set auto acknowledgment for a specified pipe (EN_AA register) */
  value = nRF24_RdRegister(nRF24_REG_EN_AA);
  if ( nRF24_AA_ON == aAaState )
  {
    value |=  (1 << aPipe);
  }
  else
  {
    value &= ~(1 << aPipe);
  }
  nRF24_WrRegister(nRF24_REG_EN_AA, value);
}

//-----------------------------------------------------------------------------
/** @brief Disables specified RX pipe
 *  @param Number of RX pipe, value from 0 to 5
 *  @return None
 */
void nRF24_ClosePipe(U8 aPipe)
{
  U8 value = nRF24_RdRegister(nRF24_REG_EN_RXADDR);
  value &= ~(1 << aPipe);
  value &= nRF24_MASK_EN_RX;
  nRF24_WrRegister(nRF24_REG_EN_RXADDR, value);
}

//-----------------------------------------------------------------------------
/** @brief Enables the auto retransmit (a.k.a. enhanced ShockBurst)
 *         for the specified RX pipe
 *  @param Number of the RX pipe, value from 0 to 5
 *  @return None
 */
void nRF24_EnableAa(U8 aPipe)
{
  /* Set bit in EN_AA register */
  U8 value = nRF24_RdRegister(nRF24_REG_EN_AA) | (1 << aPipe);
  nRF24_WrRegister(nRF24_REG_EN_AA, value);
}

//-----------------------------------------------------------------------------
/** @brief Disables the auto retransmit (a.k.a. enhanced ShockBurst)
 *         for one or all RX pipes
 *  @param Number of the RX pipe, value from 0 to 5, any other value will
 *         disable AA for all RX pipes
 *  @return None
 */
void nRF24_DisableAa(U8 aPipe)
{
  U8 value;

  if ( aPipe > 5)
  {
    /* Disable Auto-ACK for ALL pipes */
    nRF24_WrRegister(nRF24_REG_EN_AA, 0x00);
  }
  else
  {
    /* Clear bit in the EN_AA register */
    value = nRF24_RdRegister(nRF24_REG_EN_AA) & ~(1 << aPipe);
    nRF24_WrRegister(nRF24_REG_EN_AA, value);
  }
}

//-----------------------------------------------------------------------------
/** @briaf Gets value of the STATUS register
 *  @param None
 *  @return Value of STATUS register
 */
U8 nRF24_GetStatus(void)
{
  return nRF24_RdRegister(nRF24_REG_STATUS);
}

//-----------------------------------------------------------------------------
/** @brief Gets pending IRQ flags
 *  @param None
 *  @return Current status of RX_DR, TX_DS and MAX_RT bits of
 *          the STATUS register
 */
U8 nRF24_GetIrqFlags(void)
{
  return (nRF24_RdRegister(nRF24_REG_STATUS) & nRF24_MASK_STATUS_IRQ);
}

//-----------------------------------------------------------------------------
/** @brief Gets status of the RX FIFO
 *  @param None
 *  @return One of the nRF24_STATUS_RXFIFO_xx values
 */
U8 nRF24_GetStatus_RxFifo(void)
{
  return (nRF24_RdRegister(nRF24_REG_FIFO_STATUS) & nRF24_MASK_RXFIFO);
}

//-----------------------------------------------------------------------------
/** @brief Get status of the TX FIFO
 *  @param None
 *  @return One of the nRF24_STATUS_TXFIFO_xx values
 *  @note The TX_REUSE bit ignored
 */
U8 nRF24_GetStatus_TxFifo(void)
{
  return ((nRF24_RdRegister(nRF24_REG_FIFO_STATUS) & nRF24_MASK_TXFIFO) >> 4);
}

//-----------------------------------------------------------------------------
/** @brief Gets pipe number for the payload available for reading from RX FIFO
 *  @param None
 *  @return Pipe number or 0x07 if the RX FIFO is empty
 */
U8 nRF24_GetRxSource(void)
{
  return ((nRF24_RdRegister(nRF24_REG_STATUS) & nRF24_MASK_RX_P_NO) >> 1);
}

//-----------------------------------------------------------------------------
/** @brief Gets auto retransmit statistic
 *  @param None
 *  @return Value of OBSERVE_TX register which contains two counters
 *          encoded in nibbles:
 *          high - lost packets count (max value 15, can be reseted by 
 *                 write to RF_CH register)
 *          low  - retransmitted packets count (max value 15, reseted
 *                 when new transmission starts)
 */
U8 nRF24_GetRetransmitCounters(void)
{
  return nRF24_RdRegister(nRF24_REG_OBSERVE_TX);
}

//-----------------------------------------------------------------------------
/** @brief Resets packet lost counter (PLOS_CNT bits in OBSERVER_TX register)
 *  @param None
 *  @return None
 */
void nRF24_ResetPlos(void)
{
  /* The PLOS counter is reset after write to RF_CH register */
  U8 value = nRF24_RdRegister(nRF24_REG_RF_CH);
  nRF24_WrRegister(nRF24_REG_RF_CH, value);
}

//-----------------------------------------------------------------------------
/** @brief Flushes the TX FIFO
 *  @param None
 *  @return None
 */
void nRF24_FlushTx(void)
{
  nRF24_WrRegister(nRF24_CMD_FLUSH_TX, nRF24_CMD_NOP);
}

//-----------------------------------------------------------------------------
/** @brief Flushes the RX FIFO
 *  @param None
 *  @return None
 */
void nRF24_FlushRx(void)
{
  nRF24_WrRegister(nRF24_CMD_FLUSH_RX, nRF24_CMD_NOP);
}

//-----------------------------------------------------------------------------
/** @brief Writes TX payload
 *  @param Pointer to the buffer with payload data
 *  @param Payload length in bytes
 */
void nRF24_WrPayload(U8 *pBuffer, U8 aSize)
{
  nRF24_WrMultiRegister(nRF24_CMD_W_TX_PAYLOAD, pBuffer, aSize);
}

//-----------------------------------------------------------------------------
/** @brief Reads top level payload available in the RX FIFO
 *  @param Pointer to the buffer to store a payload data
 *  @param Pointer to variable to store a pipe number
 *  @return Number of bytes have been read
 */
U8 nRF24_RdPayload(U8 *pBuffer, U8 *pPipe)
{
  U8 result = 0;

  /* Extract a payload pipe number from the STATUS register */
  *pPipe = (nRF24_RdRegister(nRF24_REG_STATUS) & nRF24_MASK_RX_P_NO) >> 1;

  /* RX FIFO empty? */
  if (6 > *pPipe)
  {
    /* Get payload length */
    result = nRF24_RdRegister(nRF24_RX_PW_PIPE[*pPipe]);

    /* Read a payload from the RX FIFO */
    if ( 0 < result )
    {
      nRF24_RdMultiRegister(nRF24_CMD_R_RX_PAYLOAD, pBuffer, result);
    }
  }

  return result;
}

//-----------------------------------------------------------------------------

/** @brief Prints nRF24L01+ current configuration (for debug purposes)
 *  @param None
 *  @return None
 */
void nRF24_DumpConfig(void)
{
  U8 i, j;
  U8 aw;
  U8 buf[5];
  // Dump nRF24L01+ configuration
  // CONFIG
  i = nRF24_RdRegister(nRF24_REG_CONFIG);
  LOG
  (
  "[0x%02X] <0x%02X> - CONFIG - MASK:%1X CRC:%s CRCSZ:%u PWR:%s MODE:P%s\r\n",
    nRF24_REG_CONFIG,
    (i),
    (i >> 4) & 0x0F,
    (i >> 3) & 1 ? "ON" : "OFF",
    (i >> 2) & 1 + 1,
    (i & 0x02) ? "ON" : "OFF",
    (i & 0x01) ? "RX" : "TX"
  );

  // EN_AA
  i = nRF24_RdRegister(nRF24_REG_EN_AA);
  LOG("[0x%02X] <0x%02X> - EN_AA - ENAA: ", nRF24_REG_EN_AA, i);
  for (j = 0; j < 6; j++)
  {
    LOG
    (
      "[P%1u%s]%s",
      (j),
      (i & (1 << j)) ? "+" : "-",
      (j == 5) ? "\r\n" : " "
    );
  }

  // EN_RXADDR
  i = nRF24_RdRegister(nRF24_REG_EN_RXADDR);
  LOG("[0x%02X] <0x%02X> - EN_RXADDR: ", nRF24_REG_EN_RXADDR, i);
  for (j = 0; j < 6; j++)
  {
    LOG
    (
      "[P%1u%s]%s",
      (j),
      (i & (1 << j)) ? "+" : "-",
      (j == 5) ? "\r\n" : " "
    );
  }

  // SETUP_AW
  i = nRF24_RdRegister(nRF24_REG_SETUP_AW);
  aw = (i & 0x03) + 2;
  LOG
  (
    "[0x%02X] <0x%02X> - SETUP_AW - Addr Width=%u\r\n",
    nRF24_REG_SETUP_AW,
    i,
    aw
  );

  // SETUP_RETR
  i = nRF24_RdRegister(nRF24_REG_SETUP_RETR);
  LOG
  (
    "[0x%02X] <0x%02X> - SETUP_RETR - ARD=%1X ARC=%1X (RetrDelay=%u us, Count=%u)\r\n",
    nRF24_REG_SETUP_RETR,
    (i),
    (i >> 4) & 0x0F,
    (i & 0x0F),
    ((i >> 4) * 250) + 250,
    (i & 0x0F)
  );

  // RF_CH
  i = nRF24_RdRegister(nRF24_REG_RF_CH);
  LOG("[0x%02X] <0x%02X> - RF_CH - (%.3u GHz)\r\n", nRF24_REG_RF_CH, i, 2400 + i);

  // RF_SETUP
  i = nRF24_RdRegister(nRF24_REG_RF_SETUP);
  LOG
  (
    "[0x%02X] <0x%02X> - RF_SETUP - CONT_WAVE:%s PLL_LOCK:%s DataRate=",
    nRF24_REG_RF_SETUP,
    (i),
    (i & 0x80) ? "ON" : "OFF",
    (i & 0x80) ? "ON" : "OFF"
  );
  switch ((i & 0x28) >> 3)
  {
    case 0x00:
      LOG("1M");
      break;
    case 0x01:
      LOG("2M");
      break;
    case 0x04:
      LOG("250k");
      break;
    default:
      LOG("???");
      break;
  }
  LOG("pbs RF_PWR=");
  switch ((i & 0x06) >> 1)
  {
    case 0x00:
      LOG("-18");
      break;
    case 0x01:
      LOG("-12");
      break;
    case 0x02:
      LOG("-6");
      break;
    case 0x03:
      LOG("0");
      break;
    default:
      LOG("???");
      break;
  }
  LOG("dBm\r\n");

  // STATUS
  i = nRF24_RdRegister(nRF24_REG_STATUS);
  LOG
  (
    "[0x%02X] <0x%02X> - STATUS - IRQ:%1X RX_PIPE:%u TX_FULL:%s\r\n",
    nRF24_REG_STATUS,
    (i),
    (i >> 4) & 0x07,
    (i >> 1) & 0x07,
    (i & 1) ? "YES" : "NO"
  );

  // OBSERVE_TX
  i = nRF24_RdRegister(nRF24_REG_OBSERVE_TX);
  LOG
  (
    "[0x%02X] <0x%02X> - OBSERVE_TX - PLOS_CNT=%u ARC_CNT=%u\r\n",
    nRF24_REG_OBSERVE_TX,
    i,
    i >> 4,
    i & 0x0F
  );

  // RPD
  i = nRF24_RdRegister(nRF24_REG_RPD);
  LOG
  (
    "[0x%02X] <0x%02X> - RPD=%s\r\n",
    nRF24_REG_RPD,
    (i),
    (i & 0x01) ? "YES" : "NO"
  );

  // RX_ADDR_P0
  nRF24_RdMultiRegister(nRF24_REG_RX_ADDR_P0, buf, aw);
  LOG("[0x%02X] - RX_ADDR_P0 \"", nRF24_REG_RX_ADDR_P0);
  for (i = 0; i < aw; i++) LOG("%02X", buf[i]);
  LOG("\"\r\n");

  // RX_ADDR_P1
  nRF24_RdMultiRegister(nRF24_REG_RX_ADDR_P1, buf, aw);
  LOG("[0x%02X] - RX_ADDR_P1 \"", nRF24_REG_RX_ADDR_P1);
  for (i = 0; i < aw; i++) LOG("%02X", buf[i]);
  LOG("\"\r\n");

  // RX_ADDR_P2
  LOG("[0x%02X] - RX_ADDR_P2 \"", nRF24_REG_RX_ADDR_P2);
  for (i = 0; i < aw - 1; i++) LOG("%02X", buf[i]);
  i = nRF24_RdRegister(nRF24_REG_RX_ADDR_P2);
  LOG("%02X\"\r\n", i);

  // RX_ADDR_P3
  LOG("[0x%02X] - RX_ADDR_P3 \"", nRF24_REG_RX_ADDR_P3);
  for (i = 0; i < aw - 1; i++) LOG("%02X", buf[i]);
  i = nRF24_RdRegister(nRF24_REG_RX_ADDR_P3);
  LOG("%02X\"\r\n", i);

  // RX_ADDR_P4
  LOG("[0x%02X] - RX_ADDR_P4 \"", nRF24_REG_RX_ADDR_P4);
  for (i = 0; i < aw - 1; i++) LOG("%02X", buf[i]);
  i = nRF24_RdRegister(nRF24_REG_RX_ADDR_P4);
  LOG("%02X\"\r\n", i);

  // RX_ADDR_P5
  LOG("[0x%02X] - RX_ADDR_P5 \"", nRF24_REG_RX_ADDR_P5);
  for (i = 0; i < aw - 1; i++) LOG("%02X", buf[i]);
  i = nRF24_RdRegister(nRF24_REG_RX_ADDR_P5);
  LOG("%02X\"\r\n", i);

  // TX_ADDR
  nRF24_RdMultiRegister(nRF24_REG_TX_ADDR, buf, aw);
  LOG("[0x%02X] - TX_ADDR \"", nRF24_REG_TX_ADDR);
  for (i = 0; i < aw; i++) LOG("%02X", buf[i]);
  LOG("\"\r\n");

  // RX_PW_P0
  i = nRF24_RdRegister(nRF24_REG_RX_PW_P0);
  LOG("[0x%02X] - RX_PW_P0=%u\r\n", nRF24_REG_RX_PW_P0, i);

  // RX_PW_P1
  i = nRF24_RdRegister(nRF24_REG_RX_PW_P1);
  LOG("[0x%02X] - RX_PW_P1=%u\r\n", nRF24_REG_RX_PW_P1, i);

  // RX_PW_P2
  i = nRF24_RdRegister(nRF24_REG_RX_PW_P2);
  LOG("[0x%02X] - RX_PW_P2=%u\r\n", nRF24_REG_RX_PW_P2, i);

  // RX_PW_P3
  i = nRF24_RdRegister(nRF24_REG_RX_PW_P3);
  LOG("[0x%02X] - RX_PW_P3=%u\r\n", nRF24_REG_RX_PW_P3, i);

  // RX_PW_P4
  i = nRF24_RdRegister(nRF24_REG_RX_PW_P4);
  LOG("[0x%02X] - RX_PW_P4=%u\r\n", nRF24_REG_RX_PW_P4, i);

  // RX_PW_P5
  i = nRF24_RdRegister(nRF24_REG_RX_PW_P5);
  LOG("[0x%02X] - RX_PW_P5=%u\r\n", nRF24_REG_RX_PW_P5, i);

  // FIFO_STATUS
  i = nRF24_RdRegister(nRF24_REG_FIFO_STATUS);
  LOG("[0x%02X] <0x%02X> - FIFO_STATUS\r\n", nRF24_REG_FIFO_STATUS, i);
  
  // DYNPD
  i = nRF24_RdRegister(nRF24_REG_DYNPD);
  LOG("[0x%02X] <0x%02X> - DYNPD\r\n", nRF24_REG_DYNPD, i);

  // FEATURE
  i = nRF24_RdRegister(nRF24_REG_FEATURE);
  LOG("[0x%02X] <0x%02X> - FEATURE\r\n", nRF24_REG_FEATURE, i);
}

//-----------------------------------------------------------------------------
/** @brief Waits for and reads received packet
 *  @param Pointer to the buffer to store a data
 *  @param Pointer to variable to store a pipe number
 *  @param Waiting timeout
 *  @return Number of bytes have been received
 */
U8 nRF24_Receive(U8 * pBuffer, U8 * pPipe, U32 aTimeout)
{
  U8 status, result = 0;

  /* Wait for IRQ */
  if (pdTRUE == xSemaphoreTake(gNrfSemIrq, aTimeout))
  {
    status = nRF24_GetStatus_RxFifo();

    /* Check if Rx FIFO is not empty */
    if (nRF24_STATUS_RXFIFO_EMPTY != status)
    {
      /* The RX FIFO have some data, take a note what nRF24 can hold
       * up to three payloads of 32 bytes. Read a payload to buffer */
      result = nRF24_RdPayload(pBuffer, pPipe);
      /* Now the pBuffer holds received data, result variable holds a length
       * of received data, pPipe variable holds a number of the pipe which
       * has received the data. Do something with received data. */
    }
  }

  /* Clear any pending IRQ bits */
  (void)nRF24_ClearIrqFlags();

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Transmits a packet for a timeout
 *  @param Pointer to the buffer of packet data
 *  @param Size of packet
 *  @param Waiting timeout
 *  @return Number of bytes have been transmitted
 */
U8 nRF24_Transmit(U8 * pBuffer, U8 aSize, U32 aTimeout)
{
  U8 status, result = 0;

  /* Transfer payload data to transceiver */
  nRF24_WrPayload(pBuffer, aSize);

  /* Assert CE pin (transmission starts) */
  nRF24_TxOn();

  /* Wait for IRQ */
  if (pdTRUE == xSemaphoreTake(gNrfSemIrq, aTimeout))
  {
    /* Clear any pending IRQ flags */
    status = nRF24_ClearIrqFlags(); 

    if (0 != (status & nRF24_FLAG_TX_DS))
    {
      result = aSize;
    }
  }

  /* De-assert CE pin (nRF24 goes to StandBy-I mode) */
  nRF24_TxOff();

  return result;
}
