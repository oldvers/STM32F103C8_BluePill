#include <stdio.h>

#include "stm32f1xx.h"
#include "types.h"
#include "gpio.h"
#include "uniquedevid.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void vLEDTask(void * pvParameters)
{
  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ);
  
  while(1)
  {
    GPIO_Lo(GPIOC, 13);
    vTaskDelay(500);
    GPIO_Hi(GPIOC, 13);
    vTaskDelay(500);
  }
  //vTaskDelete(NULL);
}

int main(void)
{
  printf("STM32F103C8 Started!\r\n");
  printf("ID0 = 0x%04X\r\n", UDID_0);
  printf("ID1 = 0x%04X\r\n", UDID_1);
  printf("ID2 = 0x%08X\r\n", UDID_2);
  printf("ID2 = 0x%08X\r\n", UDID_3);
  printf("Memory Size = %d kB\r\n", FLASH_SIZE);
  
  xTaskCreate(vLEDTask,"LEDTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

  vTaskStartScheduler();

  while(TRUE) {};
}

void Fault(U32 stack[])
{
  enum {r0, r1, r2, r3, r12, lr, pc, psr};
  
  printf("Hard Fault\r\n");
  printf("  SHCSR    = 0x%08x\r\n", SCB->SHCSR);
  printf("  CFSR     = 0x%08x\r\n", SCB->CFSR);
  printf("  HFSR     = 0x%08x\r\n", SCB->HFSR);
  printf("  MMFAR    = 0x%08x\r\n", SCB->MMFAR);
  printf("  BFAR     = 0x%08x\r\n", SCB->BFAR);  

  printf("  R0       = 0x%08x\r\n", stack[r0]);
  printf("  R1       = 0x%08x\r\n", stack[r1]);
  printf("  R2       = 0x%08x\r\n", stack[r2]);
  printf("  R3       = 0x%08x\r\n", stack[r3]);
  printf("  R12      = 0x%08x\r\n", stack[r12]);
  printf("  LR [R14] = 0x%08x - Subroutine call return address\r\n", stack[lr]);
  printf("  PC [R15] = 0x%08x - Program counter\r\n", stack[pc]);
  printf("  PSR      = 0x%08x\r\n", stack[psr]);

  while(TRUE) {};
}
