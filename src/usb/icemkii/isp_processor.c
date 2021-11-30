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

#define ISP_VERSION_HW            (1)
#define ISP_VERSION_MAJOR         (2)
#define ISP_VERSION_MINOR         (10)

#define EVT_SPI_EXCH_COMPLETE     (1 << 1)
#define EVT_SPI_EXCH_TIMEOUT      (100)

#define ISP_MAX_XFER_SIZE         (4)

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
  for (timeout = 0; timeout < 10; timeout++) portNOP();
//        PORT_PIN_CLR(HWPIN_ISP_SCK);
  GPIO_Lo(SPI1_SCK_PORT, SPI1_SCK_PIN);
  //vTaskDelay(1); //    timerTicksDelay(ispClockDelay);
  for (timeout = 0; timeout < 10; timeout++) portNOP();

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

  vTaskDelay(1);

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
  vTaskDelay(1);
  /* We now need to give a positive pulse on RESET since we can't guarantee
   * that SCK was low during power up (according to instructions in Atmel's
   * data sheets).
   */
//  PORT_PIN_SET(HWPIN_ISP_RESET);  /* give a positive RESET pulse */
  GPIO_Hi(SPI1_RST_PORT, SPI1_RST_PIN);
//  vTaskDelay(1); //timerTicksDelay(ispClockDelay);
  for (timeout = 0; timeout < 10; timeout++) portNOP();
//  PORT_PIN_CLR(HWPIN_ISP_RESET);
  GPIO_Lo(SPI1_RST_PORT, SPI1_RST_PIN);



  if (NULL == gEvents)
  {
    /* Create the event group for synchronization */
    gEvents = xEventGroupCreate();
  }

  vTaskDelay(1);

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
  U8 rval = FW_SUCCESS;
  U8 i = 0;

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

    if (rval == gIspParams.pollValue)
    {
      /* success: we are in sync */
      return FW_SUCCESS;
    }

    isp_InsertClockPulse();
    /* Insert one clock pulse and try again */
//        PORT_PIN_SET(HWPIN_ISP_SCK);
//    timerTicksDelay(ispClockDelay);
//        PORT_PIN_CLR(HWPIN_ISP_SCK);
//    timerTicksDelay(ispClockDelay);
  }

  isp_DetachFromDevice(); //0);

  return FW_FAIL;
}

void ISP_LeaveProgmode(void) //stkLeaveProgIsp_t *param)
{
  isp_DetachFromDevice(); //param->preDelay);
  vTaskDelay(gIspParams.postDelay); //timerMsDelay(param->postDelay);
}

/* ------------------------------------------------------------------------- */

FW_RESULT ISP_ChipErase(void) //stkChipEraseIsp_t *param)
{
//U8   maxDelay = param->eraseDelay;
  FW_RESULT rval = FW_SUCCESS;
//
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
//	PORT_PIN_CLR(HWPIN_LED_WR);
  return rval;
}

/* ------------------------------------------------------------------------- */

//U8   ispProgramMemory(stkProgramFlashIsp_t *param, U8 isEeprom)
//{
//utilWord_t  numBytes;
//U8       rval = STK_STATUS_CMD_OK;
//U8       valuePollingMask, rdyPollingMask;
//uint        i;
//
//    PORT_PIN_SET(HWPIN_LED_WR);
//    numBytes.bytes[1] = param->numBytes[0];
//    numBytes.bytes[0] = param->numBytes[1];
//    if(param->mode & 1){    /* page mode */
//        valuePollingMask = 0x20;
//        rdyPollingMask = 0x40;
//    }else{                  /* word mode */
//        valuePollingMask = 4;
//        rdyPollingMask = 8;
//    }
//    for(i = 0; rval == STK_STATUS_CMD_OK && i < numBytes.word; i++){
//        U8 x;
//        wdt_reset();
//        cmdBuffer[1] = stkAddress.bytes[1];
//        cmdBuffer[2] = stkAddress.bytes[0];
//        cmdBuffer[3] = param->data[i];
//        x = param->cmd[0];
//        if(!isEeprom){
//            x &= ~0x08;
//            if((U8)i & 1){
//                x |= 0x08;
//                stkIncrementAddress();
//            }
//        }else{
//            stkIncrementAddress();
//        }
//        cmdBuffer[0] = x;
////        if(cmdBuffer[3] == 0xff && !(param->mode & 1) && !isEeprom)   /* skip 0xff in word mode */
////            continue;
//        ispBlockTransfer(cmdBuffer, 4);
//        if(param->mode & 1){            /* is page mode */
//            if(i < numBytes.word - 1 || !(param->mode & 0x80))
//                continue;               /* not last byte written */
//            cmdBuffer[0] = param->cmd[1];     /* write program memory page */
//            ispBlockTransfer(cmdBuffer, 4);
//        }
//        /* poll for ready after each byte (word mode) or page (page mode) */
//        if(param->mode & valuePollingMask){ /* value polling */
//            U8 d = param->data[i];
//            if(d == param->poll[0] || d == param->poll[1]){ /* must use timed polling */
//                timerMsDelay(param->delay);
//            }else{
//                U8 x = param->cmd[2];     /* read flash */
//                x &= ~0x08;
//                if((U8)i & 1){
//                    x |= 0x08;
//                }
//                cmdBuffer[0] = x;
//                timerSetupTimeout(param->delay);
//                while(ispBlockTransfer(cmdBuffer, 4) != d){
//                    if(timerTimeoutOccurred()){
//                        rval = STK_STATUS_CMD_TOUT;
//                        break;
//                    }
//                }
//            }
//        }else if(param->mode & rdyPollingMask){ /* rdy/bsy polling */
//            rval = waitUntilReady(param->delay);
//        }else{                          /* must be timed delay */
//            timerMsDelay(param->delay);
//        }
//    }
//	PORT_PIN_CLR(HWPIN_LED_WR);
//    return rval;
//}

