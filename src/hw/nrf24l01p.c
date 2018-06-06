#include "nrf24l01p.h"

//-----------------------------------------------------------------------------
void nRF24L01P_Init(void)
{
  /* CE -> GPO, Push-Pull */
  GPIO_Init(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_TYPE_OUT_PP_2MHZ);

  /* CSN -> GPO, Push-Pull */
  GPIO_Init(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_TYPE_OUT_PP_2MHZ);

  /* IRQ -> GPI, Floating */
  GPIO_Init(NRF24_IRQ_PORT, NRF24_IRQ_PIN, GPIO_TYPE_IN_FLOATING);

  /* SCK, MOSI, MISO -> AFO, Push-Pull */
  GPIO_Init(NRF24_SCK_PORT,  NRF24_SCK_PIN,  GPIO_TYPE_ALT_PP_50MHZ);
  GPIO_Init(NRF24_MOSI_PORT, NRF24_MOSI_PIN, GPIO_TYPE_ALT_PP_50MHZ);
  GPIO_Init(NRF24_MISO_PORT, NRF24_MISO_PIN, GPIO_TYPE_ALT_PP_50MHZ);

  /* SPI -> 6 MHz; DMA */
  RCC->NRF24_SPI_CLK_R |= NRF24_SPI_CLK_E;

  if(NRF24_SPI_CLK_E == RCC_APB2ENR_SPI1EN)
  {
    // 72 MHz / 16 = 4.5 MHz
    NRF24_SPI->CR1 |= 
  //  SPI_CR1_BIDIMODE |   //Bidirectional Mode
  //  SPI_CR1_BIDIOE |     //Bidirectional Mode Output Enable
  //  SPI_CR1_CRCEN |      //CRC
  //  SPI_CR1_CRCNEXT |    //
  //  SPI_CR1_DFF |        //Data Frame Format
  //  SPI_CR1_RXONLY |     //Rx Only Mode
      SPI_CR1_SSM  |       //Software Slave control
      SPI_CR1_SSI  |       //Internal Slave select 
  //  SPI_CR1_LSBFIRST |   //LSB First
      SPI_CR1_BR_0 |       //Prescaller
      SPI_CR1_BR_1 |
  //  SPI_CR1_BR_2 |
      SPI_CR1_MSTR |       //Master mode
  //  SPI_CR1_CPOL |
  //  SPI_CR1_CPHA |
  //  SPI_CR1_SPE |        //SPI Enable
      0;
  }
  else
  {
    RCC->NRF24_SPI_CLK_R |= NRF24_SPI_CLK_E;
    // 36 MHz / 8 = 4.5 MHz
    NRF24_SPI->CR1 |= 
  //  SPI_CR1_BIDIMODE |   //Bidirectional Mode
  //  SPI_CR1_BIDIOE |     //Bidirectional Mode Output Enable
  //  SPI_CR1_CRCEN |      //CRC
  //  SPI_CR1_CRCNEXT |    //
  //  SPI_CR1_DFF |        //Data Frame Format
  //  SPI_CR1_RXONLY |     //Rx Only Mode
      SPI_CR1_SSM  |       //Software Slave control
      SPI_CR1_SSI  |       //Internal Slave select 
  //  SPI_CR1_LSBFIRST |   //LSB First
  //  SPI_CR1_BR_0 |       //Prescaller
      SPI_CR1_BR_1 |
  //  SPI_CR1_BR_2 |
      SPI_CR1_MSTR |       //Master mode
  //  SPI_CR1_CPOL |
  //  SPI_CR1_CPHA |
  //  SPI_CR1_SPE |        //SPI Enable
      0;
  }

  NRF24_SPI->CR2 |= 
  //  SPI_CR2_TXEIE |      //Tx Empty Interrupt Enable
  //  SPI_CR2_RXNEIE |     //Rx Not Empty Interrupt Enable
  //  SPI_CR2_ERRIE |      //Error Interrupt Enable
  //  SPI_CR2_SSOE |       //SS Output Enable
      SPI_CR2_TXDMAEN |    //Enable DMA Tx
      SPI_CR2_RXDMAEN |    //Enable DMA Rx
      0;

  NRF24_SPI->CR1 |= SPI_CR1_SPE;

  RCC->NRF24_SPI_DMA_CLK_R |= NRF24_SPI_DMA_CLK_E;

  NRF24_SPI_DMA->IFCR |=
    (NRF24_SPI_DMA_TX_FGL | NRF24_SPI_DMA_TX_FTC | NRF24_SPI_DMA_TX_FHT | NRF24_SPI_DMA_TX_FTC) |
    (NRF24_SPI_DMA_RX_FGL | NRF24_SPI_DMA_RX_FTC | NRF24_SPI_DMA_RX_FHT | NRF24_SPI_DMA_RX_FTC);

  NRF24_SPI_DMA_TX_CHANNEL->CPAR  = (U32)&NRF24_SPI->DR;
  NRF24_SPI_DMA_TX_CHANNEL->CMAR  = (U32)0;
  NRF24_SPI_DMA_TX_CHANNEL->CNDTR = 0;
  NRF24_SPI_DMA_TX_CHANNEL->CCR   = DMA_CCR_MINC | DMA_CCR_DIR;
  
  NRF24_SPI_DMA_RX_CHANNEL->CPAR  = (U32)&NRF24_SPI->DR;
  NRF24_SPI_DMA_RX_CHANNEL->CMAR  = (U32)0;
  NRF24_SPI_DMA_RX_CHANNEL->CNDTR = 0;
  NRF24_SPI_DMA_RX_CHANNEL->CCR   = DMA_CCR_MINC;

  //HY1P8TFT_SPI_DMA_TX_CHANNEL->CCR  |= HY1P8TFT_SPI_DMA_TX_EN;
  //HY1P8TFT_SPI_DMA_RX_CHANNEL->CCR  |= HY1P8TFT_SPI_DMA_RX_EN;

//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
//  
//  //Enable SPI DMA IRQn Interrupt
//  NVICI.NVIC_IRQChannel = HY1P8TFT_SPI_DMA_RX_IRQN;
//  NVICI.NVIC_IRQChannelPreemptionPriority = 0;
//  NVICI.NVIC_IRQChannelSubPriority = 0;
//  NVICI.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVICI);

  nRF24L01P_CE_Lo();
  nRF24L01P_CSN_Hi();
}

