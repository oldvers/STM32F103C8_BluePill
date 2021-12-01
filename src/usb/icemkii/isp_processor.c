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



/* -------------------------------------------------------------------------- */

static void isp_InsertClockPulse(void)
{
  volatile U8 timeout = 0;

  GPIO_Init(SPI1_SCK_PORT, SPI1_SCK_PIN, GPIO_TYPE_OUT_PP_10MHZ, 0);
//        PORT_PIN_SET(HWPIN_ISP_SCK);
  GPIO_Hi(SPI1_SCK_PORT, SPI1_SCK_PIN);
  //vTaskDelay(1); //    timerTicksDelay(ispClockDelay);
  for (timeout = 0; timeout < ISP_PULSE_LENGTH_ITERS; timeout++) portNOP();
//        PORT_PIN_CLR(HWPIN_ISP_SCK);
  GPIO_Lo(SPI1_SCK_PORT, SPI1_SCK_PIN);
  //vTaskDelay(1); //    timerTicksDelay(ispClockDelay);
  for (timeout = 0; timeout < ISP_PULSE_LENGTH_ITERS; timeout++) portNOP();

  GPIO_Init(SPI1_SCK_PORT, SPI1_SCK_PIN, GPIO_TYPE_ALT_PP_10MHZ, 0);
}

static FW_BOOLEAN isp_IsDeviceBusy(void)
{
  gTxBuffer[0] = 0xF0;
  gTxBuffer[1] = 0x00;
  gTxBuffer[2] = 0x00;
  gTxBuffer[3] = 0x00;
  return (FW_BOOLEAN)(isp_Exchange(gTxBuffer, ISP_MAX_XFER_SIZE) & 1);
}

static FW_RESULT isp_WaitUntilReady(U32 msTimeOut)
{
  U32 time = xTaskGetTickCount() + msTimeOut;
  //timerSetupTimeout(msTimeout);
  while (FW_TRUE == isp_IsDeviceBusy())
  {
    //if (timerTimeoutOccurred())
    if (time < xTaskGetTickCount())
    {
      return FW_TIMEOUT; //STK_STATUS_RDY_BSY_TOUT;
    }

    vTaskDelay(ISP_POLL_DELAY_MS);
  }
  return FW_SUCCESS; //STK_STATUS_CMD_OK;
}

/* -------------------------------------------------------------------------- */

