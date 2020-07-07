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
#include "uart.h"

//extern void CDC_Enable_SOF(void);
//extern void CDC_Disable_SOF(void);

//static U8 rxbuf[35], txbuf[35];
//static FIFO_t tx, rx;

//static FW_BOOLEAN rxPut(U8 * data)
//{
//  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Put(&rx, data));
//}

//static FW_BOOLEAN txGet(U8 * data)
//{
//  return (FW_BOOLEAN)(FW_SUCCESS == FIFO_Get(&tx, data));
//}

void vLEDTask(void * pvParameters)
{
//    /* UART1: PA9 - Tx, PA10 - Rx*/
//    FIFO_Init(&rx, rxbuf, sizeof(rxbuf));
//    FIFO_Init(&tx, txbuf, sizeof(txbuf));
//    GPIO_Init(GPIOA,  9, GPIO_TYPE_ALT_PP_10MHZ);
//    GPIO_Init(GPIOA, 10, GPIO_TYPE_ALT_PP_10MHZ);
//    UART_Init(UART1, 115200, rxPut, txGet);
    
    /* LED */
    GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ);

//    vTaskDelay(1000);
//    CDC_Enable_SOF();
//    vTaskDelay(45);
//    CDC_Disable_SOF();
    
    while(FW_TRUE)
    {
//        /* UART1 */
//        for (U32 i = 0; i < FIFO_Size(&tx); i++)
//        {
//          FIFO_Put(&tx, (U8 *)&i);
//        }
//        UART_TxStart(UART1);

        /* LED */
        GPIO_Lo(GPIOC, 13);
        vTaskDelay(500);
        GPIO_Hi(GPIOC, 13);
        vTaskDelay(500);
//      LOG("LED\r\n");
    }
    //vTaskDelete(NULL);
}

//extern void cdc_BulkAIn(U32 aEvent);

void vJTAGICEmkIITask(void * pvParameters)
{
//  U8  Rx[130];
//  U16 RxLen = 0;
//  U32 time;

    LOG("JTAG ICE mkII Task Started\r\n");

    /* Free PB3 from JTAG */
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR &= ~(7 << 24);
    AFIO->MAPR |= (2 << 24);    
    /* Init PB3 */
    GPIO_Init(GPIOB,  3, GPIO_TYPE_OUT_PP_50MHZ);
    GPIO_Init(GPIOA,  7, GPIO_TYPE_OUT_PP_50MHZ);
    GPIO_Init(GPIOB, 11, GPIO_TYPE_OUT_PP_50MHZ);
    GPIO_Hi(GPIOB, 3);
    GPIO_Hi(GPIOB, 3);
    GPIO_Hi(GPIOB, 3);
    GPIO_Hi(GPIOB, 3);
    GPIO_Hi(GPIOB, 3);
    GPIO_Hi(GPIOB, 3);
    GPIO_Lo(GPIOB, 3);
    

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
    vTaskDelay(5000);
      
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
