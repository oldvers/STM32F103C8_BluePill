#include <stdio.h>
#include "stm32f1xx.h"
#include "types.h"
#include "interrupts.h"
#include "system.h"
#include "i2c.h"

////-----------------------------------------------------------------------------
//
////#define UART_DEBUG
//
//#ifdef UART_DEBUG
//#  define UART_LOG           LOG
//#else
//#  define UART_LOG(...)
//#endif
//
//#define UART_BAUDRATE        (115200)

///* Private typedef -----------------------------------------------------------*/

//typedef enum IIC_e
//{
//  I2C_1 = 0,
//  I2C_2,
//  I2C_3,
//  I2CS_COUNT
//} IIC_t;

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

//typedef struct I2C_Context_s
//{
//  I2C_TypeDef   * HW;
//  //I2C_CbByte      RxByteCb;
//  I2C_CbByte      RxCmpltCb;
//  I2C_CbByte      TxByteCb;
//  I2C_CbByte      TxCmpltCb;
////  U8            Address;
////  U32           Size;
////  U8          * Buffer;
////  U32           Index;
//} I2C_Context_t, * I2C_Context_p;

//U8 * TxBuffer;
//U8   TxLength;
//U8   TxCount;
//U8   TxDatAddr;
//U8   TxDevAddr;

//-----------------------------------------------------------------------------

#define I2C_RW_MASK  ((U8)0xFE)
#define I2C_RD       ((U8)1)
#define I2C_WR       ((U8)0)

///* Private macro -------------------------------------------------------------*/
///* Private variables ---------------------------------------------------------*/
//
//tI2CTransAction I2C1TA =
//{
//  I2C1,
//  0,
//  0,
//  (void *)0,
//  0,
//};
//
//tI2CTransAction I2C2TA =
//{
//  I2C2,
//  0,
//  0,
//  (void *)0,
//  0,
//};

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
//  {
//    .HW         = I2C3,
//    .Address    = 0,
//    .TxBuffer   = NULL,
//    .TxSize     = 0,
//    .RxBuffer   = NULL,
//    .RxSize     = 0,
//    .CbComplete = NULL,
//    .Index      = 0,
//  },
};

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

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

static void i2c_Complete(I2C_Context_t * pContext, FW_RESULT aResult)
{
  if (NULL != pContext->CbComplete)
  {
    (void)pContext->CbComplete(aResult);
  }
  pContext->HW->CR2 &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN);
}

//-----------------------------------------------------------------------------

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
/*TODO - Check for Rx, Modify address for read*/
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

static void i2c_Err(I2C_Context_t * pContext)
{
  U32 SR1 = pContext->HW->SR1;
  i2c_Complete(pContext, FW_FAIL);
  pContext->HW->CR1 |= I2C_CR1_STOP;
}

//-----------------------------------------------------------------------------

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

void I2C_IrqHandler(I2C_t aI2C)
{
  i2c_Irq(&gI2CCtx[aI2C]);
}

void I2C_IrqError(I2C_t aI2C)
{
  i2c_Err(&gI2CCtx[aI2C]);
}

//-----------------------------------------------------------------------------

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

void I2C1_MEx(U8 aAdr, U8 * pTx, U8 txSize, U8 * pRx, U8 rxSize)
{
    //
}

void I2C1_DeInit(void)
{
    //
}





////-----------------------------------------------------------------------------
///** @brief Initializes the UART peripheral
// *  @param aUART - UART Port Number
// *  @param aBaudRate - Baud Rate
// *  @param pRxByteCb - Callback, called when byte is received.
// *                     Result is ignored. Called in IRQ context
// *  @param pTxByteCb - Callback, called to get byte that need to be transmitted
// *                     Called in IRQ context
// *                     If returns FW_TRUE - continue transmiting
// *                     If returns FW_FALSE - transmiting stops
// *  @return None
// */
//


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