static void isp_AttachToDevice(void) //U8 stk500Delay, U8 stabDelay)
                                     //gIspParams.sckDuration, param->stabDelay);
{
  volatile U8 timeout = 0;

//  PORT_DDR_SET(HWPIN_ISP_SCK);
//  PORT_DDR_SET(HWPIN_ISP_MOSI);
//  PORT_DDR_SET(HWPIN_ISP_RESET);

  SPI_Init(SPI_1, spi_Complete);

  GPIO_Init(SPI1_RST_PORT,  SPI1_RST_PIN,  GPIO_TYPE_OUT_OD_10MHZ, 1);
  GPIO_Init(SPI1_SCK_PORT,  SPI1_SCK_PIN,  GPIO_TYPE_OUT_PP_10MHZ, 0);
  GPIO_Init(SPI1_MOSI_PORT, SPI1_MOSI_PIN, GPIO_TYPE_OUT_PP_10MHZ, 0);
  GPIO_Init(SPI1_MISO_PORT, SPI1_MISO_PIN, GPIO_TYPE_IN_FLOATING,  0);

  vTaskDelay(ISP_POLL_DELAY_MS);

 //PORT_PIN_SET(HWPIN_LED);
  //if(!PORT_PIN_VALUE(HWPIN_JUMPER)){      /* Jumper is set -> request clock below 8 kHz */
  //    ispClockDelay = (U8)(70/TIMER_TICK_US);   /* 140 us -> 7.14 kHz clock rate */
  //}else
  if (0 == gIspParams.sckDuration) //stk500Delay == 0)
  {
    /* 1.8 MHz nominal */
//    ispClockDelay = 0;
  }
  else if (1 == gIspParams.sckDuration) //stk500Delay == 1)
  {
    /* 460 kHz nominal */
//    ispClockDelay = 1;
  }
  else if (2 == gIspParams.sckDuration) //stk500Delay == 2){ /* 115 kHz nominal */
  {
//    ispClockDelay = 2;
  }
  else if (3 == gIspParams.sckDuration) //stk500Delay == 3){ /* 58 kHz nominal */
  {
//    ispClockDelay = 3;
  }
  else
  {
//    ispClockDelay = 1 + gIspParams.sckDuration/4 + gIspParams.sckDuration/16;
  }

  /* Setup initial condition: SCK, MOSI = 0 */
//  PORT_OUT(HWPIN_ISP_SCK) &= ~((1 << PORT_BIT(HWPIN_ISP_SCK)) | (1 << PORT_BIT(HWPIN_ISP_MOSI)));
//  PORT_PIN_CLR(HWPIN_ISP_RESET);  /* set RESET */
  GPIO_Lo(SPI1_RST_PORT, SPI1_RST_PIN);

  //PORT_DDR_CLR(HWPIN_ISP_DRIVER); /* make input: use internal pullup to control driver */
  //PORT_PIN_SET(HWPIN_ISP_DRIVER); /* attach to device: */
  //TCCR2 |= (1 << COM20);  /* set toggle on compare match mode -> activate clock */
  vTaskDelay(gIspParams.stabDelay); //timerMsDelay(stabDelay);
//  timerTicksDelay(ispClockDelay);    /* stabDelay may have been 0 */
  vTaskDelay(ISP_POLL_DELAY_MS);
  /* We now need to give a positive pulse on RESET since we can't guarantee
   * that SCK was low during power up (according to instructions in Atmel's
   * data sheets).
   */
//  PORT_PIN_SET(HWPIN_ISP_RESET);  /* give a positive RESET pulse */
  GPIO_Hi(SPI1_RST_PORT, SPI1_RST_PIN);
//  vTaskDelay(1); //timerTicksDelay(ispClockDelay);
  for (timeout = 0; timeout < ISP_PULSE_LENGTH_ITERS; timeout++) portNOP();
//  PORT_PIN_CLR(HWPIN_ISP_RESET);
  GPIO_Lo(SPI1_RST_PORT, SPI1_RST_PIN);



  if (NULL == gEvents)
  {
    /* Create the event group for synchronization */
    gEvents = xEventGroupCreate();
  }

  vTaskDelay(ISP_POLL_DELAY_MS);

  //SPI_Init(SPI_1, spi_Complete);

  GPIO_Init(SPI1_SCK_PORT,  SPI1_SCK_PIN,  GPIO_TYPE_ALT_PP_10MHZ, 0);
  GPIO_Init(SPI1_MOSI_PORT, SPI1_MOSI_PIN, GPIO_TYPE_ALT_PP_10MHZ, 0);
  GPIO_Init(SPI1_MISO_PORT, SPI1_MISO_PIN, GPIO_TYPE_ALT_PP_10MHZ, 0);
}

static void isp_DetachFromDevice(void) //U8 removeResetDelay)
{
//  GPIO_Init(SPI1_RST_PORT,  SPI1_RST_PIN,  GPIO_TYPE_OUT_PP_2MHZ,  1);
  GPIO_Init(SPI1_SCK_PORT,  SPI1_SCK_PIN,  GPIO_TYPE_OUT_PP_10MHZ, 0);
  GPIO_Init(SPI1_MOSI_PORT, SPI1_MOSI_PIN, GPIO_TYPE_OUT_PP_10MHZ, 0);
  GPIO_Init(SPI1_MISO_PORT, SPI1_MISO_PIN, GPIO_TYPE_IN_FLOATING,  0);

//  PORT_OUT(HWPIN_ISP_SCK) &= ~((1 << PORT_BIT(HWPIN_ISP_SCK)) | (1 << PORT_BIT(HWPIN_ISP_MOSI)));
//  PORT_PIN_CLR(HWPIN_ISP_RESET);
//  GPIO_Lo(SPI1_SCK_PORT,  SPI1_SCK_PIN);
//  GPIO_Lo(SPI1_MOSI_PORT, SPI1_MOSI_PIN);
  GPIO_Lo(SPI1_RST_PORT, SPI1_RST_PIN);

  vTaskDelay(gIspParams.preDelay); //timerMsDelay(removeResetDelay);
  //TCCR2 &= ~(1 << COM20);  /* clear toggle on compare match mode */
  //PORT_PIN_CLR(HWPIN_ISP_DRIVER); /* detach from device */
  //PORT_DDR_SET(HWPIN_ISP_DRIVER); /* set pin level to low-Z 0 */
//  PORT_PIN_SET(HWPIN_ISP_RESET);
  GPIO_Hi(SPI1_RST_PORT, SPI1_RST_PIN);
  //PORT_PIN_CLR(HWPIN_LED);

//  PORT_DDR_CLR(HWPIN_ISP_SCK);
//  PORT_DDR_CLR(HWPIN_ISP_MOSI);
//  PORT_DDR_CLR(HWPIN_ISP_RESET);
  GPIO_Init(SPI1_SCK_PORT,  SPI1_SCK_PIN,  GPIO_TYPE_IN_FLOATING,  0);
  GPIO_Init(SPI1_MOSI_PORT, SPI1_MOSI_PIN, GPIO_TYPE_IN_FLOATING,  0);
  GPIO_Init(SPI1_RST_PORT,  SPI1_RST_PIN,  GPIO_TYPE_OUT_OD_10MHZ, 1);
}

