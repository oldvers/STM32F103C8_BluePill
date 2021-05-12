#ifndef __NRF24__
#define __NRF24__

#include "types.h"
#include "nrf24l01p.h"

/* nRF24L0 instruction definitions */
#define nRF24_CMD_R_REGISTER       (U8)0x00
#define nRF24_CMD_W_REGISTER       (U8)0x20
#define nRF24_CMD_R_RX_PAYLOAD     (U8)0x61
#define nRF24_CMD_W_TX_PAYLOAD     (U8)0xA0
#define nRF24_CMD_FLUSH_TX         (U8)0xE1
#define nRF24_CMD_FLUSH_RX         (U8)0xE2
#define nRF24_CMD_REUSE_TX_PL      (U8)0xE3
#define nRF24_CMD_LOCK_UNLOCK      (U8)0x50
#define nRF24_CMD_NOP              (U8)0xFF

/* nRF24L0 register definitions */
/* Configuration register */
#define nRF24_REG_CONFIG           (U8)0x00
/* Enable "Auto acknowledgment" */
#define nRF24_REG_EN_AA            (U8)0x01
/* Enable RX addresses */
#define nRF24_REG_EN_RXADDR        (U8)0x02
/* Setup of address widths */
#define nRF24_REG_SETUP_AW         (U8)0x03
/* Setup of automatic retransmit */
#define nRF24_REG_SETUP_RETR       (U8)0x04
/* RF channel */
#define nRF24_REG_RF_CH            (U8)0x05
/* RF setup register */
#define nRF24_REG_RF_SETUP         (U8)0x06
/* Status register */
#define nRF24_REG_STATUS           (U8)0x07
/* Transmit observe register */
#define nRF24_REG_OBSERVE_TX       (U8)0x08
/* Received power detector */
#define nRF24_REG_RPD              (U8)0x09
/* Receive address data pipe 0 */
#define nRF24_REG_RX_ADDR_P0       (U8)0x0A
/* Receive address data pipe 1 */
#define nRF24_REG_RX_ADDR_P1       (U8)0x0B
/* Receive address data pipe 2 */
#define nRF24_REG_RX_ADDR_P2       (U8)0x0C
/* Receive address data pipe 3 */
#define nRF24_REG_RX_ADDR_P3       (U8)0x0D
/* Receive address data pipe 4 */
#define nRF24_REG_RX_ADDR_P4       (U8)0x0E
/* Receive address data pipe 5 */
#define nRF24_REG_RX_ADDR_P5       (U8)0x0F
/* Transmit address */
#define nRF24_REG_TX_ADDR          (U8)0x10
/* Number of bytes in RX payload in data pipe 0 */
#define nRF24_REG_RX_PW_P0         (U8)0x11
/* Number of bytes in RX payload in data pipe 1 */
#define nRF24_REG_RX_PW_P1         (U8)0x12
/* Number of bytes in RX payload in data pipe 2 */
#define nRF24_REG_RX_PW_P2         (U8)0x13
/* Number of bytes in RX payload in data pipe 3 */
#define nRF24_REG_RX_PW_P3         (U8)0x14
/* Number of bytes in RX payload in data pipe 4 */
#define nRF24_REG_RX_PW_P4         (U8)0x15
/* Number of bytes in RX payload in data pipe 5 */
#define nRF24_REG_RX_PW_P5         (U8)0x16
/* FIFO status register */
#define nRF24_REG_FIFO_STATUS      (U8)0x17
/* Enable dynamic payload length */
#define nRF24_REG_DYNPD            (U8)0x1C
/* Feature register */
#define nRF24_REG_FEATURE          (U8)0x1D

/* Register bits definitions */
#define nRF24_CONFIG_PRIM_RX       (U8)0x01
#define nRF24_CONFIG_PWR_UP        (U8)0x02
/* Data ready RX FIFO interrupt */
#define nRF24_FLAG_RX_DR           (U8)0x40
/* Data sent TX FIFO interrupt */
#define nRF24_FLAG_TX_DS           (U8)0x20
/* Maximum number of TX retransmits interrupt */
#define nRF24_FLAG_MAX_RT          (U8)0x10

/* Register masks definitions */
#define nRF24_MASK_REG_MAP         (U8)0x1F
#define nRF24_MASK_CRC             (U8)0x0C
#define nRF24_MASK_STATUS_IRQ      (U8)0x70
#define nRF24_MASK_RF_PWR          (U8)0x06
#define nRF24_MASK_RX_P_NO         (U8)0x0E
#define nRF24_MASK_DATARATE        (U8)0x28
#define nRF24_MASK_EN_RX           (U8)0x3F
#define nRF24_MASK_RX_PW           (U8)0x3F
#define nRF24_MASK_RETR_ARD        (U8)0xF0
#define nRF24_MASK_RETR_ARC        (U8)0x0F
#define nRF24_MASK_RXFIFO          (U8)0x03
#define nRF24_MASK_TXFIFO          (U8)0x30
#define nRF24_MASK_PLOS_CNT        (U8)0xF0
#define nRF24_MASK_ARC_CNT         (U8)0x0F

/* Fake address to test transceiver presence (5 bytes long) */
#define nRF24_TEST_ADDR            "nRF24"

