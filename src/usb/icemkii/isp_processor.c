//#include "hardware.h"
//#include <avr/io.h>
//#include <intrinsics.h>
//#include <avr/interrupt.h>
//#include "utils.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "isp_processor.h"

#include "board.h"
#include "spi.h"
#include "debug.h"

/* -------------------------------------------------------------------------- */

#define ISP_DEBUG

#ifdef ISP_DEBUG
#  define ISP_LOG                 DBG
#else
#  define ISP_LOG(...)
#endif
#define ISP_LOG_FUNC_START()      ISP_LOG("--- ISP Start ----------------\r\n")
#define ISP_LOG_FUNC_END()        ISP_LOG("--- ISP End ------------------\r\n")
#define ISP_LOG_SUCCESS()         ISP_LOG(" - Success\r\n")
#define ISP_LOG_ERROR()           ISP_LOG(" - Fail\r\n")

#define ISP_VERSION_HW            (1)
#define ISP_VERSION_MAJOR         (2)
#define ISP_VERSION_MINOR         (10)

#define EVT_SPI_EXCH_COMPLETE     (1 << 1)
#define EVT_SPI_EXCH_TIMEOUT      (100)

#define ISP_MAX_XFER_SIZE         (4)

#define ISP_POLL_DELAY_MS         (2)
#define ISP_PULSE_LENGTH_ITERS    (25)

#define MODE_MASK                 (1 << 0)
#define MODE_WORD                 (0 << 0) //Word/Page Mode (0 = word, 1 = page)
#define MODE_PAGE                 (1 << 0)
#define MODE_WORD_DELAY           (1 << 1) //Timed delay
#define MODE_WORD_VALUE           (1 << 2) //Value polling
#define MODE_WORD_RDY_BSY         (1 << 3) //RDY/BSY polling
#define MODE_PAGE_DELAY           (1 << 4) //Timed delay
#define MODE_PAGE_VALUE           (1 << 5) //Value polling
#define MODE_PAGE_RDY_BSY         (1 << 6) //RDY/BSY polling
#define MODE_PAGE_WRITE           (1 << 7) //Write page

/* -------------------------------------------------------------------------- */

ISP_PARAMETERS_t gIspParams =
{
  .hwVersion      = ISP_VERSION_HW,
  .swVersionMajor = ISP_VERSION_MAJOR,
  .swVersionMinor = ISP_VERSION_MINOR,
  .sckDuration    = 1,
};

static EventGroupHandle_t gEvents                      = NULL;
static FW_RESULT          gXferResult                  = FW_TIMEOUT;
static U8                 gTxBuffer[ISP_MAX_XFER_SIZE] = {0};
static U8                 gRxBuffer[ISP_MAX_XFER_SIZE] = {0};







//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** @brief SPI callback function
 *  @param aResult - SPI transaction result
 *  @return None
 */

static FW_BOOLEAN spi_Complete(FW_RESULT aResult)
{
  BaseType_t xHigherPriorityTaskWoken;

  xHigherPriorityTaskWoken = pdFALSE;

  (void)xEventGroupSetBitsFromISR
        (
          gEvents,
          EVT_SPI_EXCH_COMPLETE,
          &xHigherPriorityTaskWoken
        );
  gXferResult = aResult;

  if (xHigherPriorityTaskWoken)
  {
    taskYIELD();
  }

  return FW_TRUE;
}

//-----------------------------------------------------------------------------
/** @brief Waits for SPI transaction completion
 *  @param None
 *  @return None
 */

static FW_BOOLEAN spi_WaitForComplete(void)
{
  FW_BOOLEAN result = FW_TRUE;
  EventBits_t events = 0;

  events = xEventGroupWaitBits
           (
             gEvents,
             EVT_SPI_EXCH_COMPLETE,
             pdTRUE,
             pdFALSE,
             EVT_SPI_EXCH_TIMEOUT
           );

  if (EVT_SPI_EXCH_COMPLETE == (events & EVT_SPI_EXCH_COMPLETE))
  {
    if (FW_COMPLETE == gXferResult)
    {
      /* Success */
    }
    else
    {
      result = FW_FALSE;
    }
  }
  else
  {
    /* Timeout */
    result = FW_FALSE;
  }

  return result;
}

