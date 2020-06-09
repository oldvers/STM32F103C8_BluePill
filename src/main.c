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
//#include "vcp.h"

#include "fifo.h"

void vLEDTask(void * pvParameters)
{
    GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ);

    while(FW_TRUE)
    {
        GPIO_Lo(GPIOC, 13);
        vTaskDelay(500);
        GPIO_Hi(GPIOC, 13);
        vTaskDelay(500);
//        LOG("LED\r\n");
    }
    //vTaskDelete(NULL);
}

void vJTAGICEmkIITask(void * pvParameters)
{
//  U8  Rx[130];
//  U16 RxLen = 0;
//  U32 time;

    LOG("JTAG ICE mkII Task Started\r\n");

    /* Init PB2 to OD Hi-Z - Switch-off 1k5 PullUp from USB D+ */
    GPIO_Init(GPIOB, 2, GPIO_TYPE_OUT_OD_2MHZ);
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
  //Debug_Init();

    LOG("STM32F103C8 Started!\r\n");
    LOG("ID0 = 0x%04X\r\n", UDID_0);
    LOG("ID1 = 0x%04X\r\n", UDID_1);
    LOG("ID2 = 0x%08X\r\n", UDID_2);
    LOG("ID2 = 0x%08X\r\n", UDID_3);
    LOG("Memory Size = %d kB\r\n", FLASH_SIZE);
    LOG("SysClock = %d Hz\r\n", SystemCoreClock);

//  FIFO_Init(&Fifo, FifoBuf, sizeof(FifoBuf));
//  LOG("FIFO Capacity = %d\r\n", FIFO_Capacity(&Fifo));
//
//  for (i = 0; i < 10; i++)
//  {
//    data = 0x33;
//    FIFO_Put(&Fifo, &data);
//  }
//  LOG("--- FIFO Put 10 ---\r\n");
//  LOG("FIFO Size = %d\r\n", FIFO_Size(&Fifo));
//  LOG("FIFO Free = %d\r\n", FIFO_Free(&Fifo));
//  LOG("FIFO Sum  = %d\r\n", FIFO_Free(&Fifo) + FIFO_Size(&Fifo));
//
//  for (i = 0; i < 5; i++)
//  {
//    FIFO_Get(&Fifo, &data);
//  }
//  LOG("--- FIFO Get 5 ---\r\n");
//  LOG("FIFO Size = %d\r\n", FIFO_Size(&Fifo));
//  LOG("FIFO Free = %d\r\n", FIFO_Free(&Fifo));
//  LOG("FIFO Sum  = %d\r\n", FIFO_Free(&Fifo) + FIFO_Size(&Fifo));
//
//  for (i = 0; i < 10; i++)
//  {
//    data = 0x44;
//    FIFO_Put(&Fifo, &data);
//  }
//  LOG("--- FIFO Put 10 ---\r\n");
//  LOG("FIFO Size = %d\r\n", FIFO_Size(&Fifo));
//  LOG("FIFO Free = %d\r\n", FIFO_Free(&Fifo));
//  LOG("FIFO Sum  = %d\r\n", FIFO_Free(&Fifo) + FIFO_Size(&Fifo));
//
//  for (i = 0; i < 3; i++)
//  {
//    FIFO_Get(&Fifo, &data);
//  }
//  LOG("--- FIFO Get 3 ---\r\n");
//  LOG("FIFO Size = %d\r\n", FIFO_Size(&Fifo));
//  LOG("FIFO Free = %d\r\n", FIFO_Free(&Fifo));
//  LOG("FIFO Sum  = %d\r\n", FIFO_Free(&Fifo) + FIFO_Size(&Fifo));
//
//  for (i = 0; i < 13; i++)
//  {
//    data = 0x55;
//    FIFO_Put(&Fifo, &data);
//  }
//  LOG("--- FIFO Put 13 ---\r\n");
//  LOG("FIFO Size = %d\r\n", FIFO_Size(&Fifo));
//  LOG("FIFO Free = %d\r\n", FIFO_Free(&Fifo));
//  LOG("FIFO Sum  = %d\r\n", FIFO_Free(&Fifo) + FIFO_Size(&Fifo));
//
//  for (i = 0; i < 20; i++)
//  {
//    FIFO_Get(&Fifo, &data);
//  }
//  LOG("--- FIFO Get 20 ---\r\n");
//  LOG("FIFO Size = %d\r\n", FIFO_Size(&Fifo));
//  LOG("FIFO Free = %d\r\n", FIFO_Free(&Fifo));
//  LOG("FIFO Sum  = %d\r\n", FIFO_Free(&Fifo) + FIFO_Size(&Fifo));
//
//  for (i = 0; i < 13; i++)
//  {
//    data = 0x66;
//    FIFO_Put(&Fifo, &data);
//  }
//  LOG("--- FIFO Put 13 ---\r\n");
//  LOG("FIFO Size = %d\r\n", FIFO_Size(&Fifo));
//  LOG("FIFO Free = %d\r\n", FIFO_Free(&Fifo));
//  LOG("FIFO Sum  = %d\r\n", FIFO_Free(&Fifo) + FIFO_Size(&Fifo));

//  ICEMKII_Init();
//  USBD_Init();

//  FIFO_t fifo;
//  U8 fifobuf[17];
//  U8 i, data;

//  FIFO_Init(&fifo, fifobuf, sizeof(fifobuf));
//
//  LOG("FIFO Size = %d B\r\n", FIFO_Size(&fifo));
//  for (i = 0; i < sizeof(fifobuf); i++)
//  {
//    data = i;
//    FIFO_Put(&fifo, &data);
//  }
//  LOG("FIFO Free = %d B\r\n", FIFO_Free(&fifo));
//
//  for (i = 0; i < 5; i++)
//  {
//    FIFO_Get(&fifo, &data);
//  }
//  LOG("FIFO Free = %d B\r\n", FIFO_Free(&fifo));
//
//  for (i = 0; i < 9; i++)
//  {
//    data = i;
//    FIFO_Put(&fifo, &data);
//  }
//  LOG("FIFO Free = %d B\r\n", FIFO_Free(&fifo));

//  GPIO_Init(GPIOB, 6, GPIO_TYPE_OUT_PP_2MHZ);
//  GPIO_Lo(GPIOB, 6);
//  GPIO_Init(GPIOB, 8, GPIO_TYPE_OUT_PP_2MHZ);
//  GPIO_Hi(GPIOB, 8);

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
        vJTAGICEmkIITask,
        "JTAG ICE mkII",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );

    vTaskStartScheduler();

    while(FW_TRUE) {};
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

    while(FW_TRUE) {};
}
