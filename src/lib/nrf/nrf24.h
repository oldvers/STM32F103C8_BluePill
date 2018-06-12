#ifndef __NRF24__
#define __NRF24__

// Low level functions (hardware depended)
#include "types.h"
#include "nrf24l01p.h"

// nRF24L0 instruction definitions
#define nRF24_CMD_R_REGISTER       (U8)0x00 // Register read
#define nRF24_CMD_W_REGISTER       (U8)0x20 // Register write
#define nRF24_CMD_R_RX_PAYLOAD     (U8)0x61 // Read RX payload
#define nRF24_CMD_W_TX_PAYLOAD     (U8)0xA0 // Write TX payload
#define nRF24_CMD_FLUSH_TX         (U8)0xE1 // Flush TX FIFO
#define nRF24_CMD_FLUSH_RX         (U8)0xE2 // Flush RX FIFO
#define nRF24_CMD_REUSE_TX_PL      (U8)0xE3 // Reuse TX payload
#define nRF24_CMD_LOCK_UNLOCK      (U8)0x50 // Lock/unlock exclusive features
#define nRF24_CMD_NOP              (U8)0xFF // No operation (used for reading status register)

// nRF24L0 register definitions
#define nRF24_REG_CONFIG           (U8)0x00 // Configuration register
#define nRF24_REG_EN_AA            (U8)0x01 // Enable "Auto acknowledgment"
#define nRF24_REG_EN_RXADDR        (U8)0x02 // Enable RX addresses
#define nRF24_REG_SETUP_AW         (U8)0x03 // Setup of address widths
#define nRF24_REG_SETUP_RETR       (U8)0x04 // Setup of automatic retransmit
#define nRF24_REG_RF_CH            (U8)0x05 // RF channel
#define nRF24_REG_RF_SETUP         (U8)0x06 // RF setup register
#define nRF24_REG_STATUS           (U8)0x07 // Status register
#define nRF24_REG_OBSERVE_TX       (U8)0x08 // Transmit observe register
#define nRF24_REG_RPD              (U8)0x09 // Received power detector
#define nRF24_REG_RX_ADDR_P0       (U8)0x0A // Receive address data pipe 0
#define nRF24_REG_RX_ADDR_P1       (U8)0x0B // Receive address data pipe 1
#define nRF24_REG_RX_ADDR_P2       (U8)0x0C // Receive address data pipe 2
#define nRF24_REG_RX_ADDR_P3       (U8)0x0D // Receive address data pipe 3
#define nRF24_REG_RX_ADDR_P4       (U8)0x0E // Receive address data pipe 4
#define nRF24_REG_RX_ADDR_P5       (U8)0x0F // Receive address data pipe 5
#define nRF24_REG_TX_ADDR          (U8)0x10 // Transmit address
#define nRF24_REG_RX_PW_P0         (U8)0x11 // Number of bytes in RX payload in data pipe 0
#define nRF24_REG_RX_PW_P1         (U8)0x12 // Number of bytes in RX payload in data pipe 1
#define nRF24_REG_RX_PW_P2         (U8)0x13 // Number of bytes in RX payload in data pipe 2
#define nRF24_REG_RX_PW_P3         (U8)0x14 // Number of bytes in RX payload in data pipe 3
#define nRF24_REG_RX_PW_P4         (U8)0x15 // Number of bytes in RX payload in data pipe 4
#define nRF24_REG_RX_PW_P5         (U8)0x16 // Number of bytes in RX payload in data pipe 5
#define nRF24_REG_FIFO_STATUS      (U8)0x17 // FIFO status register
#define nRF24_REG_DYNPD            (U8)0x1C // Enable dynamic payload length
#define nRF24_REG_FEATURE          (U8)0x1D // Feature register

// Register bits definitions
#define nRF24_CONFIG_PRIM_RX       (U8)0x01 // PRIM_RX bit in CONFIG register
#define nRF24_CONFIG_PWR_UP        (U8)0x02 // PWR_UP bit in CONFIG register
#define nRF24_FLAG_RX_DR           (U8)0x40 // RX_DR bit (data ready RX FIFO interrupt)
#define nRF24_FLAG_TX_DS           (U8)0x20 // TX_DS bit (data sent TX FIFO interrupt)
#define nRF24_FLAG_MAX_RT          (U8)0x10 // MAX_RT bit (maximum number of TX retransmits interrupt)