/* -------------------------------------------------------------------------- */

FW_RESULT ISP_ReadMemory(U8 * pBuffer, U32 size) //, U8 isEeprom)
{
  //utilWord_t  numBytes;
  //U8         *p, cmd0;
  U32        i;

//  PORT_PIN_SET(HWPIN_LED_RD);
  gIspParams.cmd[3] = 0; //cmdBuffer[3] = 0;
//  numBytes.bytes[1] = param->numBytes[0];
//  numBytes.bytes[0] = param->numBytes[1];
//  p = result->data;
//  result->status1 = STK_STATUS_CMD_OK;
//  cmd0 = param->cmd;

  DBG("Address = %d\r\n", gIspParams.address.value);

  for (i = 0; /*i < numBytes.word;*/ i < size; i++)
  {
    //wdt_reset();
    gIspParams.cmd[1] = gIspParams.address.byte[1]; // cmdBuffer[1] = stkAddress.bytes[1];
    gIspParams.cmd[2] = gIspParams.address.byte[0]; // cmdBuffer[2] = stkAddress.bytes[0];

    //if (!isEeprom)
    //{
    //  if ((U8)i & 1)
    //  {
    //    cmd0 |= 0x08;
    //    stkIncrementAddress();
    //  }
    //  else
    //  {
    //    cmd0 &= ~0x08;
    //  }
    //}
    //else
    //{
    //  stkIncrementAddress();
    //}
    gIspParams.address.value++;

    //cmdBuffer[0] = cmd0;
    *pBuffer++ = isp_Exchange(gIspParams.cmd, ISP_MAX_XFER_SIZE); // *p++ = ispBlockTransfer(cmdBuffer, 4);
  }
  //*p = STK_STATUS_CMD_OK; /* status2 */
	//PORT_PIN_CLR(HWPIN_LED_RD);
  return FW_SUCCESS; // numBytes.word + 2;
}

/* ------------------------------------------------------------------------- */

//U8   ispProgramFuse(stkProgramFuseIsp_t *param)
//{
//    PORT_PIN_SET(HWPIN_LED_WR);
//    ispBlockTransfer(param->cmd, 4);
//	PORT_PIN_CLR(HWPIN_LED_WR);
//    return STK_STATUS_CMD_OK;
//}

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

//void    ispResetTarget(stkResetTargetIsp_t *param)
//{
//    PORT_PIN_SET(HWPIN_LED_WR);
//
//    PORT_DDR_SET(HWPIN_ISP_RESET);
//
//    PORT_PIN_CLR(HWPIN_ISP_RESET);
//    timerMsDelay(param->rstDelay);
//    timerTicksDelay(ispClockDelay);
//
//    PORT_PIN_SET(HWPIN_ISP_RESET);
//    timerMsDelay(param->postDelay);
//    timerTicksDelay(ispClockDelay);
//
//    PORT_DDR_CLR(HWPIN_ISP_RESET);
//
//    PORT_PIN_CLR(HWPIN_LED_WR);
//}

/* ------------------------------------------------------------------------- */
