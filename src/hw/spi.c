#include <stdio.h>
#include "stm32f1xx.h"
#include "types.h"
#include "interrupts.h"
#include "system.h"
#include "spi.h"

//-----------------------------------------------------------------------------

typedef struct SPI_Context_s
{
  SPI_TypeDef         * HW;
  DMA_Channel_TypeDef * TxDMA;
  DMA_Channel_TypeDef * RxDMA;
  U32                   DMAClrMask;
  U8                  * TxBuffer;
  U32                   TxSize;
  U8                  * RxBuffer;
  U32                   RxSize;
  SPI_CbComplete_t      CbComplete;
  U8                    Dummy;
} SPI_Context_t, * SPI_Context_p;

//-----------------------------------------------------------------------------

static SPI_Context_t gSPICtx[SPI_COUNT] =
{
  {
    .HW         = SPI1,
    .TxDMA      = DMA1_Channel3,
    .RxDMA      = DMA1_Channel2,
    .DMAClrMask = (DMA_IFCR_CTCIF4 | DMA_IFCR_CTEIF2 | DMA_IFCR_CHTIF2 |
                   DMA_IFCR_CGIF2 | DMA_IFCR_CTCIF3 | DMA_IFCR_CTEIF3 |
                   DMA_IFCR_CHTIF3 | DMA_IFCR_CGIF3),
    .TxBuffer   = NULL,
    .TxSize     = 0,
    .RxBuffer   = NULL,
    .RxSize     = 0,
    .CbComplete = NULL,
    .Dummy      = 0xFF,
  },
  {
    .HW         = SPI2,
    .TxDMA      = DMA1_Channel5,
    .RxDMA      = DMA1_Channel4,
    .DMAClrMask = (DMA_IFCR_CTCIF4 | DMA_IFCR_CTEIF4 | DMA_IFCR_CHTIF4 |
                   DMA_IFCR_CGIF4 | DMA_IFCR_CTCIF5 | DMA_IFCR_CTEIF5 |
                   DMA_IFCR_CHTIF5 | DMA_IFCR_CGIF5),
    .TxBuffer   = NULL,
    .TxSize     = 0,
    .RxBuffer   = NULL,
    .RxSize     = 0,
    .CbComplete = NULL,
    .Dummy      = 0xFF,
  },
};

//-----------------------------------------------------------------------------
/** @brief Initializes the SPI peripheral
 *  @param aSPI - A number of SPI peripheral
 *  @param pCbComplete - Pointer to the SPI complete callback function
 *  @return None
 */

void SPI_Init(SPI_t aSPI, SPI_CbComplete_t pCbComplete)
{
  SPI_Context_p SPI = &gSPICtx[aSPI];

  /* Setup IRQ Callback */
  SPI->CbComplete = pCbComplete;

  /* Enable DMA clock */
  RCC->AHBENR |= RCC_AHBENR_DMA1EN;

  if (SPI_1 == aSPI)
  {
    /* PCLK2 = HCLK */
    RCC->CFGR |= (U32)RCC_CFGR_PPRE2_DIV8;

    /* Enable SPI clock */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* 72 MHz / 16 = 4.5 MHz */
    SPI->HW->CR1 |=
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

    /* Enable SPI Interrupts */
    IRQ_SPI1_Enable();
  }
  else
  {
    /* Enable SPI clock */
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    /* 36 MHz / 8 = 4.5 MHz */
    SPI->HW->CR1 |=
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

    /* Enable SPI Interrupts */
    IRQ_SPI2_Enable();
  }

  /* Enable DMA for SPI */
  SPI->HW->CR2 |=
       //  SPI_CR2_TXEIE |      //Tx Empty Interrupt Enable
       //  SPI_CR2_RXNEIE |     //Rx Not Empty Interrupt Enable
       //  SPI_CR2_ERRIE |      //Error Interrupt Enable
       //  SPI_CR2_SSOE |       //SS Output Enable
           SPI_CR2_TXDMAEN |    //Enable DMA Tx
           SPI_CR2_RXDMAEN |    //Enable DMA Rx
           0;

  /* Clear DMA flags */
  DMA1->IFCR = SPI->DMAClrMask;

  /* Setup DMA */
  SPI->TxDMA->CPAR  = (U32)&SPI->HW->DR;
  SPI->TxDMA->CMAR  = (U32)0;
  SPI->TxDMA->CNDTR = 0;
  SPI->TxDMA->CCR   = DMA_CCR_MINC | DMA_CCR_DIR;

  SPI->RxDMA->CPAR  = (U32)&SPI->HW->DR;
  SPI->RxDMA->CMAR  = (U32)0;
  SPI->RxDMA->CNDTR = 0;
  SPI->RxDMA->CCR   = DMA_CCR_MINC;

  /* Enable SPI */
  SPI->HW->CR1 |= SPI_CR1_SPE;
}

