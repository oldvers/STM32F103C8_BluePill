#include <stdio.h>

#include "stm32f1xx.h"
#include "types.h"
#include "board.h"
#include "gpio.h"
#include "debug.h"
#include "uniquedevid.h"
#include "system.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "usb_device.h"
#include "fifo.h"


void vLEDTask(void * pvParameters)
{
  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ, 0);

  while(1)
  {
    GPIO_Lo(GPIOC, 13);
    //DBG_SetTextColorGreen();
    //printf("LED On\r\n");
    vTaskDelay(500);
    GPIO_Hi(GPIOC, 13);
    //DBG_SetTextColorRed();
    //printf("LED Off\r\n");
    vTaskDelay(500);
  }
  //vTaskDelete(NULL);
}

void vDualUartTask(void * pvParameters)
{
  /* Free PB3 from JTAG */
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
  AFIO->MAPR &= ~(7 << 24);
  AFIO->MAPR |= (2 << 24);

  /* Init PB3 */
  GPIO_Init(SWD_SWO_PORT, SWD_SWO_PIN, GPIO_TYPE_OUT_PP_50MHZ, 0);

  /* Enable WiFi Module */
  GPIO_Init(WIFI_EN_PORT, WIFI_EN_PIN, GPIO_TYPE_OUT_PP_2MHZ, 1);

  /* Test Pins */
  GPIO_Init(GPIOA,  5, GPIO_TYPE_OUT_PP_50MHZ, 0);
  GPIO_Init(GPIOA,  6, GPIO_TYPE_OUT_PP_50MHZ, 0);
  GPIO_Init(GPIOA,  7, GPIO_TYPE_OUT_PP_50MHZ, 0);
  GPIO_Init(GPIOB, 11, GPIO_TYPE_OUT_PP_50MHZ, 0);

  /* Init PB2 to OD Hi-Z - Switch-off 1k5 PullUp from USB D+ */
  GPIO_Init(USB_PUP_PORT, USB_PUP_PIN, GPIO_TYPE_OUT_OD_2MHZ, 1);

  /* Delay */
  vTaskDelay(200);

  /* Init USB. Switch-on 1k5 PullUp to USB D+ - connect USB device */
  USBD_Init();
  GPIO_Lo(USB_PUP_PORT, USB_PUP_PIN);

  vTaskDelay(5000);

  while(FW_TRUE)
  {
    vTaskDelay(5000);
  }
  //vTaskDelete(NULL);
}

int main(void)
{
  DBG_Init();
  DBG_ClearScreen();
  DBG_SetDefaultColors();

  DBG("STM32F103C8 Started!\r\n");
  DBG("Double UART Converter Task Started\r\n");
  DBG("ID0         = 0x%04X\r\n", UDID_0);
  DBG("ID1         = 0x%04X\r\n", UDID_1);
  DBG("ID2         = 0x%08X\r\n", UDID_2);
  DBG("ID2         = 0x%08X\r\n", UDID_3);
  DBG("Memory Size = %d kB\r\n", FLASH_SIZE);
  DBG("CPU clock   = %d Hz\r\n", CPUClock);
  DBG("AHB clock   = %d Hz\r\n", AHBClock);
  DBG("APB1 clock  = %d Hz\r\n", APB1Clock);
  DBG("APB2 clock  = %d Hz\r\n", APB2Clock);

  xTaskCreate
  (
    vLEDTask,
    "LED",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );

  xTaskCreate
  (
    vDualUartTask,
    "Dual UART",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );

  vTaskStartScheduler();

  while (FW_TRUE) {};
}

void on_error(void)
{
  while (FW_TRUE) {};
}