// Register masks definitions
#define nRF24_MASK_REG_MAP         (U8)0x1F // Mask bits[4:0] for CMD_RREG and CMD_WREG commands
#define nRF24_MASK_CRC             (U8)0x0C // Mask for CRC bits [3:2] in CONFIG register
#define nRF24_MASK_STATUS_IRQ      (U8)0x70 // Mask for all IRQ bits in STATUS register
#define nRF24_MASK_RF_PWR          (U8)0x06 // Mask RF_PWR[2:1] bits in RF_SETUP register
#define nRF24_MASK_RX_P_NO         (U8)0x0E // Mask RX_P_NO[3:1] bits in STATUS register
#define nRF24_MASK_DATARATE        (U8)0x28 // Mask RD_DR_[5,3] bits in RF_SETUP register
#define nRF24_MASK_EN_RX           (U8)0x3F // Mask ERX_P[5:0] bits in EN_RXADDR register
#define nRF24_MASK_RX_PW           (U8)0x3F // Mask [5:0] bits in RX_PW_Px register
#define nRF24_MASK_RETR_ARD        (U8)0xF0 // Mask for ARD[7:4] bits in SETUP_RETR register
#define nRF24_MASK_RETR_ARC        (U8)0x0F // Mask for ARC[3:0] bits in SETUP_RETR register
#define nRF24_MASK_RXFIFO          (U8)0x03 // Mask for RX FIFO status bits [1:0] in FIFO_STATUS register
#define nRF24_MASK_TXFIFO          (U8)0x30 // Mask for TX FIFO status bits [5:4] in FIFO_STATUS register
#define nRF24_MASK_PLOS_CNT        (U8)0xF0 // Mask for PLOS_CNT[7:4] bits in OBSERVE_TX register
#define nRF24_MASK_ARC_CNT         (U8)0x0F // Mask for ARC_CNT[3:0] bits in OBSERVE_TX register

// Fake address to test transceiver presence (5 bytes long)
#define nRF24_TEST_ADDR            "nRF24"

// Retransmit delay
enum
{
  nRF24_ARD_NONE   = (U8)0x00, // Dummy value for case when retransmission is not used
  nRF24_ARD_250us  = (U8)0x00,
  nRF24_ARD_500us  = (U8)0x01,
  nRF24_ARD_750us  = (U8)0x02,
  nRF24_ARD_1000us = (U8)0x03,
  nRF24_ARD_1250us = (U8)0x04,
  nRF24_ARD_1500us = (U8)0x05,
  nRF24_ARD_1750us = (U8)0x06,
  nRF24_ARD_2000us = (U8)0x07,
  nRF24_ARD_2250us = (U8)0x08,
  nRF24_ARD_2500us = (U8)0x09,
  nRF24_ARD_2750us = (U8)0x0A,
  nRF24_ARD_3000us = (U8)0x0B,
  nRF24_ARD_3250us = (U8)0x0C,
  nRF24_ARD_3500us = (U8)0x0D,
  nRF24_ARD_3750us = (U8)0x0E,
  nRF24_ARD_4000us = (U8)0x0F
};

// Data rate
enum
{
  nRF24_DR_250kbps = (U8)0x20, // 250kbps data rate
  nRF24_DR_1Mbps   = (U8)0x00, // 1Mbps data rate
  nRF24_DR_2Mbps   = (U8)0x08  // 2Mbps data rate
};

// RF output power in TX mode
enum
{
  nRF24_TXPWR_18dBm = (U8)0x00, // -18dBm
  nRF24_TXPWR_12dBm = (U8)0x02, // -12dBm
  nRF24_TXPWR_6dBm  = (U8)0x04, //  -6dBm
  nRF24_TXPWR_0dBm  = (U8)0x06  //   0dBm
};

// CRC encoding scheme
enum
{
  nRF24_CRC_off   = (U8)0x00, // CRC disabled
  nRF24_CRC_1byte = (U8)0x08, // 1-byte CRC
  nRF24_CRC_2byte = (U8)0x0c  // 2-byte CRC
};

// nRF24L01 power control
enum
{
  nRF24_PWR_UP   = (U8)0x02, // Power up
  nRF24_PWR_DOWN = (U8)0x00  // Power down
};

// Transceiver mode
enum
{
  nRF24_MODE_RX = (U8)0x01, // PRX
  nRF24_MODE_TX = (U8)0x00  // PTX
};

// Enumeration of RX pipe addresses and TX address
enum
{
  nRF24_PIPE0  = (U8)0x00, // pipe0
  nRF24_PIPE1  = (U8)0x01, // pipe1
  nRF24_PIPE2  = (U8)0x02, // pipe2
  nRF24_PIPE3  = (U8)0x03, // pipe3
  nRF24_PIPE4  = (U8)0x04, // pipe4
  nRF24_PIPE5  = (U8)0x05, // pipe5
  nRF24_PIPETX = (U8)0x06  // TX address (not a pipe in fact)
};

// State of auto acknowledgment for specified pipe
enum
{
  nRF24_AA_OFF = (U8)0x00,
  nRF24_AA_ON  = (U8)0x01
};

