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
//#define GPIO_Init(port,pin,mode)
  
  /* Enable GPIO clock */
  temp = (1 << (((U32)pPort >> 10) & 0x0F));
  temp |= (RCC_APB2ENR_AFIOEN * (U8)(aMode > 8));
  RCC->APB2ENR |= temp;
//  (U32)(  | );
  
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


  //((GPIO *)port)->CR[pin / 8] &= ~(GPIO_TYPE_MASK << ((pin % 8) * 4)); \
	//((GPIO *)port)->CR[pin / 8] |= (mode << ((pin % 8) * 4));
  
//#define GPIO_Hi(port,pin) \
//  (port->BSRR = (1 << pin))

//#define GPIO_Lo(port,pin) \
//  (port->BSRR = (1 << (pin + 16)))
}