//void I2C_Init
//(
//  I2C_t       aI2C,
//  I2C_CbByte pRxByteCb,
//  I2C_CbByte pRxCmpltCb,
//  I2C_CbByte pTxByteCb,
//  I2C_CbByte pTxCmpltCb
//)
//{
//  U32 temp = 0;
//
//  switch (aI2C)
//  {
//    case I2C_1:
//      ////NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority)
//      ////NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority)
//      ///* 2 bits for pre-emption priority and 2 bits for subpriority */
//      ////  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//      ///* Set USART1 interrupt preemption priority to 1 */
//      //NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 0));
//      ///* Set SysTick interrupt preemption priority to 3 */
//      //NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));
//
//
//
//      /* Enable I2C peripheral clock */
//      RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
//
//      /* Reset I2C peripheral */
//      RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
//      RCC->APB1RSTR &= (~RCC_APB1RSTR_I2C1RST);
//
//      /* Enable I2C Debug Mode */
//      DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_I2C1_SMBUS_TIMEOUT;
//
//      /* Enable I2C Interrupts */
//      IRQ_I2C1_Enable();
//
//      break;
//    case I2C_2:
//      // /* Enable UART peripheral clock */
//      // RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
//      //
//      // /* Reset UART peripheral */
//      // RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
//      // RCC->APB1RSTR &= (~RCC_APB1RSTR_USART2RST);
//      //
//      // /* Enable UART Interrupts */
//      // IRQ_USART2_Enable();
//
//      break;
//    case I2C_3:
//      // /* Enable UART peripheral clock */
//      // RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
//      //
//      // /* Reset UART peripheral */
//      // RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;
//      // RCC->APB1RSTR &= (~RCC_APB1RSTR_USART3RST);
//      //
//      // /* Enable UART Interrupts */
//      // IRQ_USART3_Enable();
//
//      break;
//  }
//
//
//  /* SHOULD BE MOVED UP */
//  /* SCL - I2C1 PB6 */
//  GPIO_Init(
//    GPIOB, 6,
//    GPIO_PIN_MODE_AFN,
//    GPIO_PIN_OTYPE_OD,
//    GPIO_PIN_SPEED_LO,
//    GPIO_PIN_PUPD_PU,
//    GPIO_PIN_AF4 );
//
//  /* SDA - I2C1 PB7 */
//  GPIO_Init(
//    GPIOB, 7,
//    GPIO_PIN_MODE_AFN,
//    GPIO_PIN_OTYPE_OD,
//    GPIO_PIN_SPEED_LO,
//    GPIO_PIN_PUPD_PU,
//    GPIO_PIN_AF4 );
//
//
//  /* Setup Callbacks */
//  gI2CCtx[aI2C].RxByteCb  = pRxByteCb;
//  gI2CCtx[aI2C].RxCmpltCb = pRxCmpltCb;
//  gI2CCtx[aI2C].TxByteCb  = pTxByteCb;
//  gI2CCtx[aI2C].TxCmpltCb = pTxCmpltCb;
//
//  /* Setup Control Registers */
////  gUARTCtx[aUART].HW->CR2 = ( 0 );
////  gUARTCtx[aUART].HW->CR3 = ( 0 );
////  gUARTCtx[aUART].HW->CR1 = ( USART_CR1_UE );
//
//
//  /* Calculate value from I2C Clock frequency (MHz) */
//  temp = APB1Clock / 1000000;
//
//  /* I2C Clock Frequency (MHz) */
//  gI2CCtx[aI2C].HW->CR2 = temp;
//  /* I2C SCL Duty -> 100 kHz */
//  gI2CCtx[aI2C].HW->CCR = temp * 10;
//  /* Rise Time -> 1000 ns */
//  gI2CCtx[aI2C].HW->TRISE = temp;
//  /* Enable I2C */
//  gI2CCtx[aI2C].HW->CR1 |= I2C_CR1_PE;
//}

//printf("%X\r\n", RCC_APB1ENR->I2C3EN);

//U8 I2C_Wait(U32 aOkFlags, U32 aErrorFlags)
//{
//
//}

//U8 I2C_Error(void)
//{
//  /* Disable I2C */
//  //I2C3_CR1_PE = 0;
//
//  return 0;
//}






