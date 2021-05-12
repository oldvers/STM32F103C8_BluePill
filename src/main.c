#include <stdio.h>

#include "stm32f1xx.h"
#include "types.h"
#include "gpio.h"
#include "debug.h"
#include "uniquedevid.h"
#include "system.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "usb_device.h"

void vLEDTask(void * pvParameters)
{
  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ, 0);

  while(1)
  {
    GPIO_Lo(GPIOC, 13);
    DBG_SetTextColorGreen();
    printf("LED On\r\n");
    vTaskDelay(500);
    GPIO_Hi(GPIOC, 13);
    DBG_SetTextColorRed();
    printf("LED Off\r\n");
    vTaskDelay(500);
  }
  //vTaskDelete(NULL);
}

int main(void)
{
  DBG_Init();
  DBG_ClearScreen();
  DBG_SetDefaultColors();

  printf("STM32F103C8 Started!\r\n");
  printf("ID0         = 0x%04X\r\n", UDID_0);
  printf("ID1         = 0x%04X\r\n", UDID_1);
  printf("ID2         = 0x%08X\r\n", UDID_2);
  printf("ID2         = 0x%08X\r\n", UDID_3);
  printf("Memory Size = %d kB\r\n", FLASH_SIZE);
  printf("CPU clock   = %d Hz\r\n", CPUClock);
  printf("AHB clock   = %d Hz\r\n", AHBClock);
  printf("APB1 clock  = %d Hz\r\n", APB1Clock);
  printf("APB2 clock  = %d Hz\r\n", APB2Clock);

  USBD_Init();
  
//  GPIO_Init(GPIOB, 6, GPIO_TYPE_OUT_PP_2MHZ);
//  GPIO_Lo(GPIOB, 6);
//  GPIO_Init(GPIOB, 8, GPIO_TYPE_OUT_PP_2MHZ);
//  GPIO_Hi(GPIOB, 8);
  
  xTaskCreate(vLEDTask,"LEDTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

  vTaskStartScheduler();

  while (FW_TRUE) {};
}

void on_error(void)
{
  while (FW_TRUE) {};
}