////-----------------------------------------------------------------------------
///** @brief Prepares the error response
// *  @param pReq - Pointer to the request container
// *  @param pRsp - Pointer to the response container
// *  @param pSize - Pointer to the size container
// *  @return None
// */
//
//static void cdc_SpiError(SPI_PACKET * pReq, SPI_PACKET * pRsp, U32 * pSize)
//{
//  pRsp->status = SPI_STATUS_ERROR;
//  pRsp->size = 0;
//  *pSize = 4;
//}

//-----------------------------------------------------------------------------
/** @brief Performs the SPI read transaction
 *  @param pReq - Pointer to the request container
 *  @param pRsp - Pointer to the response container
 *  @param pSize - Pointer to the size container
 *  @return None
 */

//static void isp_SpiRead(U8 * pBuffer, U32 size)
//{
////  GPIO_Lo(SPI1_CS_PORT, SPI1_CS_PIN);
//  SPI_MExchange(SPI_1, NULL, pBuffer, size);
//
//  if (FW_TRUE == spi_WaitForComplete())
//  {
//    //pRsp->status = SPI_STATUS_SUCCESS;
//    //pRsp->size = pReq->size;
//    //*pSize = (pReq->size + 4);
//  }
//  else
//  {
//    //cdc_SpiError(pReq, pRsp, pSize);
//  }
////  GPIO_Hi(SPI1_CS_PORT, SPI1_CS_PIN);
//}

////-----------------------------------------------------------------------------
///** @brief Performs the SPI write transaction
// *  @param pReq - Pointer to the request container
// *  @param pRsp - Pointer to the response container
// *  @param pSize - Pointer to the size container
// *  @return None
// */
//
//static void cdc_SpiWrite(U8 * pBuffer, U32 size)
//{
////  GPIO_Lo(SPI1_CS_PORT, SPI1_CS_PIN);
//  SPI_MExchange(SPI_1, pBuffer, NULL, size);
//
//  if (FW_TRUE == spi_WaitForComplete())
//  {
//    //pRsp->status = SPI_STATUS_SUCCESS;
//    //pRsp->size = pReq->size;
//    //*pSize = 4;
//  }
//  else
//  {
//    //cdc_SpiError(pReq, pRsp, pSize);
//  }
////  GPIO_Hi(SPI1_CS_PORT, SPI1_CS_PIN);
//}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------












//static U8 ispClockDelay;
//static U8 cmdBuffer[4];

/* ------------------------------------------------------------------------- */
/* We disable interrupts while transfer a byte. This ensures that we execute
 * at nominal speed, in spite of aggressive USB polling.
 */
//static U8 isp_BlockTransfer(U8 * pBlock, U8 size)
//{
////  U8 /*cnt,*/ result = 0/*, port, delay = ispClockDelay*/;
//
/////* minimum clock pulse width:
//// * 5 + 4 * delay clock cycles           -> Tmin = 750 ns
//// * total clock period: 12 + 8 * delay   -> fmax = 600 kHz
//// */
/////*    DBG2(0x40, block, len); */
////    cli();
////    port = PORT_OUT(HWPIN_ISP_MOSI) & ~(1 << PORT_BIT(HWPIN_ISP_MOSI));
////    do{
////        cnt = 8;
////        shift = *block++;
////        do{
////            if(shift & 0x80){
////                port |= (1 << PORT_BIT(HWPIN_ISP_MOSI));
////            }
////            PORT_OUT(HWPIN_ISP_MOSI) = port;
////            sei();
////            timerTicksDelay(delay);
////            cli();
////            PORT_PIN_SET(HWPIN_ISP_SCK);
////            shift <<= 1;
////            port &= ~(1 << PORT_BIT(HWPIN_ISP_MOSI));
////            if(PORT_PIN_VALUE(HWPIN_ISP_MISO)) /* driver is inverting */
////                shift |= 1;
////            sei();
////            timerTicksDelay(delay);
////            cli();
////            PORT_PIN_CLR(HWPIN_ISP_SCK);
////        }while(--cnt);
////    }while(--len);
////    sei();
/////*    DBG2(0x41, &shift, 1); */
//
//  SPI_MExchange(SPI_1, pBlock, gRxBuffer, size);
//
//  if (FW_FALSE == spi_WaitForComplete())
//  {
//    gRxBuffer[size - 1] = 0xFF;
//  }
//
//  return gRxBuffer[size - 1];
//}


