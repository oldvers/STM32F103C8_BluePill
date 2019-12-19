#include <stdio.h>

#include "stm32f1xx.h"
#include "types.h"
#include "gpio.h"
#include "debug.h"
#include "uniquedevid.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "usb_device.h"
#include "vcp.h"

void vLEDTask(void * pvParameters)
{
  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ);
  
  while(TRUE)
  {
    GPIO_Lo(GPIOC, 13);
    vTaskDelay(500);
    GPIO_Hi(GPIOC, 13);
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

void vVCPTask(void * pvParameters)
{
  U8  Rx[64];
  PourReq_p RxPkt;
  PourRsp_t TxPkt;
  U16 RxLen = 0;

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
  LOG("STM32F103C8 Started!\r\n");
  LOG("ID0 = 0x%04X\r\n", UDID_0);
  LOG("ID1 = 0x%04X\r\n", UDID_1);
  LOG("ID2 = 0x%08X\r\n", UDID_2);
  LOG("ID2 = 0x%08X\r\n", UDID_3);
  LOG("Memory Size = %d kB\r\n", FLASH_SIZE);
  LOG("SysClock = %d Hz\r\n", SystemCoreClock);
  
  USBD_Init();
  
//  GPIO_Init(GPIOB, 6, GPIO_TYPE_OUT_PP_2MHZ);
//  GPIO_Lo(GPIOB, 6);
//  GPIO_Init(GPIOB, 8, GPIO_TYPE_OUT_PP_2MHZ);
//  GPIO_Hi(GPIOB, 8);
  
  xTaskCreate(vLEDTask,"LEDTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
  xTaskCreate(vVCPTask,"VCPTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

  vTaskStartScheduler();

  while(TRUE) {};
}

void Fault(U32 stack[])
{
  enum {r0, r1, r2, r3, r12, lr, pc, psr};
  
  LOG("Hard Fault\r\n");
  LOG("  SHCSR    = 0x%08x\r\n", SCB->SHCSR);
  LOG("  CFSR     = 0x%08x\r\n", SCB->CFSR);
  LOG("  HFSR     = 0x%08x\r\n", SCB->HFSR);
  LOG("  MMFAR    = 0x%08x\r\n", SCB->MMFAR);
  LOG("  BFAR     = 0x%08x\r\n", SCB->BFAR);  

  LOG("  R0       = 0x%08x\r\n", stack[r0]);
  LOG("  R1       = 0x%08x\r\n", stack[r1]);
  LOG("  R2       = 0x%08x\r\n", stack[r2]);
  LOG("  R3       = 0x%08x\r\n", stack[r3]);
  LOG("  R12      = 0x%08x\r\n", stack[r12]);
  LOG("  LR [R14] = 0x%08x - Subroutine call return address\r\n", stack[lr]);
  LOG("  PC [R15] = 0x%08x - Program counter\r\n", stack[pc]);
  LOG("  PSR      = 0x%08x\r\n", stack[psr]);

  while(TRUE) {};
}
