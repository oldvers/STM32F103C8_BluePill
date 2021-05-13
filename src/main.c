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
#include "vcp.h"

/*----------------------------------------------------------------------------*/

void PWM_On(U16 aPrescaler, U16 aReload, U16 aDuty)
{
  GPIO_Init(GPIOA, 6, GPIO_TYPE_ALT_PP_2MHZ);

  RCC->APB1ENR  |= RCC_APB1ENR_TIM3EN;
  RCC->APB1RSTR |= RCC_APB1RSTR_TIM3RST;
  RCC->APB1RSTR &= (~RCC_APB1RSTR_TIM3RST);

  /* Channel 1 - PWM 1 Mode */
  TIM3->CCMR1 = (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1);
  TIM3->CCER  = (TIM_CCER_CC1E);
  TIM3->PSC   = aPrescaler;
  TIM3->ARR   = aReload;
  TIM3->CCR1  = aDuty;
  /* Enable Timer */
  TIM3->CR1   = 1;
}

void PWM_Off(void)
{
  RCC->APB1RSTR |= RCC_APB1RSTR_TIM3RST;
  RCC->APB1RSTR &= (~RCC_APB1RSTR_TIM3RST);

  RCC->APB1ENR  &= (~RCC_APB1ENR_TIM3EN);

  GPIO_Init(GPIOA, 6, GPIO_TYPE_IN_FLOATING);
}

/*----------------------------------------------------------------------------*/

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

typedef __packed struct PourReq_s
{
  U8  Req;
  U16 Param1;
  U16 Param2;
  U16 Param3;
  U16 Param4;
}
PourReq_t, * PourReq_p;

typedef __packed struct PourRsp_s
{
  U8  Rsp;
  U8  Channel;
  U16 Param;
}
PourRsp_t, * PourRsp_p;

void Pour(U8 Channel, U16 Duration, PourRsp_p TxPkt)
{
  U32 time2, time1;

  if (0 == Duration) return;

  time1 = xTaskGetTickCount();
  time2 = xTaskGetTickCount() + Duration;

  TxPkt->Rsp = 1;
  TxPkt->Channel = Channel;

  switch (Channel)
  {
    case 0:
      GPIO_Hi(GPIOA, 0);
      break;
    case 1:
      GPIO_Hi(GPIOA, 1);
      break;
    case 2:
      GPIO_Hi(GPIOA, 2);
      break;
    case 3:
      GPIO_Hi(GPIOA, 3);
      break;
    default:
      return;
  }

  while ( time1 < time2 )
  {
    vTaskDelay(10);
    time1 = xTaskGetTickCount();
    TxPkt->Param = (Duration - (time2 - time1)) * 100 / Duration;
    VCP_Write((U8 *)TxPkt, sizeof(*TxPkt), 50);
  }

  switch (Channel)
  {
    case 0:
      GPIO_Lo(GPIOA, 0);
      break;
    case 1:
      GPIO_Lo(GPIOA, 1);
      break;
    case 2:
      GPIO_Lo(GPIOA, 2);
      break;
    case 3:
      GPIO_Lo(GPIOA, 3);
      break;
    default:
      return;
  }

  TxPkt->Param = 100;
  VCP_Write((U8 *)TxPkt, sizeof(*TxPkt), 50);
}

void PourEx(U16 aPresc, U16 aReload, U16 aDuty, U16 Duration, PourRsp_p TxPkt)
{
  U32 time2, time1;

  if (((0 == aReload) && (0 == aDuty)) || (aDuty > aReload)) return;
  if (0 == Duration) return;

  time1 = xTaskGetTickCount();
  time2 = xTaskGetTickCount() + Duration;

  TxPkt->Rsp = 2;
  TxPkt->Channel = 0;

  PWM_On(aPresc, aReload, aDuty);

  while ( time1 < time2 )
  {
    vTaskDelay(10);
    time1 = xTaskGetTickCount();
    TxPkt->Param = (Duration - (time2 - time1)) * 100 / Duration;
    VCP_Write((U8 *)TxPkt, sizeof(*TxPkt), 50);
  }

  PWM_Off();

  TxPkt->Param = 100;
  VCP_Write((U8 *)TxPkt, sizeof(*TxPkt), 50);
}

void vVCPTask(void * pvParameters)
{
  U8  Rx[64];
  PourReq_p RxPkt;
  PourRsp_t TxPkt;
  U16 RxLen = 0;

  /* Init PB2 to OD Hi-Z - Switch-off 1k5 PullUp from USB D+ */
  GPIO_Init(GPIOB, 2, GPIO_TYPE_OUT_OD_2MHZ);
  GPIO_Hi(GPIOB, 2);

  /* Delay */
  vTaskDelay(200);

  /* Init USB. Switch-on 1k5 PullUp to USB D+ - connect USB device */
  USBD_Init();
  GPIO_Lo(GPIOB, 2);

  GPIO_Init(GPIOA, 0, GPIO_TYPE_OUT_PP_2MHZ);
  GPIO_Init(GPIOA, 1, GPIO_TYPE_OUT_PP_2MHZ);
  GPIO_Init(GPIOA, 2, GPIO_TYPE_OUT_PP_2MHZ);
  GPIO_Init(GPIOA, 3, GPIO_TYPE_OUT_PP_2MHZ);

  if (TRUE == VCP_Open())
  {
    while(TRUE)
    {
      RxLen = VCP_Read(Rx, sizeof(Rx), 5000);

      if (0 == RxLen)
      {
        LOG("VCP Rx: Timout\r\n");
        continue;
      }

      LOG("VCP Rx: len = %d\r\n", RxLen);
      LOG("VCP Rx: ");
      for (U8 i = 0; i < RxLen; i++) LOG("%02X ", Rx[i]);
      LOG("\r\n");

      RxPkt = (PourReq_p)Rx;

      if ( (sizeof(*RxPkt) == RxLen) && (0x01 == RxPkt->Req))
      {
        Pour(0, RxPkt->Param1, &TxPkt);
        Pour(1, RxPkt->Param2, &TxPkt);
        Pour(2, RxPkt->Param3, &TxPkt);
        Pour(3, RxPkt->Param4, &TxPkt);
      }
      else if ( (sizeof(*RxPkt) == RxLen) && (0x02 == RxPkt->Req))
      {
        PourEx
        (
          RxPkt->Param1,
          RxPkt->Param2,
          RxPkt->Param3,
          RxPkt->Param4,
          &TxPkt
        );
      }
      else
      {
        TxPkt.Rsp = 0xFF;
        TxPkt.Channel = 0xFF;
        TxPkt.Param = 0;
        VCP_Write((U8 *)&TxPkt, sizeof(TxPkt), 5000);
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

  xTaskCreate(vLEDTask,"LEDTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
  xTaskCreate(vVCPTask,"VCPTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

  vTaskStartScheduler();

  while (FW_TRUE) {};
}

void on_error(void)
{
  while (FW_TRUE) {};
}
