#include <string.h>

#include "types.h"
//#include "interrupts.h"
//#include "usb.h"
//#include "usb_definitions.h"
//#include "usb_control.h"
//#include "usb_device.h"
//#include "usb_descriptor.h"
//#include "usb_icemkii_definitions.h"
#include "board.h"
#include "gpio.h"
#include "uart.h"
#include "debug.h"
#include "tim.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#define DWIRE_DEBUG

#ifdef DWIRE_DEBUG
#  define DWIRE_LOG      DBG
#else
#  define DWIRE_LOG(...)
#endif

#define DWIRE_TX_COMPLETE              (1 << 0)
#define DWIRE_RX_COMPLETE              (1 << 1)
#define DWIRE_BAUD_COMPLETE            (1 << 2)
#define DWIRE_TIMEOUT                  (100)
#define DWIRE_BAUD_MEAS_COUNT          (6)
#define DWIRE_BAUD_MEAS_START_IDX      (2)

#define DWIRE_DISABLE                  (0x06)
#define DWIRE_RESET                    (0x07)
#define DWIRE_GO                       (0x20)
#define DWIRE_SS                       (0x21)
#define DWIRE_SS_INST                  (0x23)
#define DWIRE_RESUME                   (0x30)
#define DWIRE_RESUME_SS                (0x31)
#define DWIRE_FLAG_RUN                 (0x60)
#define DWIRE_FLAG_RUN_TO_CURSOR       (0x61)
#define DWIRE_FLAG_STEP_OUT            (0x63)
#define DWIRE_FLAG_STEP_IN             (0x79)
#define DWIRE_FLAG_SINGLE_STEP         (0x7A)
#define DWIRE_FLAG_MEMORY              (0x66)
#define DWIRE_FLAG_INST                (0x64)
#define DWIRE_FLAG_FLASH_INST          (0x44)
#define DWIRE_BAUD_128                 (0x83)
#define DWIRE_BAUD_64                  (0x82)
#define DWIRE_BAUD_32                  (0x81)
#define DWIRE_BAUD_16                  (0x80)
#define DWIRE_SET_PC_LOW               (0xC0)
#define DWIRE_SET_PC                   (0xD0)
#define DWIRE_SET_BP                   (0xD1)
#define DWIRE_SET_BP_LOW               (0xC1)
#define DWIRE_SET_IR                   (0xD2)
#define DWIRE_GET_PC                   (0xF0)
#define DWIRE_GET_BP                   (0xF1)
#define DWIRE_GET_IR                   (0xF2)
#define DWIRE_GET_SIG                  (0xF3)
#define DWIRE_RW_MODE                  (0xC2)
#define DWIRE_MODE_READ_SRAM           (0x00)
#define DWIRE_MODE_READ_REGS           (0x01)
#define DWIRE_MODE_READ_FLASH          (0x02)
#define DWIRE_MODE_WRITE_SRAM          (0x04)
#define DWIRE_MODE_WRITE_REGS          (0x05)

#define DWIRE_BUFFER_SIZE              (512 + 32)

typedef struct
{
  /* Sync parameters */
  U16                baudRateValue[DWIRE_BAUD_MEAS_COUNT];
  U8                 baudRateCnt;
  /* Rx/Tx parameters */
  struct
  {
    FW_BOOLEAN       txInProgress : 1;
    FW_BOOLEAN       txOk : 1;
  };
  U8                 txBuffer[DWIRE_BUFFER_SIZE];
  U32                txLen;
  U8                 rxBuffer[DWIRE_BUFFER_SIZE];
  U32                rxLen;
  /* RTOS objects */
  EventGroupHandle_t events;
} DWire_t, * DWire_p;

//static U8                 gTxBuffer[16]                          = {0};
//static U32                gTxLen                                 = 0;
//static U8                 gRxBuffer[16]                          = {0};
//static U32                gRxLen                                 = 0;
//static EventGroupHandle_t gEvents                                = NULL;
//static U16                gBaudRateValue[DWIRE_BAUD_MEAS_COUNT]  = {0};
//static U8                 gBaudRateCnt                           = 0;

