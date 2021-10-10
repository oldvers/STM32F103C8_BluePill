#include <stdio.h>
#include "stm32f1xx.h"
#include "types.h"
#include "interrupts.h"
#include "system.h"
#include "i2c.h"

//-----------------------------------------------------------------------------

typedef struct I2C_Context_s
{
  I2C_TypeDef      * HW;
  U8                 Address;
  U8               * TxBuffer;
  U32                TxSize;
  U8               * RxBuffer;
  U32                RxSize;
  I2C_CbComplete_t   CbComplete;
  U32                Index;
} I2C_Context_t, * I2C_Context_p;

//-----------------------------------------------------------------------------

#define I2C_RW_MASK  ((U8)0xFE)
#define I2C_RD       ((U8)1)
#define I2C_WR       ((U8)0)

//-----------------------------------------------------------------------------

I2C_Context_t gI2CCtx[I2CS_COUNT] =
{
  {
    .HW         = I2C1,
    .Address    = 0,
    .TxBuffer   = NULL,
    .TxSize     = 0,
    .RxBuffer   = NULL,
    .RxSize     = 0,
    .CbComplete = NULL,
    .Index      = 0,
  },
  {
    .HW         = I2C2,
    .Address    = 0,
    .TxBuffer   = NULL,
    .TxSize     = 0,
    .RxBuffer   = NULL,
    .RxSize     = 0,
    .CbComplete = NULL,
    .Index      = 0,
  },
};

//-----------------------------------------------------------------------------
/** @brief Initializes the I2C peripheral
 *  @param pI2CInstance - Pointer to the I2C hardware
 *  @return None
 */

static void i2c_Init(I2C_TypeDef * pI2cInstance)
{
  U32 temp = 0;

  /* Calculate value from I2C Clock frequency (MHz) */
  temp = APB1Clock / 1000000;

  /* I2C Clock Frequency (MHz) */
  pI2cInstance->CR2 = temp;

  /* I2C SCL Duty -> 100 kHz */
  pI2cInstance->CCR = temp * 5;

  /* Rise Time -> 1000 ns */
  pI2cInstance->TRISE = temp;

  /* Enable I2C */
  pI2cInstance->CR1 |= I2C_CR1_PE;
}

//-----------------------------------------------------------------------------
/** @brief Calls the callback function and disables the interrupts
 *  @param pContext - Pointer to the I2C context
 *  @param aResult - Result of the I2C transaction
 *  @return None
 */

static void i2c_Complete(I2C_Context_t * pContext, FW_RESULT aResult)
{
  if (NULL != pContext->CbComplete)
  {
    (void)pContext->CbComplete(aResult);
  }
  pContext->HW->CR2 &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN);
}

//-----------------------------------------------------------------------------
/** @brief Handles the I2C interrupts
 *  @param pContext - Pointer to the I2C context
 *  @return None
 */

static void i2c_Irq(I2C_Context_t * pContext)
{
  U32 SR1 = pContext->HW->SR1;
  U32 SR2 = 0;

  /* --- Common ----------------------------------------------------------- */
  /* EV5 - After start -> (Rd SR1, Wr DR) */
  if (I2C_SR1_SB == (SR1 & I2C_SR1_SB))
  {
    pContext->HW->DR = pContext->Address;
  }

  /* EV6 - Address sent -> (Rd SR1, Rd SR2) */
  if (I2C_SR1_ADDR == ((SR1 & I2C_SR1_ADDR)))
  {
    SR2 = pContext->HW->SR2;

    /* Receiver mode */
    if (0 == (SR2 & I2C_SR2_TRA))
    {
      if (1 == pContext->RxSize)
      {
        pContext->HW->CR1 &= ~(I2C_CR1_ACK);
        pContext->HW->CR1 |= I2C_CR1_STOP;
      }
      else
      {
        pContext->HW->CR1 |= I2C_CR1_ACK;
      }
    }
  }

  /* --- Transmitter ------------------------------------------------------ */
  /* EV8_1/EV8 - Tx Empty -> (Wr DR) */
  if (I2C_SR1_TXE == ((SR1 & I2C_SR1_TXE)))
  {
    if (pContext->Index < pContext->TxSize)
    {
      pContext->HW->DR = pContext->TxBuffer[pContext->Index++];
    }
  }

  /* EV8_2 - Transfer complete -> (STOP/RESTART) */
  if ((I2C_SR1_BTF | I2C_SR1_TXE) == ((SR1 & (I2C_SR1_BTF | I2C_SR1_TXE))))
  {
/* TODO - Check for Rx, Modify address for read */
    pContext->HW->CR1 |= I2C_CR1_STOP;
    i2c_Complete(pContext, FW_COMPLETE);
  }

  /* --- Receiver --------------------------------------------------------- */
  /* EV7_1/EV7 - Rx Not Empty -> (Rd DR) */
  if (I2C_SR1_RXNE == ((SR1 & I2C_SR1_RXNE)))
  {
    pContext->RxBuffer[pContext->Index++] = pContext->HW->DR;

    if (pContext->Index == (pContext->RxSize - 1))
    {
      pContext->HW->CR1 &= ~(I2C_CR1_ACK);
      pContext->HW->CR1 |= I2C_CR1_STOP;
    }

    if (pContext->Index == pContext->RxSize)
    {
      i2c_Complete(pContext, FW_COMPLETE);
    }
  }
}

