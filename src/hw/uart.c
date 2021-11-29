#include "types.h"
#include "system.h"
#include "interrupts.h"
#include "uart.h"
#include "gpio.h"
#include "debug.h"

//-----------------------------------------------------------------------------

#define UART_DEBUG

#ifdef UART_DEBUG
#  define UART_LOG           DBG
#else
#  define UART_LOG(...)
#endif

#define UART_BAUDRATE        (115200)

//-----------------------------------------------------------------------------

typedef struct UART_Context_s
{
  USART_TypeDef * HW;
  U32             BaudRate;
  UART_CbByte     RxByteCb;
  UART_CbByte     RxCmpltCb;
  UART_CbByte     TxByteCb;
  UART_CbByte     TxCmpltCb;
} UART_Context_t, * UART_Context_p;

//-----------------------------------------------------------------------------

static UART_Context_t gUARTCtx[UARTS_COUNT] =
{
  {
    .HW = USART1,
    .BaudRate = UART_BAUDRATE,
    .RxByteCb = NULL,
    .TxByteCb = NULL
  },
  {
    .HW = USART2,
    .BaudRate = UART_BAUDRATE,
    .RxByteCb = NULL,
    .TxByteCb = NULL
  },
  {
    .HW = USART3,
    .BaudRate = UART_BAUDRATE,
    .RxByteCb = NULL,
    .TxByteCb = NULL
  },
};

//-----------------------------------------------------------------------------
/** @brief Changes UART baud rate
 *  @param aUART - UART Port Number
 *  @param aValue - Baud rate
 *  @return None
 */

void UART_SetBaudrate(UART_t aUART, U32 aValue)
{
  U32 brr, mantissa, fraction;

  switch (aUART)
  {
    case UART1:
      /* Pre-get clock */
      brr = APB2Clock / aValue;
      break;

    case UART2:
      /* Pre-get clock */
      brr = APB1Clock / aValue;
      break;

    case UART3:
      /* Pre-get clock */
      brr = APB1Clock / aValue;
      break;
  }

  /* Setup Baud Rate */
  gUARTCtx[aUART].BaudRate = aValue;

  mantissa = brr / 16;
  fraction = brr % 16;
  brr = (mantissa << 4) | fraction;

  gUARTCtx[aUART].HW->BRR = brr;
}

//-----------------------------------------------------------------------------
/** @brief Initializes the UART peripheral
 *  @param aUART - UART Port Number
 *  @param aBaudRate - Baud Rate
 *  @param pRxByteCb - Callback, called when byte is received.
 *                     Result is ignored. Called in IRQ context
 *  @param pTxByteCb - Callback, called to get byte that need to be transmitted
 *                     Called in IRQ context
 *                     If returns FW_TRUE - continue transmiting
 *                     If returns FW_FALSE - transmiting stops
 *  @return None
 */

void UART_Init
(
  UART_t      aUART,
  U32         aBaudRate,
  UART_CbByte pRxByteCb,
  UART_CbByte pRxCmpltCb,
  UART_CbByte pTxByteCb,
  UART_CbByte pTxCmpltCb
)
{
  switch (aUART)
  {
    case UART1:
      /* Enable UART peripheral clock */
      RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

      /* Reset UART peripheral */
      RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
      RCC->APB2RSTR &= (~RCC_APB2RSTR_USART1RST);

      /* Enable UART Interrupts */
      IRQ_USART1_Enable();

      break;
    case UART2:
      /* Enable UART peripheral clock */
      RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

      /* Reset UART peripheral */
      RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
      RCC->APB1RSTR &= (~RCC_APB1RSTR_USART2RST);

      /* Enable UART Interrupts */
      IRQ_USART2_Enable();

      break;
    case UART3:
      /* Enable UART peripheral clock */
      RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

      /* Reset UART peripheral */
      RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;
      RCC->APB1RSTR &= (~RCC_APB1RSTR_USART3RST);

      /* Enable UART Interrupts */
      IRQ_USART3_Enable();

      break;
  }

  /* Setup Callbacks */
  gUARTCtx[aUART].RxByteCb  = pRxByteCb;
  gUARTCtx[aUART].RxCmpltCb = pRxCmpltCb;
  gUARTCtx[aUART].TxByteCb  = pTxByteCb;
  gUARTCtx[aUART].TxCmpltCb = pTxCmpltCb;

  /* Setup Baud Rate */
  UART_SetBaudrate(aUART, aBaudRate);

  /* Setup Control Registers */
  gUARTCtx[aUART].HW->CR2 = ( 0 );
  gUARTCtx[aUART].HW->CR3 = ( 0 );
  gUARTCtx[aUART].HW->CR1 = ( USART_CR1_UE );
}