//-----------------------------------------------------------------------------
void nRF24L01P_Exchange(U8 * txData, U8 * rxData, U32 aSize)
{
  U8 Dummy = 0xFF;

  if ( ((NULL == txData) && (NULL == rxData)) || (0 == aSize) ) return;

  //SPI Direction to Output
  //NRF24_SPI->CR1 |= SPI_CR1_BIDIOE;
  //SPI Data Frame Format 8 bit
  //NRF24_SPI->CR1 &= ~SPI_CR1_DFF;

  //Disable DMA Channels
  NRF24_SPI_DMA_TX_CHANNEL->CCR &= ~DMA_CCR_EN;
  NRF24_SPI_DMA_RX_CHANNEL->CCR &= ~DMA_CCR_EN;
  
  if ( NULL == txData )
  {
    NRF24_SPI_DMA_TX_CHANNEL->CCR  &= ~DMA_CCR_MINC;
    NRF24_SPI_DMA_TX_CHANNEL->CMAR  = (U32)&Dummy;
  }
  else
  {
    NRF24_SPI_DMA_TX_CHANNEL->CCR  |= DMA_CCR_MINC;
    NRF24_SPI_DMA_TX_CHANNEL->CMAR  = (U32)txData;
  }

  if ( NULL == rxData )
  {
    NRF24_SPI_DMA_RX_CHANNEL->CCR  &= ~DMA_CCR_MINC;
    NRF24_SPI_DMA_RX_CHANNEL->CMAR  = (U32)&Dummy;
  }
  else
  {
    NRF24_SPI_DMA_RX_CHANNEL->CCR  |= DMA_CCR_MINC;
    NRF24_SPI_DMA_RX_CHANNEL->CMAR  = (U32)rxData;
  }

  //NRF24_SPI_DMA_TX_CHANNEL->CCR &= ~(DMA_CCR_MSIZE | DMA_CCR_PSIZE);
  //NRF24_SPI_DMA_TX_CHANNEL->CCR |= (DMA_CCR_MSIZE08 | DMA_CCR_PSIZE08);

  //Clear DMA Flags
  NRF24_SPI_DMA->IFCR |= 
    (NRF24_SPI_DMA_TX_FGL | NRF24_SPI_DMA_TX_FTC | NRF24_SPI_DMA_TX_FHT | NRF24_SPI_DMA_TX_FTC |
     NRF24_SPI_DMA_RX_FGL | NRF24_SPI_DMA_RX_FTC | NRF24_SPI_DMA_RX_FHT | NRF24_SPI_DMA_RX_FTC);

  //Setup DMA Channels' Sizes
  NRF24_SPI_DMA_TX_CHANNEL->CNDTR = aSize;
  NRF24_SPI_DMA_RX_CHANNEL->CNDTR = aSize;

  //Enable DMA Channels
  NRF24_SPI_DMA_RX_CHANNEL->CCR |= DMA_CCR_EN;
  NRF24_SPI_DMA_TX_CHANNEL->CCR |= DMA_CCR_EN;

  //Wait to transmit last data
  while(!(NRF24_SPI->SR & SPI_SR_TXE));

  //Wait until not busy (infinite loop here: SPI2_SR = 0x06c3)
  while(NRF24_SPI->SR & SPI_SR_BSY);
}