/* ------------------------------------------------------------------------- */

FW_RESULT ISP_EnterProgMode(void) //stkEnterProgIsp_t *param)
{
  FW_RESULT result = FW_FAIL;
  U8 rval = 0;
  U8 i = 0;

  ISP_LOG_FUNC_START();
  ISP_LOG("Enter Prog Mode\r\n");

  isp_AttachToDevice();//gIspParams.sckDuration, param->stabDelay);

  vTaskDelay(gIspParams.cmdExeDelay); //timerMsDelay(param->cmdExeDelay);

  /* we want for(i = param->synchLoops; i--;), but avrdude sends synchLoops == 0 */
  for (i = gIspParams.synchLoops; i > 0; i--) //for (i = 32; i--;){
  {
//        wdt_reset();
//    rval = isp_BlockTransfer(gIspParams.cmd, gIspParams.pollIndex);
//    if (gIspParams.pollIndex < 4)
//    {
//      isp_BlockTransfer
//      (
//        &gIspParams.cmd[gIspParams.pollIndex],
//        4 - gIspParams.pollIndex
//      );
//    }

    rval = isp_Exchange(gIspParams.cmd, gIspParams.pollIndex);

    if (rval == gIspParams.pollValue[0])
    {
      /* Success: we are in sync */
      //return
      ISP_LOG_SUCCESS();
      result = FW_SUCCESS;
      break;
    }

    /* Insert one clock pulse and try again */
    isp_InsertClockPulse();

//        PORT_PIN_SET(HWPIN_ISP_SCK);
//    timerTicksDelay(ispClockDelay);
//        PORT_PIN_CLR(HWPIN_ISP_SCK);
//    timerTicksDelay(ispClockDelay);
  }

  if (FW_FAIL == result)
  {
    ISP_LOG_ERROR();
    isp_DetachFromDevice(); //0);
  }

  ISP_LOG_FUNC_END();
  //return FW_FAIL;
  return result;
}

void ISP_LeaveProgmode(void) //stkLeaveProgIsp_t *param)
{
  ISP_LOG_FUNC_START();
  ISP_LOG("Leave Prog Mode\r\n");

  isp_DetachFromDevice(); //param->preDelay);
  vTaskDelay(gIspParams.postDelay); //timerMsDelay(param->postDelay);

  ISP_LOG_FUNC_END();
}

/* ------------------------------------------------------------------------- */