//-----------------------------------------------------------------------------
/** @brief Handles the I2C error interrupts
 *  @param pContext - Pointer to the I2C context
 *  @return None
 */

static void i2c_Err(I2C_Context_t * pContext)
{
  U32 SR1 = pContext->HW->SR1;

  /* Call the callback function */
  i2c_Complete(pContext, FW_FAIL);

  /* Clear all the error flags */
  pContext->HW->SR1 &= ~(I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO |
                         I2C_SR1_OVR | I2C_SR1_PECERR | I2C_SR1_TIMEOUT);

  /* Generate a stop condition on the bus */
  pContext->HW->CR1 |= I2C_CR1_STOP;
}

//-----------------------------------------------------------------------------
/** @brief Initializes the I2C peripheral
 *  @param aI2C - A number of I2C peripheral
 *  @param pCbComplete - Pointer to the callback function
 *  @return None
 */

void I2C_Init(I2C_t aI2C, I2C_CbComplete_t pCbComplete)
{
  if (I2C_1 == aI2C)
  {
    /* Enable I2C peripheral clock */
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /* Reset I2C peripheral */
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    RCC->APB1RSTR &= (~RCC_APB1RSTR_I2C1RST);

    /* Enable I2C Interrupts */
    IRQ_I2C1_Enable();
  }
  else
  {
    /* Enable I2C peripheral clock */
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

    /* Reset I2C peripheral */
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
    RCC->APB1RSTR &= (~RCC_APB1RSTR_I2C2RST);

    /* Enable I2C Interrupts */
    IRQ_I2C2_Enable();
  }

  /* Enable I2C Debug Mode */
  DBGMCU->CR |= ( DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT |
                  DBGMCU_CR_DBG_I2C2_SMBUS_TIMEOUT );

  /* Setup the callback */
  gI2CCtx[aI2C].CbComplete = pCbComplete;

  /* Init the peripheral */
  i2c_Init(gI2CCtx[aI2C].HW);
}

//-----------------------------------------------------------------------------
/** @brief Starts the I2C master write transaction
 *  @param aI2C - A number of the I2C peripheral
 *  @param aAdr - Address of the I2C device
 *  @param pTx - Pointer to the transmitted buffer
 *  @param txSize - The size of the transmitted data
 *  @return None
 */

void I2C_MWr(I2C_t aI2C, U8 aAdr, U8 * pTx, U8 txSize)
{
  I2C_Context_p pCtx = &gI2CCtx[I2C_1];

/*TODO - Check if I2C is busy*/

  pCtx->Address  = ((aAdr << 1) & I2C_RW_MASK) | I2C_WR;
  pCtx->TxBuffer = pTx;
  pCtx->TxSize   = txSize;
  pCtx->Index    = 0;

  /* Enable interrupts */
  pCtx->HW->CR2 |= (I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN);

  /* Start */
  pCtx->HW->CR1 |= I2C_CR1_START;
}

//-----------------------------------------------------------------------------
/** @brief I2C interrupt handler
 *  @param aI2C - A number of the I2C peripheral
 *  @return None
 */

void I2C_IrqHandler(I2C_t aI2C)
{
  i2c_Irq(&gI2CCtx[aI2C]);
}

//-----------------------------------------------------------------------------
/** @brief I2C error interrupt handler
 *  @param aI2C - A number of the I2C peripheral
 *  @return None
 */

void I2C_IrqError(I2C_t aI2C)
{
  i2c_Err(&gI2CCtx[aI2C]);
}

//-----------------------------------------------------------------------------
/** @brief Starts the I2C master read transaction
 *  @param aI2C - A number of the I2C peripheral
 *  @param aAdr - Address of the I2C device
 *  @param pRx - Pointer to the read data
 *  @param rxSize - Size of the data to be read
 *  @return None
 */

void I2C_MRd(I2C_t aI2C, U8 aAdr, U8 * pRx, U8 rxSize)
{
  I2C_Context_p pCtx = &gI2CCtx[I2C_1];

/*TODO - Check if I2C is busy*/

  pCtx->Address  = ((aAdr << 1) & I2C_RW_MASK) | I2C_RD;
  pCtx->RxBuffer = pRx;
  pCtx->RxSize   = rxSize;
  pCtx->Index    = 0;

  /* Enable interrupts */
  pCtx->HW->CR2 |= (I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN);

  /* Start */
  pCtx->HW->CR1 |= I2C_CR1_START;
}

//-----------------------------------------------------------------------------
/** @brief Starts the I2C wrire/read transaction
 *  @param aI2C - A number of the I2C peripheral
 *  @param aAdr - Address of the I2C device
 *  @param pTx - Pointer to the data to be written
 *  @param txSize - Size of the data to be written
 *  @param pRx - Pointer to the data to be read
 *  @param rxSize - Size of the data to be read
 *  @return None
 */

void I2C1_MEx(I2C_t aI2C, U8 aAdr, U8 * pTx, U8 txSize, U8 * pRx, U8 rxSize)
{
    //
}

//-----------------------------------------------------------------------------
/** @brief DeInitializes the I2C peripheral
 *  @param aI2C - A number of the I2C peripheral
 *  @return None
 */

void I2C1_DeInit(I2C_t aI2C)
{
    //
}

//-----------------------------------------------------------------------------