/* Retransmit delay */
enum
{
  /* Dummy value for case when retransmission is not used */
  nRF24_ARD_NONE   = (U8)0x00,
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

/* Data rate */
enum
{
  nRF24_DR_250kbps = (U8)0x20,
  nRF24_DR_1Mbps   = (U8)0x00,
  nRF24_DR_2Mbps   = (U8)0x08,
};

/* RF output power in TX mode */
enum
{
  nRF24_TXPWR_18dBm = (U8)0x00,
  nRF24_TXPWR_12dBm = (U8)0x02,
  nRF24_TXPWR_6dBm  = (U8)0x04,
  nRF24_TXPWR_0dBm  = (U8)0x06,
};

/* CRC encoding scheme */
enum
{
  nRF24_CRC_off   = (U8)0x00,
  nRF24_CRC_1byte = (U8)0x08,
  nRF24_CRC_2byte = (U8)0x0c,
};

/* nRF24L01 power control */
enum
{
  nRF24_PWR_UP   = (U8)0x02,
  nRF24_PWR_DOWN = (U8)0x00,
};

/* Transceiver mode */
enum
{
  nRF24_MODE_RX = (U8)0x01,
  nRF24_MODE_TX = (U8)0x00,
};

/* Enumeration of RX pipe addresses and TX address */
enum
{
  nRF24_PIPE0  = (U8)0x00,
  nRF24_PIPE1  = (U8)0x01,
  nRF24_PIPE2  = (U8)0x02,
  nRF24_PIPE3  = (U8)0x03,
  nRF24_PIPE4  = (U8)0x04,
  nRF24_PIPE5  = (U8)0x05,
  nRF24_PIPETX = (U8)0x06,
};

/* State of auto acknowledgment for specified pipe */
enum
{
  nRF24_AA_OFF = (U8)0x00,
  nRF24_AA_ON  = (U8)0x01,
};

/* Status of the RX FIFO */
enum
{
  nRF24_STATUS_RXFIFO_DATA  = (U8)0x00,
  nRF24_STATUS_RXFIFO_EMPTY = (U8)0x01,
  nRF24_STATUS_RXFIFO_FULL  = (U8)0x02,
  nRF24_STATUS_RXFIFO_ERROR = (U8)0x03,
};

/* Status of the TX FIFO */
enum
{
  nRF24_STATUS_TXFIFO_DATA  = (U8)0x00,
  nRF24_STATUS_TXFIFO_EMPTY = (U8)0x01,
  nRF24_STATUS_TXFIFO_FULL  = (U8)0x02,
  nRF24_STATUS_TXFIFO_ERROR = (U8)0x03,
};

/* Addresses of the RX_PW_P# registers */
static const U8 nRF24_RX_PW_PIPE[6] =
{
  nRF24_REG_RX_PW_P0,
  nRF24_REG_RX_PW_P1,
  nRF24_REG_RX_PW_P2,
  nRF24_REG_RX_PW_P3,
  nRF24_REG_RX_PW_P4,
  nRF24_REG_RX_PW_P5
};

/* Addresses of the address registers */
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

/* Function prototypes */
FW_BOOLEAN nRF24_Init(void);
void nRF24_SetPowerMode         (U8 aMode);
void nRF24_SetOperationalMode   (U8 aMode);
void nRF24_SetRfChannel         (U8 aChannel);
void nRF24_SetAutoRetr          (U8 aArd, U8 aArc);
void nRF24_SetAddrWidth         (U8 aAddrWidth);
void nRF24_SetAddr              (U8 pipe, const U8 *pAddr);
void nRF24_SetTxPower           (U8 aTxPwr);
void nRF24_SetDataRate          (U8 aDataRate);
void nRF24_SetCrcScheme         (U8 aScheme);
void nRF24_SetRxPipe            (U8 aPipe, U8 aAaState, U8 aPayloadLen);
void nRF24_ClosePipe            (U8 aPipe);
void nRF24_EnableAa             (U8 aPipe);
void nRF24_DisableAa            (U8 aPipe);

U8   nRF24_GetStatus            (void);
U8   nRF24_GetIrqFlags          (void);
U8   nRF24_GetStatus_RxFifo     (void);
U8   nRF24_GetStatus_TxFifo     (void);
U8   nRF24_GetRxSource          (void);
U8   nRF24_GetRetransmitCounters(void);

void nRF24_ResetPlos            (void);
void nRF24_FlushTx              (void);
void nRF24_FlushRx              (void);

void nRF24_WrPayload            (U8 *pBuffer, U8 aSize);
U8   nRF24_RdPayload            (U8 *pBuffer, U8 *pPipe);

U8   nRF24_Receive              (U8 * pBuffer, U8 * pPipe, U32 aTimeout);
U8   nRF24_Transmit             (U8 * pBuffer, U8 aSize, U32 aTimeout);

void nRF24_DumpConfig           (void);

#define nRF24_RxOn()            nRF24L01P_CE_Hi()
#define nRF24_RxOff()           nRF24L01P_CE_Lo()

#define nRF24_TxOn()            nRF24L01P_CE_Hi()
#define nRF24_TxOff()           nRF24L01P_CE_Lo()

#endif /* __NRF24__ */