static DWire_t gDWire = {0};

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from UART to the Tx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static FW_BOOLEAN uart_RxByte(U8 * pByte)
{
  if (FW_TRUE == gDWire.txInProgress)
  {
    gDWire.txOk &= (FW_BOOLEAN)(gDWire.txBuffer[gDWire.rxLen] == *pByte);
    gDWire.rxLen++;
  }
  else if (sizeof(gDWire.rxBuffer) > gDWire.rxLen)
  {
    gDWire.rxBuffer[gDWire.rxLen] = *pByte;
    gDWire.rxLen++;
  }
  return FW_TRUE;
//  else
//  {
//    return FW_FALSE;
//  }
}

//-----------------------------------------------------------------------------
/** @brief Receive complete callback
 *  @param pByte - Optional pointer to the latest received byte (def. NULL)
 *  @return TRUE, that means UART line idle is received
 */

static FW_BOOLEAN uart_RxComplete(U8 * pByte)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  (void)xEventGroupSetBitsFromISR
        (
          gDWire.events,
          DWIRE_RX_COMPLETE,
          &xHigherPriorityTaskWoken
        );

  if (xHigherPriorityTaskWoken)
  {
    taskYIELD();
  }

  return FW_TRUE;
}

//-----------------------------------------------------------------------------
/** @brief Gets Byte that need to be transmitted from the Rx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return TRUE if byte has been gotten successfully
 */

static FW_BOOLEAN uart_TxByte(U8 * pByte)
{
  if (0 < gDWire.txLen)
  {
    gDWire.txLen--;
    *pByte = gDWire.txBuffer[gDWire.txLen];
    return FW_TRUE;
  }
  else
  {
    return FW_FALSE;
  }
}

//-----------------------------------------------------------------------------
/** @brief Transmit complete callback
 *  @param pByte - Optional pointer to the latest received byte (def. NULL)
 *  @return TRUE, that means UART transmission is complete
 */

static FW_BOOLEAN uart_TxComplete(U8 * pByte)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (FW_TRUE == gDWire.txOk)
  {
    (void)xEventGroupSetBitsFromISR
          (
            gDWire.events,
            DWIRE_TX_COMPLETE,
            &xHigherPriorityTaskWoken
          );

    if (xHigherPriorityTaskWoken)
    {
      taskYIELD();
    }
  }
  gDWire.txInProgress = FW_FALSE;
  gDWire.rxLen = 0;

  return FW_TRUE;
}

//-----------------------------------------------------------------------------
/** @brief Waits for I2C transaction completion
 *  @param None
 *  @return None
 */

static FW_BOOLEAN uart_WaitForComplete(U32 eventMask)
{
  FW_BOOLEAN result = FW_TRUE;
  EventBits_t events = 0;

  events = xEventGroupWaitBits
           (
             gDWire.events,
             eventMask,
             pdTRUE, // Clear bits
             pdTRUE, // Wait for all
             DWIRE_TIMEOUT
           );

  if (eventMask != (events & eventMask))
  {
    result = FW_FALSE;
  }

  return result;
}

//-----------------------------------------------------------------------------
/** @brief Performs the I2C read transaction
 *  @param pReq - Pointer to the request container
 *  @param pRsp - Pointer to the response container
 *  @param pSize - Pointer to the size container
 *  @return None
 */

static U32 uart_Read(U32 size) //U8 * pBuffer, U32 size)
{
  U32 i = 0;

  gDWire.rxLen = 0;

  UART_RxStart(UART2);

  //if (FW_TRUE ==
  if (FW_TRUE == uart_WaitForComplete(DWIRE_RX_COMPLETE))
  {
    DWIRE_LOG("DWire <-- ");
    for (i = 0; i < gDWire.rxLen; i++)
    {
      DWIRE_LOG("%02X ", gDWire.rxBuffer[i]);
    }
    DWIRE_LOG("\r\n");
  }
  //{
  //  pRsp->status = I2C_STATUS_SUCCESS;
  //  pRsp->size = pReq->size;
  //  *pSize = (pReq->size + 4);
  //}
  //else
  //{
  //  cdc_I2cError(pReq, pRsp, pSize);
  //}

  return gDWire.rxLen;
}

