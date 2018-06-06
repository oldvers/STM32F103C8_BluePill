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
  U8 i = 0, Pipe;
#else
  U32 timeout;
#endif
  U8 Addr[5] = { 'n', 'R', 'F', '2', '4' }, Payload[32], PayloadLen, Status;

  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ);
  GPIO_Hi(GPIOC, 13);

  nRF24_Init();

#ifdef NRF_RX
  nRF24_SetPowerMode(nRF24_PWR_UP); // wake-up transceiver (in case if it sleeping)
  vTaskDelay(5);
  nRF24_DisableAa(0xFF); // disable ShockBurst
  nRF24_SetRfChannel(90); // set RF channel to 2490MHz
  nRF24_SetDataRate(nRF24_DR_2Mbps); // 2Mbit/s data rate
  nRF24_SetCrcScheme(nRF24_CRC_2byte); // 1-byte CRC scheme
  nRF24_SetAddrWidth(5); // address width is 5 bytes
  nRF24_SetAddr(nRF24_PIPE1, (U8 *)&Addr); // program pipe address
  nRF24_SetRxPipe(nRF24_PIPE1, nRF24_AA_OFF, 10); // enable RX pipe#1 with Auto-ACK: disabled, payload length: 10 bytes
  nRF24_SetOperationalMode(nRF24_MODE_RX); // switch transceiver to the RX mode
  vTaskDelay(5);
  // then pull CE pin to HIGH, and the nRF24 will start a receive...
  nRF24_RxOn();
#else
  nRF24_SetPowerMode(nRF24_PWR_UP); // wake-up transceiver (in case if it sleeping)
  vTaskDelay(5);
  nRF24_DisableAa(0xFF); // disable ShockBurst
  nRF24_SetRfChannel(90); // set RF channel to 2490MHz
  nRF24_SetDataRate(nRF24_DR_2Mbps); // 2Mbit/s data rate
  nRF24_SetCrcScheme(nRF24_CRC_2byte); // 1-byte CRC scheme
  nRF24_SetAddrWidth(5); // address width is 5 bytes
  nRF24_SetTxPower(nRF24_TXPWR_0dBm); // configure TX power
  nRF24_SetAddr(nRF24_PIPETX, Addr); // program TX address
  nRF24_SetOperationalMode(nRF24_MODE_TX); // switch transceiver to the TX mode
  vTaskDelay(5);
  // the nRF24 is ready for transmission, upload a payload,
  //  then pull CE pin to HIGH and it will transmit a packet...
#endif

  nRF24_DumpConfig();

  while(TRUE)
  {
#ifdef NRF_RX
    if (0 == GPIO_In(NRF24_IRQ_PORT, NRF24_IRQ_PIN))
    {
      Status = nRF24_GetStatus_RxFifo();
      /* Constantly poll the status of RX FIFO... */
      if (Status != nRF24_STATUS_RXFIFO_EMPTY)
      {
        /* The RX FIFO have some data, take a note what nRF24 can hold
         * up to three payloads of 32 bytes... */
        Pipe = nRF24_RdPayload(Payload, &PayloadLen); // read a payload to buffer
        nRF24_ClearIrqFlags(); // clear any pending IRQ bits
        /* Now the nRF24_payload buffer holds received data
         * PayloadLen variable holds a length of received data
         * Pipe variable holds a number of the pipe which has received the data
         * ... do something with received data ... */
        GPIO_Lo(GPIOC, 13);
        i = 0;
        LOG("Received %d bytes on Pipe Number %d\r\n", PayloadLen, Pipe);
      }
    }
    vTaskDelay(5);
    if (22 > i)
    {
      i++;
    }
    else
    {
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
    nRF24_WrPayload(Payload, PayloadLen); // transfer payload data to transceiver

    nRF24_TxOn(); // assert CE pin (transmission starts)
    timeout = 10000;
    while (timeout--)
    {
      if (0 == GPIO_In(NRF24_IRQ_PORT, NRF24_IRQ_PIN)) break;
    }
    nRF24_TxOff(); // de-assert CE pin (nRF24 goes to StandBy-I mode)

    Status = nRF24_GetStatus();
    nRF24_ClearIrqFlags(); // clear any pending IRQ flags

    if ((0 != timeout) && (Status & nRF24_FLAG_TX_DS))
    {
      //if (Status & (nRF24_FLAG_TX_DS | nRF24_FLAG_MAX_RT))
      //{
      //  // transmission ended, exit loop
      //  break;
      //}

      // Successful transmission
      GPIO_Lo(GPIOC, 13);
      vTaskDelay(100);
      GPIO_Hi(GPIOC, 13);
      LOG("Transmittion complete\r\n");
    }

    
    //if (Status & nRF24_FLAG_MAX_RT)
    //{
    //  // Auto retransmit counter exceeds the programmed maximum limit (payload in FIFO is not removed)
    //  // Also the software can flush the TX FIFO here...
    //  //return TX_MAXRT_ERROR;
    //}

    // In fact that should not happen
    //return TX_UNKNOWN_ERROR;
    vTaskDelay(500);
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
  xTaskCreate(vVCPTask,"VCPTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
#endif
  xTaskCreate(vLEDTask,"LEDTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

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
