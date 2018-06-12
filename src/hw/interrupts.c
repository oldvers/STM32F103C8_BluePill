#include "system.h"
#include "types.h"
#include "interrupts.h"
#include "hardware.h"
#include "usb.h"
#include "nrf24l01p.h"

void NMI_Handler(void)
{
}

//void HardFault_Handler(void)
//{
//  /* Go to infinite loop when Hard Fault exception occurs */
//  while (1)
//  {
//  }
//}

void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  //ShowInfo("-Mem Manage-");
  while (1)
  {
  }
}

void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  //ShowInfo("-Bus Fault-");
  while (1)
  {
  }
}

void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  //ShowInfo("-Usage Fault-");
  while (1)
  {
  }
}

//void SVC_Handler(void)
//{
//}

void DebugMon_Handler(void)
{
}

//void PendSV_Handler(void)
//{
//}

//void SysTick_Handler(void)
//{
//}

void I2C1_EV_IRQHandler(void)
{
  //I2C_EV_IRQHandler(I2C1);
}

void I2C1_ER_IRQHandler(void)
{
  //I2C_ER_IRQHandler(I2C1);
}

void I2C2_EV_IRQHandler(void)
{
  //I2C_EV_IRQHandler(I2C2);
}

void I2C2_ER_IRQHandler(void)
{
  //I2C_ER_IRQHandler(I2C2);
}

void TIM1_CC_IRQHandler(void)
{
  //DHT21_TIM_IRQHandler();
}

/*void PPP_IRQHandler(void)
{
}*/

void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_IRQHandler();
}

void EXTI15_10_IRQHandler(void)
{
  if (0 != SYS_BITBAND_HW(EXTI->PR, NRF24_IRQ_PIN))
  {
    /* Clear pending bit */
    SYS_BITBAND_HW(EXTI->PR, NRF24_IRQ_PIN) = 1;
    /* Call IRQ Handler */
    nRF24L01P_IrqHandler();
  }
}
