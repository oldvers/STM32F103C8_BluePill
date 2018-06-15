#include <stdio.h>

#include "stm32f1xx.h"
#include "types.h"
#include "gpio.h"
#include "debug.h"
#include "uniquedevid.h"

#include "nrf24.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "usb_device.h"
#include "vcp.h"

void vLEDTask(void * pvParameters)
{
#ifdef NRF_RX
  U8 Pipe;
#endif
  U8 Addr[5] = { 'n', 'R', 'F', '2', '4' }, Payload[32], PayloadLen;

  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ);
  GPIO_Hi(GPIOC, 13);

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
  nRF24_SetRxPipe(nRF24_PIPE1, nRF24_AA_OFF, 10);
  nRF24_SetOperationalMode(nRF24_MODE_RX);
  vTaskDelay(MsToTicks(10));
  /* Then pull CE pin to HIGH, and the nRF24 will start a receive... */
#else
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
      GPIO_Lo(GPIOC, 13);
      VCP_Write(Payload, PayloadLen, 5);
      LOG("Received %d bytes on Pipe Number %d\r\n", PayloadLen, Pipe);
      vTaskDelay(MsToTicks(50));
      GPIO_Hi(GPIOC, 13);
    }
#else
    PayloadLen = 10;
    Payload[0] = 'H';
    Payload[1] = 'e';
    Payload[2] = 'l';
    Payload[3] = 'l';
    Payload[4] = 'o';
    Payload[5] = ',';
    Payload[6] = ' ';
    Payload[7] = 'C';
    Payload[8] = 'h';
    Payload[9] = '!';
    if (0 != nRF24_Transmit(Payload, PayloadLen, MsToTicks(30)))
    {
      GPIO_Lo(GPIOC, 13);
      LOG("Transmittion complete\r\n");
      vTaskDelay(MsToTicks(100));
      GPIO_Hi(GPIOC, 13);
    }
    vTaskDelay(MsToTicks(500));
#endif
  }
  //vTaskDelete(NULL);
}

void vVCPTask(void * pvParameters)
{
  U8  Rx[130];
  U16 RxLen = 0;
  U32 time;
  
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
  //xTaskCreate(vVCPTask,"VCPTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
#endif
  xTaskCreate(vLEDTask,"LEDTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

  vTaskStartScheduler();

  while(TRUE) {};
}
