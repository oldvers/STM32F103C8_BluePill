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

#include "board.h"
#include "gpio.h"

#include "fifo.h"

void vLEDTask(void * pvParameters)
{
  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ, 0);

//  GPIO_Init(UART2_RTX_PORT, UART2_RTX_PIN, GPIO_TYPE_OUT_OD_2MHZ, 1);

//  GPIO_Lo(UART2_RTX_PORT, UART2_RTX_PIN);
//  vTaskDelay(50);
//  GPIO_Hi(UART2_RTX_PORT, UART2_RTX_PIN);
//  vTaskDelay(2);

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

void vJTAGICEmkIITask(void * pvParameters)
{
//  U8  Rx[130];
//  U16 RxLen = 0;
//  U32 time;

    DBG("JTAG ICE mkII Task Started\r\n");

    /* Init PB2 to OD Hi-Z - Switch-off 1k5 PullUp from USB D+ */
    GPIO_Init(GPIOB, 2, GPIO_TYPE_OUT_OD_2MHZ, 0);
    GPIO_Hi(GPIOB, 2);

    /* Delay */
    vTaskDelay(200);

    /* Init USB. Switch-on 1k5 PullUp to USB D+ - connect USB device */
    USBD_Init();
    GPIO_Lo(GPIOB, 2);

//  if (TRUE == VCP_Open())
//  {
    while(FW_TRUE)
    {
//      RxLen = VCP_Read(Rx, sizeof(Rx), 5000);
//      if (0 < RxLen)
//      {
//        LOG("VCP Rx: len = %d\r\n", RxLen);
//        LOG("VCP Rx: ");
//        for (U8 i = 0; i < RxLen; i++) LOG("%02X ", Rx[i]);
//        LOG("\r\n");

//        time = xTaskGetTickCount();
//        VCP_Write(Rx, RxLen, 5000);
//        LOG("VCP Tx: time = %d\r\n", xTaskGetTickCount() - time);
//      }
//      else
//      {
//        LOG("VCP Rx: Timout\r\n");
//      }
        vTaskDelay(5000);
    }
//  }
//  VCP_Close();
//  vTaskDelete(NULL);
}



//U8     FifoBuf[17];
//FIFO_t Fifo;
//U8     i, data;
//extern void ICEMKII_Init(void);

int main(void)
{
  DBG_Init();
  DBG_ClearScreen();
  DBG_SetDefaultColors();

  DBG("STM32F103C8 Started!\r\n");
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
    "LEDTask",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );

  xTaskCreate
  (
    vJTAGICEmkIITask,
    "ICEMKII",
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