//-----------------------------------------------------------------------------
/** @brief DeInitializes the UART peripheral
 *  @param aUART - UART Port Number
 *  @return None
 */

void UART_DeInit(UART_t aUART)
{
  switch (aUART)
  {
    case UART1:
      /* Disable UART Interrupts */
      IRQ_USART1_Disable();

      /* Reset UART peripheral */
      RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
      RCC->APB2RSTR &= (~RCC_APB2RSTR_USART1RST);

      /* Disable UART peripheral clock */
      RCC->APB2ENR &= (~RCC_APB2ENR_USART1EN);

      break;
    case UART2:
      /* Disable UART Interrupts */
      IRQ_USART2_Disable();

      /* Reset UART peripheral */
      RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
      RCC->APB1RSTR &= (~RCC_APB1RSTR_USART2RST);

      /* Disable UART peripheral clock */
      RCC->APB1ENR &= (~RCC_APB1ENR_USART2EN);

      break;
    case UART3:
      /* Disable UART Interrupts */
      IRQ_USART3_Disable();

      /* Reset UART peripheral */
      RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;
      RCC->APB1RSTR &= (~RCC_APB1RSTR_USART3RST);

      /* Disable UART peripheral clock */
      RCC->APB1ENR &= (~RCC_APB1ENR_USART3EN);

      break;
  }

  gUARTCtx[aUART].BaudRate = UART_BAUDRATE;
  gUARTCtx[aUART].RxByteCb = NULL;
  gUARTCtx[aUART].TxByteCb = NULL;
}

//-----------------------------------------------------------------------------
/** @brief UART Interrupt Handler
 *  @param aUART - UART Port Number
 *  @return None
 */