//-----------------------------------------------------------------------------
/** @brief Performs the I2C write transaction
 *  @param pReq - Pointer to the request container
 *  @param pRsp - Pointer to the response container
 *  @param pSize - Pointer to the size container
 *  @return None
 */

static FW_BOOLEAN uart_WriteRead(void) //U8 * pBuffer, U32 size)
{
  //gTxLen = size;
  FW_BOOLEAN result = FW_FALSE;
  U32 i = 0; //, len = gTxLen;

  gDWire.rxLen = 0;
  gDWire.txInProgress = FW_TRUE;
  gDWire.txOk = FW_TRUE;

  //DWIRE_LOG("DWire: Received: L = %d, B[0] = %02X\r\n", gRxLen, gRxBuffer[0]);
  DWIRE_LOG("DWire --> ");
  for (i = 0; i < gDWire.txLen; i++)
  {
    DWIRE_LOG("%02X ", gDWire.txBuffer[i]);
  }
  DWIRE_LOG("\r\n");

  UART_TxStart(UART2);

  //if (FW_TRUE ==
  result = uart_WaitForComplete(DWIRE_TX_COMPLETE | DWIRE_RX_COMPLETE);
  //{
  //  pRsp->status = I2C_STATUS_SUCCESS;
  //  pRsp->size = pReq->size;
  //  *pSize = 4;
  //}
  //else
  //{
  //  cdc_I2cError(pReq, pRsp, pSize);
  //}

//  if (FW_FALSE != result)
//  {
//    if ( (len <= gRxLen) && (0 == memcmp(gTxBuffer, gRxBuffer, len)) )
//    {
//      result = FW_TRUE;
//    }
//  }

  if ( (FW_TRUE == result) && (0 < gDWire.rxLen) )
  {
    DWIRE_LOG("DWire <-- ");
    for (i = 0; i < gDWire.rxLen; i++)
    {
      DWIRE_LOG("%02X ", gDWire.rxBuffer[i]);
    }
    DWIRE_LOG("\r\n");
  }

  return result;
}









//-----------------------------------------------------------------------------

static void dwire_BaudCaptComplete(TIM_CH_t aChannel, U16 aValue)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  GPIO_Hi(GPIOB, 5);
  gDWire.baudRateValue[gDWire.baudRateCnt++] = aValue;
  if (DWIRE_BAUD_MEAS_COUNT == gDWire.baudRateCnt)
  {
    (void)xEventGroupSetBitsFromISR
      (
        gDWire.events,
        DWIRE_BAUD_COMPLETE,
        &xHigherPriorityTaskWoken
      );

    if (xHigherPriorityTaskWoken)
    {
      taskYIELD();
    }
  }
  GPIO_Lo(GPIOB, 5);
}

//-----------------------------------------------------------------------------

