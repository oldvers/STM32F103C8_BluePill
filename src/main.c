#include <stdio.h>

#include "stm32f1xx.h"
#include "types.h"
#include "board.h"
#include "gpio.h"
#include "debug.h"
#include "uniquedevid.h"
#include "system.h"
#include "debug.h"
#include "fifo.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "usb_device.h"
#include "usb_definitions.h"
#include "usb_control.h"

extern const U8 USB_ConfigDescriptor[];

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
#if 0 //#ifdef DBG_ENABLE
  USB_CONFIGURATION_DESCRIPTOR * cd;
  cd = (USB_CONFIGURATION_DESCRIPTOR *)USB_ConfigDescriptor;
#endif

  DBG("Double UART Converter Task Started\r\n");

#if 0 //#ifdef DBG_ENABLE
  /* --------- */
  DBG("Config Desc Total Length = 0x%04X = %d\r\n",
      cd->wTotalLength, cd->wTotalLength);

  DBG("--- CDC 1 ------------------------------------\r\n");
  DBG("Interface Count = %d\r\n",     USB_CDC_IF_CNT);
  DBG("    IF Num 0    = %d\r\n",     USB_CDC_IF_NUM);
  DBG("Endpoint Count  = %d\r\n",     USB_CDC_EP_CNT);
  DBG("    EP 0 Blk O  = 0x%02X\r\n", USB_CDC_EP_BLK_O);
  DBG("    EP 1 Blk I  = 0x%02X\r\n", USB_CDC_EP_BLK_I);

  DBG("--- CDC 2 ------------------------------------\r\n");
  DBG("Interface Count = %d\r\n",     USB_CDD_IF_CNT);
  DBG("    IF Num 0    = %d\r\n",     USB_CDD_IF_NUM);
  DBG("Endpoint Count  = %d\r\n",     USB_CDD_EP_CNT);
  DBG("    EP 0 Blk O  = 0x%02X\r\n", USB_CDD_EP_BLK_O);
  DBG("    EP 1 Blk I  = 0x%02X\r\n", USB_CDD_EP_BLK_I);

  DBG("--- Total ------------------------------------\r\n");
  DBG("Interface Count = %d\r\n",     USB_IF_CNT);
  DBG("Endpoint Count  = %d\r\n",     USB_EP_CNT);
  DBG("    EP 0 Ctl I  = 0x%02X\r\n", EP0_I);
  DBG("    EP 1 Ctl O  = 0x%02X\r\n", EP0_O);
#endif

  /* --------- */
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

  /* --------- */
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