//U8 I2C_Wr(U8 DeviceAddr, U8 DataAddr, U8 * Data, U8 Length)
//{
//// //  U32 timeout = 25000;
////
////   TxBuffer = Data;
////   TxLength = Length;
////   TxCount = 0;
////   TxDatAddr = DataAddr;
////   TxDevAddr = DeviceAddr;
////
////   /* Enable I2C */
////   I2C3_CR1_PE = 1;
////
////   /* Enable EVT_IT and BUF_IT */
////   I2C3->CR2 |= (I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN | I2C_CR2_ITERREN);
//// //        /* Set the I2C direction to Transmission */
//// //        I2CDirection = I2C_DIRECTION_TX;
//// //        SlaveAddress &= OAR1_ADD0_Reset;
//// //        Address = SlaveAddress;
//// //        if (I2Cx == I2C1)    NumbOfBytes1 = NumByteToWrite;
//// //        else NumbOfBytes2 = NumByteToWrite;
//// //        /* Send START condition */
//// //        I2Cx->CR1 |= CR1_START_Set;
////
////
//// //NVIC_ClearPendingIRQ(IRQn_Type IRQn)
//// //NVIC_ClearPendingIRQ(IRQn_Type IRQn)
////
////   NVIC_EnableIRQ( I2C3_EV_IRQn );
////   NVIC_EnableIRQ( I2C3_ER_IRQn );
////
////   /* Start */
////   I2C3_CR1_START = 1;
////
//// //        /* Wait until the START condition is generated on the bus: the START bit is cleared by hardware */
//// //        while ((I2Cx->CR1&0x100) == 0x100);
//// //        /* Wait until BUSY flag is reset: a STOP has been generated on the bus signaling the end
//// //        of transmission */
//// //        while ((I2Cx->SR2 &0x0002) == 0x0002);
//// //    }
////
////
//// //  while ( (0 == (I2C3->SR1 & I2C_SR1_SB)) && (timeout-- > 0)) {};
//// //
//// //  /* Slave Address */
//// //  I2C3->DR = (DeviceAddr & 0xFE);
//// //  timeout = 0x5000;
//// //  while ( (0 == (I2C3->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF))) && (timeout-- > 0)) {};
//// //  if (0 == (I2C3->SR2 & I2C_SR2_TRA)) return I2C_Error();
//// //  if (1 == (I2C3->SR1 & I2C_SR1_AF)) return I2C_Error();
//// //
//// //  /* Data Address */
//// //
//// //  /* Disable I2C */
//// //  I2C3_CR1_PE = 0;
//
//  return 0;
//}

//void I2C_Rd(U8 DeviceAddr, U8 DataAddr, U8 * Data, U8 Length)
//{
//  //
//}






//void I2C1_EV_IRQHandler(void)
//{

//    __IO uint32_t SR1Register =0;
//    __IO uint32_t SR2Register =0;
//


//#ifdef SLAVE_DMA_USE
//    /* Read SR1 register */
//    SR1Register = I2C1->SR1;

//    /* If ADDR is set */
//    if ((SR1Register & 0x0002) == 0x0002)
//    {
//        /* In slave Transmitter/Receiver mode, when using DMA, it is recommended to update the buffer
//          base address and the buffer size before clearing ADDR flag. In fact, the only
//          period when the slave has control  on the bus(SCL is stretched so master can not initiate
//          transfers) is the period between ADDR is set and ADDR is cleared. Otherwise, the master can
//          initiate transfers and the buffer size & the buffer address have not yet been updated.*/

//        /* Update the DMA channels memory base address and count */
//        I2C_DMAConfig (I2C1, Buffer_Tx1, 0xFFFF, I2C_DIRECTION_TX);
//        I2C_DMAConfig (I2C1, Buffer_Rx1, 0xFFFF, I2C_DIRECTION_RX);
//        /* Clear ADDR by reading SR2 register */
//        SR2Register = I2C1->SR2;
//    }
//#else
//    /* Read the I2C1 SR1 and SR2 status registers */
//    SR1Register = I2C1->SR1;
//    SR2Register = I2C1->SR2;

//    /* If I2C1 is slave (MSL flag = 0) */
//    if ((SR2Register &0x0001) != 0x0001)
//    {
//        /* If ADDR = 1: EV1 */
//        if ((SR1Register & 0x0002) == 0x0002)
//        {
//            /* Clear SR1Register and SR2Register variables to prepare for next IT */
//            SR1Register = 0;
//            SR2Register = 0;
//            /* Initialize the transmit/receive counters for next transmission/reception
//            using Interrupt  */
//            Tx_Idx1 = 0;
//            Rx_Idx1 = 0;
//        }
//        /* If TXE = 1: EV3 */
//        if ((SR1Register & 0x0080) == 0x0080)
//        {
//            /* Write data in data register */
//            I2C1->DR = Buffer_Tx1[Tx_Idx1++];
//            SR1Register = 0;
//            SR2Register = 0;
//        }
//        /* If RXNE = 1: EV2 */
//        if ((SR1Register & 0x0040) == 0x0040)
//        {
//            /* Read data from data register */
//            Buffer_Rx1[Rx_Idx1++] = I2C1->DR;
//            SR1Register = 0;
//            SR2Register = 0;