FW_RESULT ISP_ChipErase(void) //stkChipEraseIsp_t *param)
{
//U8   maxDelay = param->eraseDelay;
  FW_RESULT rval = FW_SUCCESS;
//
  ISP_LOG_FUNC_START();
  ISP_LOG("Chip Erase");

//    PORT_PIN_SET(HWPIN_LED_WR);
  (void)isp_Exchange(gIspParams.cmd, ISP_MAX_XFER_SIZE);
  if (0 != gIspParams.pollMethod)
  {
//  if(maxDelay < 10)   /* allow at least 10 ms */
    if (10 > gIspParams.eraseDelay)
//            maxDelay = 10;
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
//	PORT_PIN_CLR(HWPIN_LED_WR);
  return rval;
}

/* ------------------------------------------------------------------------- */

FW_RESULT ISP_ProgramMemory(U8 * pBuffer, U32 size, ISP_MEMORY_t memory) //stkProgramFlashIsp_t *param, U8 isEeprom)
{
//utilWord_t  numBytes;
  FW_RESULT rval = FW_SUCCESS;
  U8 valuePollingMask, rdyPollingMask;
  U32 i;
  U32 time = 0;


//    PORT_PIN_SET(HWPIN_LED_WR);
//    numBytes.bytes[1] = param->numBytes[0];
//    numBytes.bytes[0] = param->numBytes[1];




//#define MODE_MASK                 (1 << 0)
//#define MODE_WORD                 (0 << 0) //Word/Page Mode (0 = word, 1 = page)
//#define MODE_PAGE                 (1 << 0)
//#define MODE_WORD_DELAY           (1 << 1) //Timed delay
//#define MODE_WORD_VALUE           (1 << 2) //Value polling
//#define MODE_WORD_RDY_BSY         (1 << 3) //RDY/BSY polling
//#define MODE_PAGE_DELAY           (1 << 4) //Timed delay
//#define MODE_PAGE_VALUE           (1 << 5) //Value polling
//#define MODE_PAGE_RDY_BSY         (1 << 6) //RDY/BSY polling
//#define MODE_PAGE_WRITE           (1 << 7) //Write page






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

  for (i = 0; (rval == FW_SUCCESS) && (i < size); i++)
  {
//    U8 x;
//    wdt_reset();
    gTxBuffer[0] = gIspParams.cmd[0];
    gTxBuffer[1] = gIspParams.address.byte[1]; // cmdBuffer[1] = stkAddress.bytes[1];
    gTxBuffer[2] = gIspParams.address.byte[0]; // cmdBuffer[2] = stkAddress.bytes[0];
    gTxBuffer[3] = pBuffer[i]; // cmdBuffer[3] = param->data[i];
//    x = gIspParams.cmd[0]; // x = param->cmd[0];
    if (ISP_FLASH == memory) // !isEeprom)
    {
      gTxBuffer[0] &= ~(0x08);
      if (1 == (i & 1))
      {
        gTxBuffer[0] |= 0x08;
        gIspParams.address.value++; // stkIncrementAddress();
      }
    }
    else
    {
      gIspParams.address.value++; // stkIncrementAddress();
    }
//    gTxBuffer[0] = x; // cmdBuffer[0] = x;


////        if(cmdBuffer[3] == 0xff && !(param->mode & 1) && !isEeprom)   /* skip 0xff in word mode */
////            continue;
    (void)isp_Exchange(gTxBuffer, ISP_MAX_XFER_SIZE); // ispBlockTransfer(cmdBuffer, 4);



    if (MODE_PAGE == (gIspParams.mode & MODE_MASK))
    {
      /* Page mode */
      if ( (i < (size - 1)) || !(gIspParams.mode & 0x80) )
      {
        /* Not last byte written */
        continue;
      }

      /* Write program memory page */
      gTxBuffer[0] = gIspParams.cmd[1]; //cmdBuffer[0] = param->cmd[1];
      gTxBuffer[3] = 0;
      (void)isp_Exchange(gTxBuffer, ISP_MAX_XFER_SIZE); //ispBlockTransfer(cmdBuffer, 4);
    }


    /* Poll for ready after each byte (word mode) or page (page mode) */
    if (gIspParams.mode & valuePollingMask)
    {
      /* Value polling */
      U8 d = pBuffer[i];
      if ( (d == gIspParams.pollValue[0]) || (d == gIspParams.pollValue[1]) ) //if (d == param->poll[0] || d == param->poll[1])
      {
        /* Must use timed polling */
        vTaskDelay(gIspParams.cmdExeDelay); // timerMsDelay(param->delay);
      }
      else
      {
        gTxBuffer[0] = gIspParams.cmd[2]; // param->cmd[2];     /* Read flash */
        gTxBuffer[0] &= ~(0x08);
        if (1 == (i & 1))
        {
          gTxBuffer[0] |= (0x08);
        }
//        gTxBuffer[0] = x; // cmdBuffer[0] = x;

//      timerSetupTimeout(param->delay);
        time = xTaskGetTickCount() + gIspParams.cmdExeDelay;
        while (isp_Exchange(gTxBuffer, ISP_MAX_XFER_SIZE) != d)// while(ispBlockTransfer(cmdBuffer, 4) != d)
        {
          //if (timerTimeoutOccurred())
          if (time < xTaskGetTickCount())
          {
            rval = FW_TIMEOUT; // rval = STK_STATUS_CMD_TOUT;
            break;
          }
          vTaskDelay(ISP_POLL_DELAY_MS);
        }
      }
    }
    else if (gIspParams.mode & rdyPollingMask)
    {
      /* rdy/bsy polling */
      rval = isp_WaitUntilReady(gIspParams.cmdExeDelay); // param->delay);
    }
    else
    {
      /* must be timed delay */
      vTaskDelay(gIspParams.cmdExeDelay); // timerMsDelay(param->delay);
    }
  }
//	PORT_PIN_CLR(HWPIN_LED_WR);
  return rval;
}

/* -------------------------------------------------------------------------- */

FW_RESULT ISP_ReadMemory(U8 * pBuffer, U32 size, ISP_MEMORY_t memory)
{
  //utilWord_t  numBytes;
  //U8         *p, cmd0;
  U32        i;

  ISP_LOG_FUNC_START();
  ISP_LOG("Read Memory\r\n");
  ISP_LOG("Address = %08X\r\n", gIspParams.address.value);
  ISP_LOG("Size    = %d\r\n", size);

//  PORT_PIN_SET(HWPIN_LED_RD);
  gTxBuffer[0] = gIspParams.cmd[0];
  gTxBuffer[3] = 0; //cmdBuffer[3] = 0;
//  numBytes.bytes[1] = param->numBytes[0];
//  numBytes.bytes[0] = param->numBytes[1];
//  p = result->data;
//  result->status1 = STK_STATUS_CMD_OK;
//  cmd0 = param->cmd;

//  DBG("Address = %d\r\n", gIspParams.address.value);

  for (i = 0; /*i < numBytes.word;*/ i < size; i++)
  {
    //wdt_reset();
    gTxBuffer[1] = gIspParams.address.byte[1]; // cmdBuffer[1] = stkAddress.bytes[1];
    gTxBuffer[2] = gIspParams.address.byte[0]; // cmdBuffer[2] = stkAddress.bytes[0];

    if (ISP_EEPROM == memory)
    {
      gIspParams.address.value++; // stkIncrementAddress();
    }
    else
    {
      gTxBuffer[0] &= ~(0x08);
      if (1 == (i & 1))
      {
        gTxBuffer[0] |= (0x08);
        gIspParams.address.value++; // stkIncrementAddress();
      }
      //else
      //{
      //  gTxBuffer[0] &= ~(0x08);
      //}
    }
    //cmdBuffer[0] = cmd0;
    *pBuffer++ = isp_Exchange(gTxBuffer, ISP_MAX_XFER_SIZE); // *p++ = ispBlockTransfer(cmdBuffer, 4);
  }
  //*p = STK_STATUS_CMD_OK; /* status2 */
	//PORT_PIN_CLR(HWPIN_LED_RD);

  ISP_LOG_FUNC_END();

  return FW_SUCCESS; // numBytes.word + 2;
}

/* ------------------------------------------------------------------------- */

void ISP_ProgramFLSO(void) // stkProgramFuseIsp_t *param)
{
//    PORT_PIN_SET(HWPIN_LED_WR);
  (void)isp_Exchange(gIspParams.cmd, ISP_MAX_XFER_SIZE); // ispBlockTransfer(param->cmd, 4);
//	PORT_PIN_CLR(HWPIN_LED_WR);
//    return STK_STATUS_CMD_OK;
}

/* ------------------------------------------------------------------------- */

U8 ISP_ReadFLSO(void) //stkReadFuseIsp_t * param)
{
  //U8 rval = 0;

  //PORT_PIN_SET(HWPIN_LED_RD);

  return isp_Exchange(gIspParams.cmd, gIspParams.pollIndex);
  //  if(param->retAddr < 4)
  //      ispBlockTransfer(param->cmd + param->retAddr, 4 - param->retAddr);
	//PORT_PIN_CLR(HWPIN_LED_RD);
  //  return rval;
}

/* ------------------------------------------------------------------------- */

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

/* ------------------------------------------------------------------------- */

void ISP_ResetTarget(void) // stkResetTargetIsp_t *param)
{
//    PORT_PIN_SET(HWPIN_LED_WR);
//
  GPIO_Init(SPI1_RST_PORT,  SPI1_RST_PIN,  GPIO_TYPE_OUT_OD_10MHZ, 1); // PORT_DDR_SET(HWPIN_ISP_RESET);

  GPIO_Lo(SPI1_RST_PORT, SPI1_RST_PIN); // PORT_PIN_CLR(HWPIN_ISP_RESET);
  vTaskDelay(gIspParams.preDelay); // timerMsDelay(param->rstDelay);
//timerTicksDelay(ispClockDelay);

  GPIO_Hi(SPI1_RST_PORT, SPI1_RST_PIN); // PORT_PIN_SET(HWPIN_ISP_RESET);
  vTaskDelay(gIspParams.postDelay); // timerMsDelay(param->postDelay);
//timerTicksDelay(ispClockDelay);

//    PORT_DDR_CLR(HWPIN_ISP_RESET);
//
//    PORT_PIN_CLR(HWPIN_LED_WR);
}

/* ------------------------------------------------------------------------- */
