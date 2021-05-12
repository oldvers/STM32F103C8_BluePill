#include <stdio.h>

#include "stm32f1xx.h"
#include "types.h"
#include "gpio.h"
#include "debug.h"
#include "uniquedevid.h"
#include "system.h"
#include "debug.h"

#include "nrf24.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "usb_device.h"
#include "vcp.h"
#include "sensors.h"

void vMainTask(void * pvParameters)
{
#ifdef NRF_RX
  U8 Pipe;
#endif
  U8 Addr[5] = { 'n', 'R', 'F', '2', '4' }, Payload[32];
  U8 PayloadLen = sizeof(SENS_DATA);

  GPIO_Init(LEDG_PORT, LEDG_PIN, GPIO_TYPE_OUT_OD_2MHZ);
  GPIO_Hi(LEDG_PORT, LEDG_PIN);

  nRF24_Init();

#ifdef NRF_RX
  /* Wake-up transceiver */
  nRF24_SetPowerMode(nRF24_PWR_UP);
  vTaskDelay(MsToTicks(10));
  /* Disable ShockBurst */
  nRF24_DisableAa(0xFF);
  /* Set RF channel to 2490MHz */
  nRF24_SetRfChannel(90);
  nRF24_SetDataRate(nRF24_DR_2Mbps);
  nRF24_SetCrcScheme(nRF24_CRC_2byte);
  nRF24_SetAddrWidth(5);
  nRF24_SetAddr(nRF24_PIPE1, (U8 *)&Addr);
  nRF24_SetRxPipe(nRF24_PIPE1, nRF24_AA_OFF, PayloadLen);
  nRF24_SetOperationalMode(nRF24_MODE_RX);
  vTaskDelay(MsToTicks(10));
  /* Then pull CE pin to HIGH, and the nRF24 will start a receive... */
#else
  /* Init sensors unit */
  Sensors_Init();

  /* Wake-up transceiver */
  nRF24_SetPowerMode(nRF24_PWR_UP);
  vTaskDelay(MsToTicks(10));
  /* Disable ShockBurst */
  nRF24_DisableAa(0xFF);
  /* Set RF channel to 2490MHz */
  nRF24_SetRfChannel(90);
  nRF24_SetDataRate(nRF24_DR_2Mbps);
  nRF24_SetCrcScheme(nRF24_CRC_2byte);
  nRF24_SetAddrWidth(5);
  nRF24_SetTxPower(nRF24_TXPWR_0dBm);
  nRF24_SetAddr(nRF24_PIPETX, Addr);
  nRF24_SetOperationalMode(nRF24_MODE_TX);
  vTaskDelay(MsToTicks(10));
  /* The nRF24 is ready for transmission, upload a payload,
   * then pull CE pin to HIGH and it will transmit a packet. */
#endif

  nRF24_DumpConfig();
  VCP_Open();

#ifdef NRF_RX
  /* Pull CE pin to HIGH, and the nRF24 will start a receive. */
  nRF24_RxOn();
#endif

  while(TRUE)
  {
#ifdef NRF_RX
    PayloadLen = nRF24_Receive(Payload, &Pipe, MsToTicks(5000));
    if (0 != PayloadLen)
    {
      GPIO_Lo(LEDG_PORT, LEDG_PIN);
      VCP_Write(Payload, PayloadLen, MsToTicks(10));
      //LOG("Received %d bytes on Pipe Number %d\r\n", PayloadLen, Pipe);
      //vTaskDelay(MsToTicks(50));
      GPIO_Hi(LEDG_PORT, LEDG_PIN);
    }
#else
    GPIO_Lo(LEDG_PORT, LEDG_PIN);
    Sensors_Measure((SENS_DATA *)Payload);
    if (0 != nRF24_Transmit(Payload, PayloadLen, MsToTicks(10)))
    {
      //LOG("Transmittion complete\r\n");
      //vTaskDelay(MsToTicks(100));
      GPIO_Lo(LEDG_PORT, LEDG_PIN);
    }
    GPIO_Hi(LEDG_PORT, LEDG_PIN);
    vTaskDelay(MsToTicks(10));
#endif
  }
  //vTaskDelete(NULL);
}

void vVCPTask(void * pvParameters)
{
  U8  Rx[130];
  U16 RxLen = 0;
  U32 time;
  printf("CPU clock   = %d Hz\r\n", CPUClock);
  printf("AHB clock   = %d Hz\r\n", AHBClock);
  printf("APB1 clock  = %d Hz\r\n", APB1Clock);
  printf("APB2 clock  = %d Hz\r\n", APB2Clock);

  if (TRUE == VCP_Open())
  {
    while(TRUE)
    {
      RxLen = VCP_Read(Rx, sizeof(Rx), 5000);
      if (0 < RxLen)
      {
        LOG("VCP Rx: len = %d\r\n", RxLen);
        LOG("VCP Rx: ");
        for (U8 i = 0; i < RxLen; i++) LOG("%02X ", Rx[i]);
        LOG("\r\n");

        time = xTaskGetTickCount();
        VCP_Write(Rx, RxLen, 5000);
        LOG("VCP Tx: time = %d\r\n", xTaskGetTickCount() - time);
      }
      else
      {
        LOG("VCP Rx: Timout\r\n");
      }
    }
  }
  VCP_Close();
  vTaskDelete(NULL);
}

//void vSensTask(void * pvParameters)
//{
////  U8  Rx[130];
////  U16 RxLen = 0;
////  U32 time;

//  Sensors_Init();

//  vTaskDelay(MsToTicks(10));

//  while(TRUE)
//  {
//    Sensors_Measure();
//    vTaskDelay(MsToTicks(1000));

////    RxLen = VCP_Read(Rx, sizeof(Rx), 5000);
////    if (0 < RxLen)
////    {
////      LOG("VCP Rx: len = %d\r\n", RxLen);
////      LOG("VCP Rx: ");
////      for (U8 i = 0; i < RxLen; i++) LOG("%02X ", Rx[i]);
////      LOG("\r\n");

////      time = xTaskGetTickCount();
////      VCP_Write(Rx, RxLen, 5000);
////      LOG("VCP Tx: time = %d\r\n", xTaskGetTickCount() - time);
////    }
////    else
////    {
////      LOG("VCP Rx: Timout\r\n");
////    }
//  }

//}

int main(void)
{
  LOG("STM32F103C8 Started!\r\n");
  LOG("ID0 = 0x%04X\r\n", UDID_0);
  LOG("ID1 = 0x%04X\r\n", UDID_1);
  LOG("ID2 = 0x%08X\r\n", UDID_2);
  LOG("ID2 = 0x%08X\r\n", UDID_3);
  LOG("Memory Size = %d kB\r\n", FLASH_SIZE);
  LOG("SysClock = %d Hz\r\n", SystemCoreClock);

#ifdef NRF_RX
  USBD_Init();
//  xTaskCreate
//  (
//    vVCPTask,
//    "VCPTask",
//    configMINIMAL_STACK_SIZE,
//    NULL,
//    tskIDLE_PRIORITY + 1,
//    NULL
//  );
#else
//  xTaskCreate
//  (
//    vSensTask,
//    "SensTask",
//    configMINIMAL_STACK_SIZE,
//    NULL,
//    tskIDLE_PRIORITY + 1,
//    NULL
//  );
#endif
  xTaskCreate
  (
    vMainTask,
    "MainTask",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );

  vTaskStartScheduler();

  while(TRUE) {};
}
