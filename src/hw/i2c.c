#include "i2c.h"
#include "gpio.h"

typedef struct sI2CTransAction
{
  I2C_TypeDef * I2C;
  U8            Address;
  U32           Size;
  U8          * Buffer;
  U32           Index;
} tI2CTransAction, * pI2CTransAction;

#define RD  (U8)0x01
#define WR  (U8)0x00

tI2CTransAction I2C1TA =
{
  I2C1,
  0,
  0,
  (void *)0,
  0,
};

tI2CTransAction I2C2TA =
{
  I2C2,
  0,
  0,
  (void *)0,
  0,
};

static U32 OneExchTimeout = 1000;

/** @brief Reads buffer of bytes from I2C slave device
  * @param Pointer to I2C instance
  * @param Slave device address
  * @param Pointer to where data will be placed
  * @param Number of bytes to be read
  * @return Number of bytes read
  */
U32 I2C_MasterRead(I2C_TypeDef * pI2C, U8 aAddress, U8 * pBuffer, U32 aSize)
{
  pI2CTransAction pI2CTA;
  U32 to = OneExchTimeout * aSize;

  if (pI2C == I2C1)
  {
    pI2CTA = &I2C1TA;
  }
  else
  {
    pI2CTA = &I2C2TA;
  }

  pI2CTA->Address = (aAddress << 1) | RD;
  pI2CTA->Buffer = pBuffer;
  pI2CTA->Size = aSize;
  pI2CTA->Index = 0;

  /* Enable I2C interrupts (EVT, ERR, BUF) */
  pI2CTA->I2C->CR2 |= (I2C_CR2_ITEVTEN | I2C_CR2_ITERREN | I2C_CR2_ITBUFEN);
  /* Send START condition */
  pI2CTA->I2C->CR1 |= I2C_CR1_START;
  /* Wait until the START condition is generated on the bus:
   *  START bit is cleared by hardware */
  while ((pI2CTA->I2C->CR1 & I2C_CR1_START) == I2C_CR1_START);
  /* Wait until BUSY flag is reset (until a STOP is generated) */
  while (((pI2CTA->I2C->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY) && (0 < to--));
  /* Enable Acknowledgement to be ready for another reception */
  pI2CTA->I2C->CR1 |= I2C_CR1_ACK;

  return pI2CTA->Index;
}

/** @brief Writes number of bytes to I2C slave device
  * @param Pointer to I2C instance
  * @param Address of I2C slave device
  * @param Buffer to write to slave
  * @param Number of bytes to be written
  * @return Number of bytes written
  */
U32 I2C_MasterWrite(I2C_TypeDef * pI2C, U8 aAddress, U8 * pBuffer, U32 aSize)
{
  pI2CTransAction pI2CTA;
  U32 to = OneExchTimeout * aSize;

  if (pI2C == I2C1)
  {
    pI2CTA = &I2C1TA;
  }
  else
  {
    pI2CTA = &I2C2TA;
  }

  pI2CTA->Address = (aAddress << 1) | WR;
  pI2CTA->Buffer = pBuffer;
  pI2CTA->Size = aSize;
  pI2CTA->Index = 0;

  /* Enable I2C Interrupts */
  pI2CTA->I2C->CR2 |= (I2C_CR2_ITEVTEN | I2C_CR2_ITERREN | I2C_CR2_ITBUFEN);
  /* Send START condition */
  pI2CTA->I2C->CR1 |= I2C_CR1_START;
  /* Wait until the START condition is generated on the bus:
   *  START bit is cleared by hardware */
  while ((pI2CTA->I2C->CR1 & I2C_CR1_START) == I2C_CR1_START);
  /* Wait until BUSY flag is reset: a STOP has been generated on the bus
   *  signaling the end of transmission */
  while (((pI2CTA->I2C->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY) && (0 < to--));

  return pI2CTA->Index;
}

/** @brief Initializes I2C peripheral
  * @param Pointer to I2C instance
  * @param I2C event interrupt priority
  * @param I2C error interrupt priority
  * @return None
  */
void I2C_Init(I2C_TypeDef * pI2C, U32 evtIrqPrio, U32 errIrqPrio)
{
  if (pI2C == I2C1)
  {
    NVIC_SetPriority(I2C1_EV_IRQn, evtIrqPrio); 
    NVIC_EnableIRQ(I2C1_EV_IRQn);

    NVIC_SetPriority(I2C1_ER_IRQn, errIrqPrio); 
    NVIC_EnableIRQ(I2C1_ER_IRQn);

    /* Enable stopping of I2C while dubug */
    DBGMCU->CR |= DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT;

    /* Enable I2C1 clock */
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    /* Enable I2C1 Reset State */
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    /* Release I2C1 from Reset State */
    RCC->APB1RSTR &= ~(RCC_APB1RSTR_I2C1RST);
  }
  else /* pI2C == I2C2 */
  {
    NVIC_SetPriority(I2C2_EV_IRQn, evtIrqPrio); 
    NVIC_EnableIRQ(I2C2_EV_IRQn);

    NVIC_SetPriority(I2C2_ER_IRQn, errIrqPrio); 
    NVIC_EnableIRQ(I2C2_ER_IRQn);

    /* Enable stopping of I2C while dubug */
    DBGMCU->CR |= DBGMCU_CR_DBG_I2C1_SMBUS_TIMEOUT;

    /* Enable I2C2 clock */
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    /* Enable I2C2 Reset State */
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
    /* Release I2C2 from Reset State */
    RCC->APB1RSTR &= ~(RCC_APB1RSTR_I2C2RST);
  }

  /* Master mode is selected as soon as the Start condition is generated
   *  on the bus with a START bit.
   * The following is the required sequence in master mode.
   *  1. Program the peripheral input clock in I2C_CR2 Register in order
   *     to generate correct timings
   *     (PCLK1 = 36 MHz) */
  pI2C->CR2 &= ~(I2C_CR2_FREQ);
  pI2C->CR2 |= 36;
  /*  2. Configure the clock control registers for Fast Mode 400 kHz
   *     T_PCLK1 = 1/36000000 = 27.78 ns
   *     T_High/T_Low = 33%/66% = 1/400000/3 = 833 ns
   *     CCR = T_High / T_PCLK1 = 0.000000833/0.00000002778 ~= 30 */
  pI2C->CCR = I2C_CCR_FS | 30;
  /*  3. Configure the rise time register
   *     T_Rise = 300 ns (for Fast Mode 400 kHz)
   *     TRISE = T_Rise/T_PCLK1 + 1 = 0.0000003/0.00000002778 + 1 = 11 */
  pI2C->TRISE = 11;
  /*  4. Program the I2C_CR1 register to enable the peripheral */
  pI2C->CR1 |= I2C_CR1_PE;
  /*  5. Set the START bit in the I2C_CR1 register to generate a Start
   *     condition */
  if (1000 == OneExchTimeout)
  {
    OneExchTimeout = 36000000 / 400000 * 9;
  }
}

/** @brief Handles I2C Event interrupt request
  * @param Pointer to I2C instance
  * @return None
  */
void I2C_EV_IRQHandler(I2C_TypeDef * pI2C)
{
  pI2CTransAction pI2CTA;
  volatile unsigned short SR1   = 0;
  volatile unsigned short SR2   = 0;
  volatile unsigned long  Event = 0;
  
  if (pI2C == I2C1)
  {
    pI2CTA = &I2C1TA;
  }
  else
  {
    pI2CTA = &I2C2TA;
  }

  /* Read the I2Cx status register */
  SR1 = pI2CTA->I2C->SR1;
  SR2 = pI2CTA->I2C->SR2;

  /* EV5 = I2C master sent a START on the bus (SB == 1) */
  if ((SR1 & I2C_SR1_SB) == I2C_SR1_SB)
  {
    /* Send the slave address for transmition or for reception
     *  (according to the configured value in the write master
     *  write routine) */
    pI2CTA->I2C->DR = pI2CTA->Address;
    SR1 = 0;
    SR2 = 0;
  }

  /* Master mode (MSL == 1) */  
  if ((SR2 & I2C_SR2_MSL) == I2C_SR2_MSL)
  {
    /* EV6 = I2C slave address acknowlaged (ADDR = 1) */
    if ((SR1 & I2C_SR1_ADDR) == I2C_SR1_ADDR)
    {
      /* Master is Transmitter (TRA == 1) */
      if ((SR2 & I2C_SR2_TRA) == I2C_SR2_TRA)
      {
        /* EV8_1 = I2C Shift register EMPTY, Data Register EMPTY (TXE == 1) */
        /* Initialize the Transmit counter */
        pI2CTA->Index = 0;
        /* Write the first data in the data register */
        pI2CTA->I2C->DR = pI2CTA->Buffer[pI2CTA->Index++];
        
        /* EV8 = I2C Shift register NOT EMPTY, Data Register
         *  EMPTY (TXE == 1). Decrement the number of bytes to be written */
        pI2CTA->Size--;
        /* If no further data to be sent, disable the I2C BUF IT in order to
         *  not have a TxE interrupt */
        if (pI2CTA->Size == 0)
        {
          pI2CTA->I2C->CR2 &= ~I2C_CR2_ITBUFEN;
        }
      }
      /* Master is Receiver (TRA == 0) */
      else
      {
        /* Initialize Receive counter */
        pI2CTA->Index = 0;
        /* At this stage, ADDR is cleared because both SR1 and SR2 were read.
         * EV6_1: used for single byte reception. The ACK disable and the STOP
         *  Programming should be done just after ADDR is cleared. */
        if (pI2CTA->Size == 1)
        {
          /* Clear ACK */
          pI2CTA->I2C->CR1 &= ~I2C_CR1_ACK;
          /* Program the STOP */
          pI2CTA->I2C->CR1 |= I2C_CR1_STOP;
        }
      }
      SR1 = 0;
      SR2 = 0;
    }

    /* EV8 = I2C Shift register NOT EMPTY, Data Register EMPTY (TXE == 1) */
    if ((SR1 & (I2C_SR1_TXE | I2C_SR1_BTF)) == I2C_SR1_TXE)
    {
      /* Master transmits the remaing data: from Data2 until the last one */
      /* If there is still data to write */
      if (pI2CTA->Size != 0)
      {
        /* Write the data in DR register */
        pI2CTA->I2C->DR = pI2CTA->Buffer[pI2CTA->Index++];
        /* Decrment the number of data to be written */
        pI2CTA->Size--;
        /* If  no data remains to write, disable the BUF IT in order to not
         *  have again a TxE interrupt */
        if (pI2CTA->Size == 0)
        {
          /* Disable the BUF IT */
          pI2CTA->I2C->CR2 &= ~I2C_CR2_ITBUFEN;
        }
      }
      SR1 = 0;
      SR2 = 0;
    }

    /* EV8_2 = I2C Shift register EMPTY, Data Register EMPTY
     *  (TXE == 1, BTF == 1) */
    if ((SR1 & (I2C_SR1_TXE | I2C_SR1_BTF)) == (I2C_SR1_TXE | I2C_SR1_BTF))
    {
      /* Program the STOP */
      pI2CTA->I2C->CR1 |= I2C_CR1_STOP;
      /* Disable EVT IT In order to not have again a BTF IT */
      pI2CTA->I2C->CR2 &= ~I2C_CR2_ITEVTEN;
      SR1 = 0;
      SR2 = 0;
    }

    /* EV7 = I2C Data Register NOT EMPTY (RXNE == 1) */
    if ((SR1 & I2C_SR1_RXNE) == I2C_SR1_RXNE)
    {
      /* Read the data register */
      pI2CTA->Buffer[pI2CTA->Index++] = pI2CTA->I2C->DR;
      /* Decrement the number of bytes to be read */
      pI2CTA->Size--;
      /* EV7_1 = If it remains only one byte to read, disable ACK and
       *  program the STOP (RXNE == 1) */
      if (pI2CTA->Size == 1)
      {
        /* Clear ACK */
        pI2CTA->I2C->CR1 &= ~I2C_CR1_ACK;
        /* Program the STOP */
        pI2CTA->I2C->CR1 |= I2C_CR1_STOP;
      }
      SR1 = 0;
      SR2 = 0;
    }
  }
}

/** @brief Handles I2C Error interrupt request
  * @param Pointer to I2C instance
  * @return None
  */
void I2C_ER_IRQHandler(I2C_TypeDef * pI2C)
{
  pI2CTransAction pI2CTA;
  volatile unsigned short SR1 = 0;

  if (pI2C == I2C1)
  {
    pI2CTA = &I2C1TA;
  }
  else
  {
    pI2CTA = &I2C2TA;
  }

  SR1 = pI2CTA->I2C->SR1;

  /* AF == 1 */
  if ((SR1 & I2C_SR1_AF) == I2C_SR1_AF)
  {
    pI2CTA->I2C->SR1 &= ~I2C_SR1_AF;
    SR1 = 0;
  }

  /* ARLO == 1 */
  if ((SR1 & I2C_SR1_ARLO) == I2C_SR1_ARLO)
  {
    pI2CTA->I2C->SR1 &= ~I2C_SR1_AF;
    SR1 = 0;
  }

  /* BERR == 1 */
  if ((SR1 & I2C_SR1_BERR) == I2C_SR1_BERR)
  {
    pI2CTA->I2C->SR1 &= ~I2C_SR1_BERR;
    SR1 = 0;
  }

  /* OVR == 1 */
  if ((SR1 & I2C_SR1_OVR) == I2C_SR1_OVR)
  {
    pI2CTA->I2C->SR1 &= ~I2C_SR1_OVR;
    SR1 = 0;
  }
}