void UART_IrqHandler(UART_t aUART)
{
  U32 sr     = gUARTCtx[aUART].HW->SR;
  U32 cr1    = gUARTCtx[aUART].HW->CR1;
  U32 cr3    = gUARTCtx[aUART].HW->CR3;
  U32 errors = 0;
  U16 data   = 0;

  /* If no error occurs */
  errors = (sr & (USART_SR_PE | USART_SR_FE | USART_SR_ORE | USART_SR_NE));
  if (0 == errors)
  {
    /* UART in mode Receiver */
    if ((0 != (sr & USART_SR_RXNE)) && (0 != (cr1 & USART_CR1_RXNEIE)))
    {
      data = gUARTCtx[aUART].HW->DR;
      if (NULL != gUARTCtx[aUART].RxByteCb)
      {
        (void)gUARTCtx[aUART].RxByteCb((U8 *)&data);
      }
    }

    /* UART Receive Complete */
    if ((0 != (sr & USART_SR_IDLE)) && (0 != (cr1 & USART_CR1_IDLEIE)))
    {
      data = gUARTCtx[aUART].HW->DR;
      if (NULL != gUARTCtx[aUART].RxCmpltCb)
      {
        (void)gUARTCtx[aUART].RxCmpltCb(NULL);
      }
    }
  }
  else
  {
    if (0 != (sr & USART_SR_PE))
    {
      UART_LOG("UART: Parity Error!\r\n");
    }
    if (0 != (sr & USART_SR_NE))
    {
      UART_LOG("UART: Noise Error!\r\n");
    }
    if (0 != (sr & USART_SR_FE))
    {
      UART_LOG("UART: Frame Error!\r\n");
    }
    if (0 != (sr & USART_SR_ORE))
    {
      UART_LOG("UART: Overrun Error!\r\n");
    }
    data = gUARTCtx[aUART].HW->DR;

//    /* If some errors occur */
//    if ( (0 != (cr3 & USART_CR3_EIE)) ||
//         (0 != (cr1 & (USART_CR1_RXNEIE | USART_CR1_PEIE))) )
//    {
//      /* UART parity error interrupt occurred */
//      if ((0 != (sr & USART_SR_PE)) && (0 != (cr1 & USART_CR1_PEIE)))
//      {
////        huart->ErrorCode |= HAL_UART_ERROR_PE;
//      }
//
//      /* UART noise error interrupt occurred */
//      if ((0 != (sr & USART_SR_NE)) && (0 != (cr3 & USART_CR3_EIE)))
//      {
////        huart->ErrorCode |= HAL_UART_ERROR_NE;
//      }
//
//      /* UART frame error interrupt occurred */
//      if ((0 != (sr & USART_SR_FE)) && (0 != (cr3 & USART_CR3_EIE)))
//      {
////        huart->ErrorCode |= HAL_UART_ERROR_FE;
//      }
//
//      /* UART Over-Run interrupt occurred */
//      if ((0 != (sr & USART_SR_ORE)) && (0 != (cr3 & USART_CR3_EIE)))
//      {
////        huart->ErrorCode |= HAL_UART_ERROR_ORE;
//      }
//
//      /* Call UART Error Call back function if needed */
//      if (huart->ErrorCode != HAL_UART_ERROR_NONE)
//      {
//        /* UART in mode Receiver */
//        if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
//        {
//          UART_Receive_IT(huart);
//        }
//
//        /* If Overrun error occurs, or if any error occurs in DMA mode reception,
//           consider error as blocking */
//        dmarequest = HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR);
//        if(((huart->ErrorCode & HAL_UART_ERROR_ORE) != RESET) || dmarequest)
//        {
//          /* Blocking error : transfer is aborted
//             Set the UART state ready to be able to start again the process,
//             Disable Rx Interrupts, and disable Rx DMA request, if ongoing */
//          UART_EndRxTransfer(huart);
//
//          /* Disable the UART DMA Rx request if enabled */
//          if(HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR))
//          {
//            CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);
//
//            /* Abort the UART DMA Rx channel */
//            if(huart->hdmarx != NULL)
//            {
//              /* Set the UART DMA Abort callback :
//                 will lead to call HAL_UART_ErrorCallback() at end of DMA abort procedure */
//              huart->hdmarx->XferAbortCallback = UART_DMAAbortOnError;
//              if(HAL_DMA_Abort_IT(huart->hdmarx) != HAL_OK)
//              {
//                /* Call Directly XferAbortCallback function in case of error */
//                huart->hdmarx->XferAbortCallback(huart->hdmarx);
//              }
//            }
//            else
//            {
//              /* Call user error callback */
//              HAL_UART_ErrorCallback(huart);
//            }
//          }
//          else
//          {
//            /* Call user error callback */
//            HAL_UART_ErrorCallback(huart);
//          }
//        }
//        else
//        {
//          /* Non Blocking error : transfer could go on.
//             Error is notified to user through user error callback */
//          HAL_UART_ErrorCallback(huart);
//          huart->ErrorCode = HAL_UART_ERROR_NONE;
//        }
//      }
//      return;
  } /* End if some error occurs */

  /* UART in mode Transmitter */
  if ((0 != (sr & USART_SR_TXE)) && (0 != (cr1 & USART_CR1_TXEIE)))
  {
    if (NULL != gUARTCtx[aUART].TxByteCb)
    {
      if (FW_TRUE == gUARTCtx[aUART].TxByteCb((U8 *)&data))
      {
        /* Transmit data */
        gUARTCtx[aUART].HW->DR = data;
      }
      else
      {
        /* Disable UART Tx Buffer Empty Interrupt */
        gUARTCtx[aUART].HW->CR1 &= ~(USART_CR1_TXEIE);
        /* Enable UART Tx Complete Interrupt */
        gUARTCtx[aUART].HW->CR1 |= (USART_CR1_TCIE);
      }
    }
    else
    {
      /* Disable UART Tx Buffer Empty Interrupt */
      gUARTCtx[aUART].HW->CR1 &= ~(USART_CR1_TE | USART_CR1_TXEIE);
    }
  }

  /* UART in mode Transmitter Complete */
  if ((0 != (sr & USART_SR_TC)) && (0 != (cr1 & USART_CR1_TCIE)))
  {
    /* Disable UART Tx Interrupts */
    gUARTCtx[aUART].HW->CR1 &= ~(USART_CR1_TE | USART_CR1_TCIE);
    if (NULL != gUARTCtx[aUART].TxCmpltCb)
    {
      (void)gUARTCtx[aUART].TxCmpltCb(NULL);
    }
  }
}

//-----------------------------------------------------------------------------
/** @brief Enables transmiting via UART using Interrupts
 *  @param aUART - UART Port Number
 *  @return None
 *  @note When Tx Data Register empty - callback function is called to get
 *        the next byte to transmit
 */

void UART_TxStart(UART_t aUART)
{
  /* Check if transmission is in progress */
  if (0 != (gUARTCtx[aUART].HW->CR1 & USART_CR1_TE)) return;

  gUARTCtx[aUART].HW->CR1 |= (USART_CR1_TE | USART_CR1_TXEIE);
  UART_LOG("UART: Tx Start\r\n");
}

//-----------------------------------------------------------------------------
/** @brief Enables receiving via UART using Interrupts
 *  @param aUART - UART Port Number
 *  @return None
 *  @note When Rx Data Register is not empty - callback function is called
 *        to put the received byte to higher layer
 */

void UART_RxStart(UART_t aUART)
{
  /* Check if reception is in progress */
  if (0 != (gUARTCtx[aUART].HW->CR1 & USART_CR1_RE)) return;

  gUARTCtx[aUART].HW->CR1 |= (USART_CR1_RXNEIE | USART_CR1_IDLEIE);
  gUARTCtx[aUART].HW->CR1 |= (USART_CR1_RE);
  UART_LOG("UART: Rx Start\r\n");
}