//        }
//        /* If STOPF =1: EV4 (Slave has detected a STOP condition on the bus */
//        if (( SR1Register & 0x0010) == 0x0010)
//        {
//            I2C1->CR1 |= CR1_PE_Set;
//            SR1Register = 0;
//            SR2Register = 0;

//        }
//    } /* End slave mode */

//#endif

//    /* If SB = 1, I2C1 master sent a START on the bus: EV5) */
//    if ((SR1Register &0x0001) == 0x0001)
//    {

//        /* Send the slave address for transmssion or for reception (according to the configured value
//            in the write master write routine */
//        I2C1->DR = Address;
//        SR1Register = 0;
//        SR2Register = 0;
//    }
//    /* If I2C1 is Master (MSL flag = 1) */

//    if ((SR2Register &0x0001) == 0x0001)
//    {
//        /* If ADDR = 1, EV6 */
//        if ((SR1Register &0x0002) == 0x0002)
//        {
//            /* Write the first data in case the Master is Transmitter */
//            if (I2CDirection == I2C_DIRECTION_TX)
//            {
//                /* Initialize the Transmit counter */
//                Tx_Idx1 = 0;
//                /* Write the first data in the data register */
//                I2C1->DR = Buffer_Tx1[Tx_Idx1++];
//                /* Decrement the number of bytes to be written */
//                NumbOfBytes1--;
//                /* If no further data to be sent, disable the I2C BUF IT
//                in order to not have a TxE  interrupt */
//                if (NumbOfBytes1 == 0)
//                {
//                    I2C1->CR2 &= (uint16_t)~I2C_IT_BUF;
//                }

//            }
//            /* Master Receiver */
//            else

//            {
//                /* Initialize Receive counter */
//                Rx_Idx1 = 0;
//                /* At this stage, ADDR is cleared because both SR1 and SR2 were read.*/
//                /* EV6_1: used for single byte reception. The ACK disable and the STOP
//                Programming should be done just after ADDR is cleared. */
//                if (NumbOfBytes1 == 1)
//                {
//                    /* Clear ACK */
//                    I2C1->CR1 &= CR1_ACK_Reset;
//                    /* Program the STOP */
//                    I2C1->CR1 |= CR1_STOP_Set;
//                }
//            }
//            SR1Register = 0;
//            SR2Register = 0;

//        }
//        /* Master transmits the remaing data: from data2 until the last one.  */
//        /* If TXE is set */
//        if ((SR1Register &0x0084) == 0x0080)
//        {
//            /* If there is still data to write */
//            if (NumbOfBytes1!=0)
//            {
//                /* Write the data in DR register */
//                I2C1->DR = Buffer_Tx1[Tx_Idx1++];
//                /* Decrment the number of data to be written */
//                NumbOfBytes1--;
//                /* If  no data remains to write, disable the BUF IT in order
//                to not have again a TxE interrupt. */
//                if (NumbOfBytes1 == 0)
//                {
//                    /* Disable the BUF IT */
//                    I2C1->CR2 &= (uint16_t)~I2C_IT_BUF;
//                }
//            }
//            SR1Register = 0;
//            SR2Register = 0;
//        }
//        /* If BTF and TXE are set (EV8_2), program the STOP */
//        if ((SR1Register &0x0084) == 0x0084)
//        {

//            /* Program the STOP */
//            I2C1->CR1 |= CR1_STOP_Set;
//            /* Disable EVT IT In order to not have again a BTF IT */
//            I2C1->CR2 &= (uint16_t)~I2C_IT_EVT;
//            SR1Register = 0;
//            SR2Register = 0;
//        }
//        /* If RXNE is set */
//        if ((SR1Register &0x0040) == 0x0040)
//        {
//            /* Read the data register */
//            Buffer_Rx1[Rx_Idx1++] = I2C1->DR;
//            /* Decrement the number of bytes to be read */
//            NumbOfBytes1--;
//            /* If it remains only one byte to read, disable ACK and program the STOP (EV7_1) */
//            if (NumbOfBytes1 == 1)
//            {
//                /* Clear ACK */
//                I2C1->CR1 &= CR1_ACK_Reset;
//                /* Program the STOP */
//                I2C1->CR1 |= CR1_STOP_Set;
//            }
//            SR1Register = 0;
//            SR2Register = 0;
//        }

//    }


//}


//void I2C3_EV_IRQHandler(void)
//{
//
//}
//
//void I2C3_ER_IRQHandler(void)
//{
//
//}