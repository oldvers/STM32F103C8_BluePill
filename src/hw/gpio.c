#include "gpio.h"

typedef struct
{
  volatile U32 CR[2];
  volatile U32 IDR;
  volatile U32 ODR;
  volatile U32 BSRR;
  volatile U32 BRR;
  volatile U32 LCKR;
} GPIO;

void GPIO_Init(GPIO_TypeDef * pPort, U8 aPin, U8 aMode, U8 aValue)
{
  U32 temp = 0;

  /* Enable GPIO clock */
  temp = (1 << (((U32)pPort >> 10) & 0x0F));
  temp |= (RCC_APB2ENR_AFIOEN * (U8)(aMode > 8));
  RCC->APB2ENR |= temp;

  /* Set default value */
  if (0 == aValue)
  {
    pPort->BSRR = (1 << (aPin + 16));
  }
  else
  {
    pPort->BSRR = (1 << aPin);
  }

  /* Set GPIO operation mode */
  temp = ((GPIO *)pPort)->CR[aPin / 8];
  temp &= ~(GPIO_TYPE_MASK << ((aPin % 8) * 4));
  temp |= (aMode << ((aPin % 8) * 4));
  ((GPIO *)pPort)->CR[aPin / 8] = temp;
}