//-----------------------------------------------------------------------------
/** @brief Starts the SPI data exchange transaction
 *  @param aSPI - A number of the SPI peripheral
 *  @param pTx - Pointer to the data to be transmitted
 *  @param pRx - Pointer to the data to be received
 *  @param aSize - Size of the data to be exchanged
 *  @return None
 */

void SPI_MExchange(SPI_t aSPI, U8 * pTx, U8 * pRx, U32 aSize)
{
  SPI_Context_p SPI = &gSPICtx[aSPI];

  if ( ((NULL == pTx) && (NULL == pRx)) || (0 == aSize) ) return;

  /* Disable DMA Channels */
  SPI->TxDMA->CCR &= ~(DMA_CCR_EN);
  SPI->RxDMA->CCR &= ~(DMA_CCR_EN);

  if (NULL == pTx)
  {
    SPI->Dummy = 0xFF;
    SPI->TxDMA->CCR  &= ~(DMA_CCR_MINC);
    SPI->TxDMA->CMAR  = (U32)&SPI->Dummy;
  }
  else
  {
    SPI->TxDMA->CCR  |= (DMA_CCR_MINC);
    SPI->TxDMA->CMAR  = (U32)pTx;
  }

  if (NULL == pRx)
  {
    SPI->Dummy = 0x00;
    SPI->RxDMA->CCR  &= ~(DMA_CCR_MINC);
    SPI->RxDMA->CMAR  = (U32)&SPI->Dummy;
  }
  else
  {
    SPI->RxDMA->CCR  |= (DMA_CCR_MINC);
    SPI->RxDMA->CMAR  = (U32)pRx;
  }

  /* Clear DMA Flags */
  DMA1->IFCR = SPI->DMAClrMask;

  /* Setup DMA Channels' Sizes */
  SPI->TxDMA->CNDTR = aSize;
  SPI->RxDMA->CNDTR = aSize;

  /* Enable DMA Channels */
  SPI->RxDMA->CCR |= (DMA_CCR_TCIE | DMA_CCR_EN);
  SPI->TxDMA->CCR |= (DMA_CCR_EN);
}

//-----------------------------------------------------------------------------
/** @brief Handles the SPI interrupts
 *  @param aSPI - A number of the SPI peripheral
 *  @return None
 */

void SPI_IrqHandler(SPI_t aSPI)
{
  SPI_Context_p SPI = &gSPICtx[aSPI];

  SPI->TxDMA->CCR &= ~(DMA_CCR_EN);
  SPI->RxDMA->CCR &= ~(DMA_CCR_EN);
  DMA1->IFCR = SPI->DMAClrMask;

  if (NULL != SPI->CbComplete)
  {
    SPI->CbComplete(FW_COMPLETE);
  }
}

//-----------------------------------------------------------------------------
/** @brief DeInitializes the SPI peripheral
 *  @param aSPI - A number of the SPI peripheral
 *  @return None
 */

void SPI_DeInit(SPI_t aSPI)
{
  //
}

//-----------------------------------------------------------------------------

void SPI_SetBaudRate(U32 value)
{
//  U32 brr, mantissa, fraction;
//
//  switch (aUART)
//  {
//    case UART1:
//      /* Pre-get clock */
//      brr = APB2Clock / aValue;
//      break;
//
//    case UART2:
//      /* Pre-get clock */
//      brr = APB1Clock / aValue;
//      break;
//
//    case UART3:
//      /* Pre-get clock */
//      brr = APB1Clock / aValue;
//      break;
//  }
//
//  /* Setup Baud Rate */
//  gUARTCtx[aUART].BaudRate = aValue;
//
//  mantissa = brr / 16;
//  fraction = brr % 16;
//  brr = (mantissa << 4) | fraction;
//
//  gUARTCtx[aUART].HW->BRR = brr;
}

U32 SPI_GetBaudRate(SPI_t aSPI) //, U8 * pTx, U8 * pRx, U32 aSize)
{
  //SPI_Context_p SPI = &gSPICtx[aSPI];
  return 0;
}

//-----------------------------------------------------------------------------

U8 SPI_GetLatestXferValue(SPI_t aSPI)
{
  SPI_Context_p SPI = &gSPICtx[aSPI];
  return SPI->Dummy;
}

//-----------------------------------------------------------------------------
/** @brief Sets the SPI prescaler
 *  @param aSPI - A number of the SPI peripheral
 *  @param value - Prescaler value
 *  @return True - in case of success
 */
FW_BOOLEAN SPI_SetBaudratePrescaler(SPI_t aSPI, U16 value)
{
  FW_BOOLEAN result = FW_FALSE;
  SPI_Context_p SPI = &gSPICtx[aSPI];
  U32 prescaler = 0;

  if ((2 <= value) && (256 >= value) && (0 == (value & (value - 1))))
  {
    prescaler = (30 - __CLZ(value));

    SPI->HW->CR1 &= (U32)~(SPI_CR1_BR);
    SPI->HW->CR1 |= ((prescaler << SPI_CR1_BR_Pos) & SPI_CR1_BR_Msk);
    result = FW_TRUE;
  }

  return result;
}

//-----------------------------------------------------------------------------
