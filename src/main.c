#include <stdio.h>

#include "board.h"
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
#include "vcp.h"

void vLEDTask(void * pvParameters)
{
  while (FW_TRUE)
  {
    DBG_SetTextColorGreen();
    //printf("LED On\r\n");
    vTaskDelay(500);
    DBG_SetTextColorRed();
    //printf("LED Off\r\n");
    vTaskDelay(500);
  }
  //vTaskDelete(NULL);
}

void vVCPTask(void * pvParameters)
{
  U8  Rx[130];
  U16 RxLen = 0;
  U32 time;

  /* Init PB2 to OD Hi-Z - Switch-off 1k5 PullUp from USB D+ */
  GPIO_Init(USB_PUP_PORT, USB_PUP_PIN, GPIO_TYPE_OUT_OD_2MHZ, 1);

  /* Delay */
  vTaskDelay(200);

  /* Init USB. Switch-on 1k5 PullUp to USB D+ - connect USB device */
  USBD_Init();
  GPIO_Lo(USB_PUP_PORT, USB_PUP_PIN);

  vTaskDelay(5000);

  if (FW_TRUE == VCP_Open())
  {
    while (FW_TRUE)
    {
      RxLen = VCP_Read(Rx, sizeof(Rx), 5000);
      if (0 < RxLen)
      {
        DBG("VCP Rx: len = %d\r\n", RxLen);
        DBG("VCP Rx: ");
        for (U8 i = 0; i < RxLen; i++) DBG("%02X ", Rx[i]);
        DBG("\r\n");

        time = xTaskGetTickCount();
        VCP_Write(Rx, RxLen, 5000);
        DBG("VCP Tx: time = %d\r\n", xTaskGetTickCount() - time);
      }
      else
      {
        DBG("VCP Rx: Timout\r\n");
      }
    }
  }
  VCP_Close();
  vTaskDelete(NULL);
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

//  USBD_Init();

//  GPIO_Init(GPIOB, 6, GPIO_TYPE_OUT_PP_2MHZ);
//  GPIO_Lo(GPIOB, 6);
//  GPIO_Init(GPIOB, 8, GPIO_TYPE_OUT_PP_2MHZ);
//  GPIO_Hi(GPIOB, 8);

  xTaskCreate
  (
    vLEDTask,
    "LEDTask",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );

  xTaskCreate
  (
    vVCPTask,
    "VCPTask",
    configMINIMAL_STACK_SIZE * 4,
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