FW_BOOLEAN DWire_Sync(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U32 idx           = 0;
  U32 baudrate      = 0;
  U16 time          = 0;

  DWIRE_LOG("DWire: Sync\r\n");

  memset(gDWire.baudRateValue, 0, sizeof(gDWire.baudRateValue));
  gDWire.baudRateCnt = 0;

  /* Init timer */
  TIM2_InitInputCapture(TIM_CH3, dwire_BaudCaptComplete);

  /* Activate the Reset pin */
  GPIO_Init(TIM2_CH3_PORT, TIM2_CH3_PIN, GPIO_TYPE_OUT_OD_2MHZ, 0);
  vTaskDelay(5);

  /* Enable the timer */
  TIM2_Enable();

  /* Release the Reset pin */
  GPIO_Init(TIM2_CH3_PORT, TIM2_CH3_PIN, GPIO_TYPE_IN_FLOATING, 0);

  /* Wait for measurement is complete */
  result = uart_WaitForComplete(DWIRE_BAUD_COMPLETE);

  /* Disable the timer */
  TIM2_Disable();
  TIM2_DeInit();

  if (FW_TRUE == result)
  {
    /* Calculate the baudrate */
    baudrate = 0;
    for (idx = DWIRE_BAUD_MEAS_START_IDX; idx < DWIRE_BAUD_MEAS_COUNT; idx++)
    {
      time = gDWire.baudRateValue[idx] - gDWire.baudRateValue[idx - 1];
      DWIRE_LOG(" - Meas[%d]  = %d\r\n", idx - 2, time);

      baudrate += time;
    }
    baudrate /= (DWIRE_BAUD_MEAS_COUNT - DWIRE_BAUD_MEAS_START_IDX);
    baudrate = 2 * CPUClock / baudrate;
    DWIRE_LOG(" - BaudRate = %d\r\n", baudrate);

    /* Initialize the UART */
    UART_Init
    (
      UART2,
      baudrate,
      uart_RxByte,
      uart_RxComplete,
      uart_TxByte,
      uart_TxComplete
    );

    /* Enable the bidirectional mode */
    UART_BiDirModeEn(UART2);

    /* Activate the Reset pin */
    GPIO_Init(UART2_RTX_PORT, UART2_RTX_PIN, GPIO_TYPE_OUT_OD_2MHZ, 0);
    vTaskDelay(5);

    /* Release the Reset pin */
    GPIO_Init(UART2_RTX_PORT, UART2_RTX_PIN, GPIO_TYPE_ALT_OD_2MHZ, 1);

    /* Wait for the Sync byte from the target */
    idx = uart_Read(1);//gRxBuffer, 1);

    if ( (1 == idx) && (0x55 == gDWire.rxBuffer[0]) )
    {
      DWIRE_LOG("DWire: In sync!\r\n");
    }
    else
    {
      result = FW_FALSE;
      DWIRE_LOG("DWire: Sync error!\r\n");
    }

//  vTaskDelay(50);
//
//  gTxBuffer[0] = 0x07;
//  gTxLen = 1;
//  DBG("DWire: Transmit: L = %d, B[0] = %02X\r\n", gTxLen, gTxBuffer[0]);
//  uart_Write(gTxBuffer, 1);
//
//  (void)uart_Read(gRxBuffer, 1);
//  DBG("DWire: Received: L = %d, B[0] = %02X\r\n", gRxLen, gRxBuffer[0]);
  }

//  (void)uart_Read(gRxBuffer, 1);
//  DBG("DWire: Received: L = %d, B[0] = %02X\r\n", gRxLen, gRxBuffer[0]);
//
//  vTaskDelay(50);
//
//  gTxBuffer[0] = 0x07;
//  gTxLen = 1;
//  DBG("DWire: Transmit: L = %d, B[0] = %02X\r\n", gTxLen, gTxBuffer[0]);
//  uart_Write(gTxBuffer, 1);
//
//  (void)uart_Read(gRxBuffer, 1);
//  DBG("DWire: Received: L = %d, B[0] = %02X\r\n", gRxLen, gRxBuffer[0]);
//
//  while(1)
//  {
//    vTaskDelay(500);
//  }

  return result;
}





/* -------------------------------------------------------------------------- */

U16 DWire_ReadSignature(void)
{
  U16 result = 0;

  DWIRE_LOG("DWire: Read Signature\r\n");

  gDWire.txBuffer[0] = DWIRE_GET_SIG;
  gDWire.txLen = 1;
  if (FW_TRUE == uart_WriteRead())
  {
    //if (2 == uart_Read(2))
    if (2 == gDWire.rxLen)
    {
      result = gDWire.rxBuffer[1] + (gDWire.rxBuffer[0] << 8);
      DWIRE_LOG("DWire: Signature = %04X\r\n", result);
    }
  }

  return result;
}

/* -------------------------------------------------------------------------- */