// Status of the RX FIFO
enum
{
  nRF24_STATUS_RXFIFO_DATA  = (U8)0x00, // The RX FIFO contains data and available locations
  nRF24_STATUS_RXFIFO_EMPTY = (U8)0x01, // The RX FIFO is empty
  nRF24_STATUS_RXFIFO_FULL  = (U8)0x02, // The RX FIFO is full
  nRF24_STATUS_RXFIFO_ERROR = (U8)0x03  // Impossible state: RX FIFO cannot be empty and full at the same time
};

// Status of the TX FIFO
enum
{
  nRF24_STATUS_TXFIFO_DATA  = (U8)0x00, // The TX FIFO contains data and available locations
  nRF24_STATUS_TXFIFO_EMPTY = (U8)0x01, // The TX FIFO is empty
  nRF24_STATUS_TXFIFO_FULL  = (U8)0x02, // The TX FIFO is full
  nRF24_STATUS_TXFIFO_ERROR = (U8)0x03  // Impossible state: TX FIFO cannot be empty and full at the same time
};

// Result of RX FIFO reading
//typedef enum
//{
//  nRF24_RX_PIPE0  = (U8)0x00, // Packet received from the PIPE#0
//  nRF24_RX_PIPE1  = (U8)0x01, // Packet received from the PIPE#1
//  nRF24_RX_PIPE2  = (U8)0x02, // Packet received from the PIPE#2
//  nRF24_RX_PIPE3  = (U8)0x03, // Packet received from the PIPE#3
//  nRF24_RX_PIPE4  = (U8)0x04, // Packet received from the PIPE#4
//  nRF24_RX_PIPE5  = (U8)0x05, // Packet received from the PIPE#5
//  nRF24_RX_EMPTY  = (U8)0xff  // The RX FIFO is empty
//} nRF24_RXResult;


// Addresses of the RX_PW_P# registers
static const U8 nRF24_RX_PW_PIPE[6] =
{
  nRF24_REG_RX_PW_P0,
  nRF24_REG_RX_PW_P1,
  nRF24_REG_RX_PW_P2,
  nRF24_REG_RX_PW_P3,
  nRF24_REG_RX_PW_P4,
  nRF24_REG_RX_PW_P5
};

// Addresses of the address registers
static const U8 nRF24_ADDR_REGS[7] =
{
  nRF24_REG_RX_ADDR_P0,
  nRF24_REG_RX_ADDR_P1,
  nRF24_REG_RX_ADDR_P2,
  nRF24_REG_RX_ADDR_P3,
  nRF24_REG_RX_ADDR_P4,
  nRF24_REG_RX_ADDR_P5,
  nRF24_REG_TX_ADDR
};

// Function prototypes
U8 nRF24_Init(void);
//U8 nRF24_Check(void);

void nRF24_SetPowerMode(U8 aMode);
void nRF24_SetOperationalMode(U8 aMode);
void nRF24_SetRfChannel(U8 aChannel);
void nRF24_SetAutoRetr(U8 aArd, U8 aArc);
void nRF24_SetAddrWidth(U8 aAddrWidth);
void nRF24_SetAddr(U8 pipe, const U8 *pAddr);
void nRF24_SetTxPower(U8 aTxPwr);
void nRF24_SetDataRate(U8 aDataRate);
void nRF24_SetCrcScheme(U8 aScheme);
void nRF24_SetRxPipe(U8 aPipe, U8 aAaState, U8 aPayloadLen);
void nRF24_ClosePipe(U8 aPipe);
void nRF24_EnableAa(U8 aPipe);
void nRF24_DisableAa(U8 aPipe);

U8 nRF24_GetStatus(void);
U8 nRF24_GetIrqFlags(void);
U8 nRF24_GetStatus_RxFifo(void);
U8 nRF24_GetStatus_TxFifo(void);
U8 nRF24_GetRxSource(void);
U8 nRF24_GetRetransmitCounters(void);

void nRF24_ResetPlos(void);
void nRF24_FlushTx(void);
void nRF24_FlushRx(void);
//void nRF24_ClearIrqFlags(void);

void nRF24_WrPayload(U8 *pBuffer, U8 aSize);
U8 nRF24_RdPayload(U8 *pBuffer, U8 *pPipe);

U8 nRF24_Receive(U8 * pBuffer, U8 * pPipe, U32 aTimeout);

void nRF24_DumpConfig(void);

#define nRF24_RxOn()     nRF24L01P_CE_Hi()
#define nRF24_RxOff()    nRF24L01P_CE_Lo()

#define nRF24_TxOn()     nRF24L01P_CE_Hi()
#define nRF24_TxOff()    nRF24L01P_CE_Lo()

#endif // __NRF24_H