static U8 isp_Exchange(U8 * pTxBuffer, U32 retSize)
{
  SPI_MExchange(SPI_1, pTxBuffer, gRxBuffer, ISP_MAX_XFER_SIZE);

  if (FW_FALSE == spi_WaitForComplete())
  {
    gRxBuffer[retSize - 1] = 0xFF;
  }

  return gRxBuffer[retSize - 1];
}



//-----------------------------------------------------------------------------

static void isp_InsertClockPulse(void)
{
  volatile U8 timeout = 0;

  /* Reinit the SCK pin as GPIO */
  GPIO_Init(SPI1_SCK_PORT, SPI1_SCK_PIN, GPIO_TYPE_OUT_PP_10MHZ, 0);

  /* Set SCK line */
  GPIO_Hi(SPI1_SCK_PORT, SPI1_SCK_PIN);
  for (timeout = 0; timeout < ISP_PULSE_LENGTH_ITERS; timeout++) portNOP();

  /* Clear SCK line */
  GPIO_Lo(SPI1_SCK_PORT, SPI1_SCK_PIN);
  for (timeout = 0; timeout < ISP_PULSE_LENGTH_ITERS; timeout++) portNOP();

  /* Reinit the SCK pin as SPI SCK function */
  GPIO_Init(SPI1_SCK_PORT, SPI1_SCK_PIN, GPIO_TYPE_ALT_PP_10MHZ, 0);
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN isp_IsDeviceBusy(void)
{
  gTxBuffer[0] = 0xF0;
  gTxBuffer[1] = 0x00;
  gTxBuffer[2] = 0x00;
  gTxBuffer[3] = 0x00;
  return (FW_BOOLEAN)(isp_Exchange(gTxBuffer, ISP_MAX_XFER_SIZE) & 1);
}

//-----------------------------------------------------------------------------

static FW_RESULT isp_WaitUntilReady(U32 msTimeOut)
{
  U32 time = xTaskGetTickCount() + msTimeOut;

  while (FW_TRUE == isp_IsDeviceBusy())
  {
    if (time < xTaskGetTickCount())
    {
      return FW_TIMEOUT;
    }

    vTaskDelay(ISP_POLL_DELAY_MS);
  }
  return FW_SUCCESS;
}

//-----------------------------------------------------------------------------

static void isp_AttachToDevice(void)
{
  volatile U8 timeout = 0;

  /* Init the SPI */
  SPI_Init(SPI_1, spi_Complete);

  /* Setup initial condition: SCK, MOSI = 0 */
  GPIO_Init(SPI1_RST_PORT,  SPI1_RST_PIN,  GPIO_TYPE_OUT_OD_10MHZ, 1);
  GPIO_Init(SPI1_SCK_PORT,  SPI1_SCK_PIN,  GPIO_TYPE_OUT_PP_10MHZ, 0);
  GPIO_Init(SPI1_MOSI_PORT, SPI1_MOSI_PIN, GPIO_TYPE_OUT_PP_10MHZ, 0);
  GPIO_Init(SPI1_MISO_PORT, SPI1_MISO_PIN, GPIO_TYPE_IN_FLOATING,  0);

  vTaskDelay(ISP_POLL_DELAY_MS);

  /* Set the SPI baudrate */
  if (0 == gIspParams.sckDuration)
  {
    /* 1.8 MHz nominal */
    //ispClockDelay = 0;
  }
  else if (1 == gIspParams.sckDuration)
  {
    /* 460 kHz nominal */
    //ispClockDelay = 1;
  }
  else if (2 == gIspParams.sckDuration)
  {
    /* 115 kHz nominal */
    //ispClockDelay = 2;
  }
  else if (3 == gIspParams.sckDuration)
  {
    /* 58 kHz nominal */
    //ispClockDelay = 3;
  }
  else
  {
    //ispClockDelay = 1 + gIspParams.sckDuration/4 + gIspParams.sckDuration/16;
  }

  /* Clear RESET */
  GPIO_Lo(SPI1_RST_PORT, SPI1_RST_PIN);

  /* Attach to device: */
  vTaskDelay(gIspParams.stabDelay);
  /* stabDelay may have been 0 */
  vTaskDelay(ISP_POLL_DELAY_MS);

  /* We now need to give a positive pulse on RESET since we can't guarantee
   * that SCK was low during power up (according to instructions in Atmel's
   * data sheets).
   */
  /* Give a positive RESET pulse */
  GPIO_Hi(SPI1_RST_PORT, SPI1_RST_PIN);
  for (timeout = 0; timeout < ISP_PULSE_LENGTH_ITERS; timeout++) portNOP();
  GPIO_Lo(SPI1_RST_PORT, SPI1_RST_PIN);

  /* Create the event group for synchronization */
  if (NULL == gEvents)
  {
    gEvents = xEventGroupCreate();
  }
  vTaskDelay(ISP_POLL_DELAY_MS);

  /* Reinit the pins as SPI */
  GPIO_Init(SPI1_SCK_PORT,  SPI1_SCK_PIN,  GPIO_TYPE_ALT_PP_10MHZ, 0);
  GPIO_Init(SPI1_MOSI_PORT, SPI1_MOSI_PIN, GPIO_TYPE_ALT_PP_10MHZ, 0);
  GPIO_Init(SPI1_MISO_PORT, SPI1_MISO_PIN, GPIO_TYPE_ALT_PP_10MHZ, 0);
}

//-----------------------------------------------------------------------------

static void isp_DetachFromDevice(void)
{
  /* Reinit the SPI pins */
  GPIO_Init(SPI1_SCK_PORT,  SPI1_SCK_PIN,  GPIO_TYPE_OUT_PP_10MHZ, 0);
  GPIO_Init(SPI1_MOSI_PORT, SPI1_MOSI_PIN, GPIO_TYPE_OUT_PP_10MHZ, 0);
  GPIO_Init(SPI1_MISO_PORT, SPI1_MISO_PIN, GPIO_TYPE_IN_FLOATING,  0);

  /* Reset the target */
  GPIO_Lo(SPI1_RST_PORT, SPI1_RST_PIN);
  vTaskDelay(gIspParams.preDelay);
  GPIO_Hi(SPI1_RST_PORT, SPI1_RST_PIN);

  /* Deinit the SPI pins */
  GPIO_Init(SPI1_SCK_PORT,  SPI1_SCK_PIN,  GPIO_TYPE_IN_FLOATING,  0);
  GPIO_Init(SPI1_MOSI_PORT, SPI1_MOSI_PIN, GPIO_TYPE_IN_FLOATING,  0);
  GPIO_Init(SPI1_RST_PORT,  SPI1_RST_PIN,  GPIO_TYPE_OUT_OD_10MHZ, 1);
}

//-----------------------------------------------------------------------------

FW_RESULT ISP_EnterProgMode(void)
{
  FW_RESULT result = FW_FAIL;
  U8 rval = 0;
  U8 i = 0;

  ISP_LOG_FUNC_START();
  ISP_LOG("Enter Prog Mode\r\n");

  isp_AttachToDevice();
  vTaskDelay(gIspParams.cmdDelay);

  /* Try to sync */
  for (i = gIspParams.synchLoops; i > 0; i--)
  {
    rval = isp_Exchange(gIspParams.cmd, gIspParams.pollIndex);

    if (rval == gIspParams.pollValue[0])
    {
      /* Success: we are in sync */
      ISP_LOG_SUCCESS();
      result = FW_SUCCESS;
      break;
    }

    /* Insert one clock pulse and try again */
    isp_InsertClockPulse();
  }

  if (FW_FAIL == result)
  {
    ISP_LOG_ERROR();
    isp_DetachFromDevice();
  }

  ISP_LOG_FUNC_END();

  return result;
}

//-----------------------------------------------------------------------------

void ISP_LeaveProgmode(void)
{
  ISP_LOG_FUNC_START();
  ISP_LOG("Leave Prog Mode\r\n");

  isp_DetachFromDevice();
  vTaskDelay(gIspParams.postDelay);

  ISP_LOG_FUNC_END();
}

//-----------------------------------------------------------------------------

FW_RESULT ISP_ChipErase(void)
{
  FW_RESULT rval = FW_SUCCESS;

  ISP_LOG_FUNC_START();
  ISP_LOG("Chip Erase");

  /* Send the command */
  (void)isp_Exchange(gIspParams.cmd, ISP_MAX_XFER_SIZE);
  if (0 != gIspParams.pollMethod)
  {
    /* Allow at least 10 ms */
    if (10 > gIspParams.eraseDelay)
    {
      gIspParams.eraseDelay = 10;
    }

    rval = isp_WaitUntilReady(gIspParams.eraseDelay);
  }
  else
  {
    vTaskDelay(gIspParams.eraseDelay);
  }

  if (FW_SUCCESS == rval)
  {
    ISP_LOG_SUCCESS();
  }
  else
  {
    ISP_LOG_ERROR();
  }
  ISP_LOG_FUNC_END();

  return rval;
}

//-----------------------------------------------------------------------------

FW_RESULT ISP_ProgramMemory(U8 * pBuffer, U32 size, ISP_MEMORY_t memory)
{
  FW_RESULT rval = FW_SUCCESS;
  U8 valuePollingMask = 0, rdyPollingMask = 0;
  U32 i = 0, time = 0;

  ISP_LOG_FUNC_START();
  ISP_LOG("Program Memory");
  ISP_LOG("Address = %08X\r\n", gIspParams.address.value);
  ISP_LOG("Size    = %d\r\n", size);

  /* Prepare the parameters */
  if (MODE_PAGE == (gIspParams.mode & MODE_MASK))
  {
    /* Page mode */
    valuePollingMask = MODE_PAGE_VALUE; //0x20;
    rdyPollingMask = MODE_PAGE_RDY_BSY; //0x40;
  }
  else
  {
    /* Word mode */
    valuePollingMask = MODE_WORD_VALUE; //4;
    rdyPollingMask = MODE_WORD_RDY_BSY; //8;
  }

  /* Programm the memory */
  for (i = 0; (rval == FW_SUCCESS) && (i < size); i++)
  {
    /* Load the data to the target */
    gTxBuffer[0] = gIspParams.cmd[0];
    gTxBuffer[1] = gIspParams.address.byte[1];
    gTxBuffer[2] = gIspParams.address.byte[0];
    gTxBuffer[3] = pBuffer[i];

    if (ISP_FLASH == memory)
    {
      gTxBuffer[0] &= ~(0x08);
      if (1 == (i & 1))
      {
        gTxBuffer[0] |= 0x08;
        gIspParams.address.value++;
      }
    }
    else
    {
      gIspParams.address.value++;
    }
    (void)isp_Exchange(gTxBuffer, ISP_MAX_XFER_SIZE);


    /* Check if program page command needed */
    if (MODE_PAGE == (gIspParams.mode & MODE_MASK))
    {
      /* Page mode */
      if ( (i < (size - 1)) || !(gIspParams.mode & 0x80) )
      {
        /* Not last byte written */
        continue;
      }

      /* Write program memory page */
      gTxBuffer[0] = gIspParams.cmd[1];
      gTxBuffer[3] = 0;
      (void)isp_Exchange(gTxBuffer, ISP_MAX_XFER_SIZE);
    }

    /* Poll for ready after each byte (word mode) or page (page mode) */
    if (gIspParams.mode & valuePollingMask)
    {
      /* Value polling */
      U8 d = pBuffer[i];
      if ( (d == gIspParams.pollValue[0]) || (d == gIspParams.pollValue[1]) )
      {
        /* Must use timed polling */
        vTaskDelay(gIspParams.cmdDelay);
      }
      else
      {
        /* Read flash */
        gTxBuffer[0] = gIspParams.cmd[2];
        gTxBuffer[0] &= ~(0x08);
        if (1 == (i & 1))
        {
          gTxBuffer[0] |= (0x08);
        }

        time = xTaskGetTickCount() + gIspParams.cmdDelay;
        while (isp_Exchange(gTxBuffer, ISP_MAX_XFER_SIZE) != d)
        {
          if (time < xTaskGetTickCount())
          {
            rval = FW_TIMEOUT;
            break;
          }
          vTaskDelay(ISP_POLL_DELAY_MS);
        }
      }
    }
    else if (gIspParams.mode & rdyPollingMask)
    {
      /* Rdy/Bsy polling */
      rval = isp_WaitUntilReady(gIspParams.cmdDelay);
    }
    else
    {
      /* Must be timed delay */
      vTaskDelay(gIspParams.cmdDelay);
    }
  }

  if (FW_SUCCESS == rval)
  {
    ISP_LOG_SUCCESS();
  }
  else
  {
    ISP_LOG_ERROR();
  }
  ISP_LOG_FUNC_END();

  return rval;
}

//-----------------------------------------------------------------------------

FW_RESULT ISP_ReadMemory(U8 * pBuffer, U32 size, ISP_MEMORY_t memory)
{
  U32 i = 0;

  ISP_LOG_FUNC_START();
  ISP_LOG("Read Memory\r\n");
  ISP_LOG("Address = %08X\r\n", gIspParams.address.value);
  ISP_LOG("Size    = %d\r\n", size);

  gTxBuffer[0] = gIspParams.cmd[0];
  gTxBuffer[3] = 0;
  for (i = 0; i < size; i++)
  {
    gTxBuffer[1] = gIspParams.address.byte[1];
    gTxBuffer[2] = gIspParams.address.byte[0];

    if (ISP_EEPROM == memory)
    {
      gIspParams.address.value++;
    }
    else
    {
      gTxBuffer[0] &= ~(0x08);
      if (1 == (i & 1))
      {
        gTxBuffer[0] |= (0x08);
        gIspParams.address.value++;
      }
    }
    *pBuffer++ = isp_Exchange(gTxBuffer, ISP_MAX_XFER_SIZE);
  }

  ISP_LOG_FUNC_END();

  return FW_SUCCESS;
}

//-----------------------------------------------------------------------------

void ISP_ProgramFLSO(void)
{
  ISP_LOG_FUNC_START();
  ISP_LOG("Program Fuse/Lock/Osc\r\n");

  (void)isp_Exchange(gIspParams.cmd, ISP_MAX_XFER_SIZE);

  ISP_LOG_FUNC_END();
}

//-----------------------------------------------------------------------------

U8 ISP_ReadFLSO(void)
{
  U8 rval = 0;

  ISP_LOG_FUNC_START();
  ISP_LOG("Read Fuse/Lock/Osc/Signature\r\n");

  rval = isp_Exchange(gIspParams.cmd, gIspParams.pollIndex);

  ISP_LOG_FUNC_END();

  return rval;
}

//-----------------------------------------------------------------------------

//uint    ispMulti(stkMultiIsp_t *param, stkMultiIspResult_t *result)
//{
//U8   cnt1, i, *p;
//
//    cnt1 = param->numTx;
//    if(cnt1 > param->rxStartAddr)
//        cnt1 = param->rxStartAddr;
//    ispBlockTransfer(param->txData, cnt1);
//
//    p = result->rxData;
//    for(i = 0; i < param->numTx - cnt1; i++){
//        U8 b = ispBlockTransfer(&param->txData[cnt1] + i, 1);
//        if(i < param->numRx)
//            *p++ = b;
//        wdt_reset();
//    }
//
//    for(; i < param->numRx; i++){
//        cmdBuffer[0] = 0;
//        *p++ = ispBlockTransfer(cmdBuffer, 1);
//        wdt_reset();
//    }
//    *p = result->status1 = STK_STATUS_CMD_OK;
//    return (uint)param->numRx + 2;
//}

//-----------------------------------------------------------------------------

void ISP_ResetTarget(void)
{
  ISP_LOG_FUNC_START();
  ISP_LOG("Reset target\r\n");

  /* Reinit the Reset pin */
  GPIO_Init(SPI1_RST_PORT,  SPI1_RST_PIN,  GPIO_TYPE_OUT_OD_10MHZ, 1);

  /* Reset the target */
  GPIO_Lo(SPI1_RST_PORT, SPI1_RST_PIN);
  vTaskDelay(gIspParams.preDelay);
  GPIO_Hi(SPI1_RST_PORT, SPI1_RST_PIN);
  vTaskDelay(gIspParams.postDelay);

  ISP_LOG_FUNC_END();
}

//-----------------------------------------------------------------------------