//static void uart_Open(void)
//{
//  /* UART3: PB10 - Tx, PB11 - Rx */
//  GPIO_Init(UART3_TX_PORT, UART3_TX_PIN, GPIO_TYPE_IN_PUP_PDN, 1);
//  GPIO_Init(UART3_RX_PORT, UART3_RX_PIN, GPIO_TYPE_IN_PUP_PDN, 1);
//
//  //UART_DeInit(UART2);
//  UART_Init
//  (
//    UART2,
//    86400,
//    uart_RxByte,
//    uart_RxComplete,
//    uart_TxByte,
//    uart_TxComplete
//  );
//
//  /* UART3: PB10 - Tx, PB11 - Rx */
//  GPIO_Init(UART3_TX_PORT, UART3_TX_PIN, GPIO_TYPE_ALT_PP_10MHZ, 1);
//  GPIO_Init(UART3_RX_PORT, UART3_RX_PIN, GPIO_TYPE_IN_PUP_PDN,   1);
//
//  UART_RxStart(UART3);
//}

/* -------------------------------------------------------------------------- */

void dwire_Task(void * pvParameters)
{
  DWIRE_LOG("Debug Wire Task Started...\r\n");

//  (void)dwire_Sync();

//  UART_Init
//  (
//    UART2,
//    86400,
//    uart_RxByte,
//    uart_RxComplete,
//    uart_TxByte,
//    uart_TxComplete
//  );
//
//  USART2->CR3 |= USART_CR3_HDSEL;
//
//  GPIO_Init(UART2_RTX_PORT, UART2_RTX_PIN, GPIO_TYPE_OUT_OD_2MHZ, 0);
//  //GPIO_Lo(UART2_RTX_PORT, UART2_RTX_PIN);
//  vTaskDelay(50);
//  //GPIO_Hi(UART2_RTX_PORT, UART2_RTX_PIN);
//  //vTaskDelay(2);
//
//  GPIO_Init(UART2_RTX_PORT, UART2_RTX_PIN, GPIO_TYPE_ALT_OD_2MHZ, 1);
//
//  (void)uart_Read(gRxBuffer, 1);
//  DBG("DWire: Received: L = %d, B[0] = %02X\r\n", gRxLen, gRxBuffer[0]);
//
//  vTaskDelay(50);
//
//  gTxBuffer[0] = 0x07;
//  gTxLen = 1;
//  DBG("DWire: Transmit: L = %d, B[0] = %02X\r\n", gTxLen, gTxBuffer[0]);
//  uart_Write(gTxBuffer, 1);
//
//  (void)uart_Read(gRxBuffer, 1);
//  DBG("DWire: Received: L = %d, B[0] = %02X\r\n", gRxLen, gRxBuffer[0]);

  while(1)
  {
    vTaskDelay(500);
  }
}

/* -------------------------------------------------------------------------- */

void DWire_Init(void)
{
  /* Event Group for flow control */
  if (NULL == gDWire.events)
  {
    gDWire.events = xEventGroupCreate();
  }

  /* Init the test pin (temporarily) */
  GPIO_Init(GPIOB, 5, GPIO_TYPE_OUT_PP_50MHZ, 0);
  GPIO_Init(GPIOB, 9, GPIO_TYPE_OUT_PP_50MHZ, 0);

//  /* FIFOs */
//  gRxFifo = FIFO_Init(gRxBuffer, sizeof(gRxBuffer));
//  gTxFifo = FIFO_Init(gTxBuffer, sizeof(gTxBuffer));

  /* Create Semaphores/Mutex for VCP */
//  gVcpSemRx = xSemaphoreCreateBinary();
//  gVcpMutRx = xSemaphoreCreateMutex();
//  gVcpSemTx = xSemaphoreCreateBinary();
//  gVcpMutTx = xSemaphoreCreateMutex();

//  U8 * buffer = NULL;
//  U32 size = 0;

  /* Clear Port context */
  //memset(&gPort, 0, sizeof(gPort));
  /* Port is not ready yet */
  //gPort.cdc.ready = FW_FALSE;

  /* Create the event group for synchronization */
//  gIceMkII.events = xEventGroupCreate();
//  (void)xEventGroupClearBits
//        (
//          gIceMkII.events,
//          ICEMKII_TX_COMPLETE //| EVT_I2C_EXCH_COMPLETE
//        );

  /* Create the Debug Wire task */
//  xTaskCreate
//  (
//    dwire_Task,
//    "DWIRE",
//    2 * configMINIMAL_STACK_SIZE,
//    NULL,
//    tskIDLE_PRIORITY + 1,
//    NULL
//  );
}