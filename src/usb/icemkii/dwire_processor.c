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

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"


static U8                 gTxBuffer[16] = {0};
static U32                gTxLen        = 0;
static U8                 gRxBuffer[16] = {0};
static U32                gRxLen        = 0;
static EventGroupHandle_t gEvents       = NULL;

#define DWIRE_TX_COMPLETE         (1 << 0)
#define DWIRE_RX_COMPLETE         (1 << 1)
#define DWIRE_TIMEOUT             (100)

//-----------------------------------------------------------------------------
/** @brief Puts received Byte from UART to the Tx FIFO
 *  @param pByte - Pointer to the container for Byte
 *  @return None
 */

static FW_BOOLEAN uart_RxByte(U8 * pByte)
{
  if (sizeof(gRxBuffer) > gRxLen)
  {
    gRxBuffer[gRxLen] = *pByte;
    gRxLen++;
    return FW_TRUE;
  }
  else
  {
    return FW_FALSE;
  }
}

//-----------------------------------------------------------------------------
/** @brief Receive complete callback
 *  @param pByte - Optional pointer to the latest received byte (def. NULL)
 *  @return TRUE, that means UART line idle is received
 */

static FW_BOOLEAN uart_RxComplete(U8 * pByte)
{
  BaseType_t xHigherPriorityTaskWoken;

  xHigherPriorityTaskWoken = pdFALSE;

  (void)xEventGroupSetBitsFromISR
        (
          gEvents,
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
  if (0 < gTxLen)
  {
    gTxLen--;
    *pByte = gTxBuffer[gTxLen];
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
  BaseType_t xHigherPriorityTaskWoken;

  xHigherPriorityTaskWoken = pdFALSE;

  (void)xEventGroupSetBitsFromISR
        (
          gEvents,
          DWIRE_TX_COMPLETE,
          &xHigherPriorityTaskWoken
        );

  if (xHigherPriorityTaskWoken)
  {
    taskYIELD();
  }

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
             gEvents,
             eventMask,
             pdTRUE,
             pdFALSE,
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

static U32 uart_Read(U8 * pBuffer, U32 size)
{
  gRxLen = 0;
  UART_RxStart(UART2);

  //if (FW_TRUE ==
  (void)uart_WaitForComplete(DWIRE_RX_COMPLETE);
  //{
  //  pRsp->status = I2C_STATUS_SUCCESS;
  //  pRsp->size = pReq->size;
  //  *pSize = (pReq->size + 4);
  //}
  //else
  //{
  //  cdc_I2cError(pReq, pRsp, pSize);
  //}

  return gRxLen;
}

//-----------------------------------------------------------------------------
/** @brief Performs the I2C write transaction
 *  @param pReq - Pointer to the request container
 *  @param pRsp - Pointer to the response container
 *  @param pSize - Pointer to the size container
 *  @return None
 */

static void uart_Write(U8 * pBuffer, U32 size)
{
  gTxLen = size;
  UART_TxStart(UART2);

  //if (FW_TRUE ==
  (void)uart_WaitForComplete(DWIRE_TX_COMPLETE);
  //{
  //  pRsp->status = I2C_STATUS_SUCCESS;
  //  pRsp->size = pReq->size;
  //  *pSize = 4;
  //}
  //else
  //{
  //  cdc_I2cError(pReq, pRsp, pSize);
  //}
}













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
  DBG("Debug Wire Task Started...\r\n");

  UART_Init
  (
    UART2,
    86400,
    uart_RxByte,
    uart_RxComplete,
    uart_TxByte,
    uart_TxComplete
  );

  USART2->CR3 |= USART_CR3_HDSEL;

  GPIO_Init(UART2_RTX_PORT, UART2_RTX_PIN, GPIO_TYPE_OUT_OD_2MHZ, 0);
  //GPIO_Lo(UART2_RTX_PORT, UART2_RTX_PIN);
  vTaskDelay(50);
  //GPIO_Hi(UART2_RTX_PORT, UART2_RTX_PIN);
  //vTaskDelay(2);

  GPIO_Init(UART2_RTX_PORT, UART2_RTX_PIN, GPIO_TYPE_ALT_OD_2MHZ, 1);

  (void)uart_Read(gRxBuffer, 1);
  DBG("DWire: Received: L = %d, B[0] = %02X\r\n", gRxLen, gRxBuffer[0]);

  vTaskDelay(50);

  gTxBuffer[0] = 0x06;
  gTxLen = 1;
  DBG("DWire: Transmit: L = %d, B[0] = %02X\r\n", gTxLen, gTxBuffer[0]);
  uart_Write(gTxBuffer, 1);

  (void)uart_Read(gRxBuffer, 1);
  DBG("DWire: Received: L = %d, B[0] = %02X\r\n", gRxLen, gRxBuffer[0]);

  while(1)
  {
    vTaskDelay(500);
  }
}

/* -------------------------------------------------------------------------- */

void DWIRE_Init(void)
{
  /* Event Group for flow control */
  gEvents = xEventGroupCreate();

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
  xTaskCreate
  (
    dwire_Task,
    "DWIRE",
    2 * configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );
}