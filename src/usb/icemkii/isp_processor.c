//#include "hardware.h"
//#include <avr/io.h>
//#include <avr/wdt.h>
//#include <avr/interrupt.h>
//#include "utils.h"

#include "FreeRTOS.h"
#include "task.h"

#include "isp_processor.h"

//#include "timer.h"
//#include "oddebug.h"

/* -------------------------------------------------------------------------- */

#define ISP_VERSION_HW            (1)
#define ISP_VERSION_MAJOR         (2)
#define ISP_VERSION_MINOR         (10)


ISP_PARAMETERS_t gIspParams =
{
  .hwVersion      = ISP_VERSION_HW,
  .swVersionMajor = ISP_VERSION_MAJOR,
  .swVersionMinor = ISP_VERSION_MINOR,
  .sckDuration    = 1,
};


//static U8 ispClockDelay;
//static U8 cmdBuffer[4];

/* ------------------------------------------------------------------------- */
/* We disable interrupts while transfer a byte. This ensures that we execute
 * at nominal speed, in spite of aggressive USB polling.
 */
static U8 isp_BlockTransfer(U8 * pBlock, U8 size)
{
  U8 /*cnt,*/ shift = 0/*, port, delay = ispClockDelay*/;

///* minimum clock pulse width:
// * 5 + 4 * delay clock cycles           -> Tmin = 750 ns
// * total clock period: 12 + 8 * delay   -> fmax = 600 kHz
// */
///*    DBG2(0x40, block, len); */
//    cli();
//    port = PORT_OUT(HWPIN_ISP_MOSI) & ~(1 << PORT_BIT(HWPIN_ISP_MOSI));
//    do{
//        cnt = 8;
//        shift = *block++;
//        do{
//            if(shift & 0x80){
//                port |= (1 << PORT_BIT(HWPIN_ISP_MOSI));
//            }
//            PORT_OUT(HWPIN_ISP_MOSI) = port;
//            sei();
//            timerTicksDelay(delay);
//            cli();
//            PORT_PIN_SET(HWPIN_ISP_SCK);
//            shift <<= 1;
//            port &= ~(1 << PORT_BIT(HWPIN_ISP_MOSI));
//            if(PORT_PIN_VALUE(HWPIN_ISP_MISO)) /* driver is inverting */
//                shift |= 1;
//            sei();
//            timerTicksDelay(delay);
//            cli();
//            PORT_PIN_CLR(HWPIN_ISP_SCK);
//        }while(--cnt);
//    }while(--len);
//    sei();
///*    DBG2(0x41, &shift, 1); */
  return shift;
}

/* ------------------------------------------------------------------------- */

static void isp_InsertClockPulse(void)
{
  //
}

//static U8    deviceIsBusy(void)
//{
//    cmdBuffer[0] = 0xf0;
//    cmdBuffer[1] = 0;
//    return ispBlockTransfer(cmdBuffer, 4) & 1;
//}
//
//static U8    waitUntilReady(U8 msTimeout)
//{
//    timerSetupTimeout(msTimeout);
//    while(deviceIsBusy()){
//        if(timerTimeoutOccurred())
//            return STK_STATUS_RDY_BSY_TOUT;
//    }
//    return STK_STATUS_CMD_OK;
//}

/* ------------------------------------------------------------------------- */

static void isp_AttachToDevice(void) //U8 stk500Delay, U8 stabDelay)
{
//    PORT_DDR_SET(HWPIN_ISP_SCK);
//    PORT_DDR_SET(HWPIN_ISP_MOSI);
//    PORT_DDR_SET(HWPIN_ISP_RESET);
//
//	//PORT_PIN_SET(HWPIN_LED);
//    //if(!PORT_PIN_VALUE(HWPIN_JUMPER)){      /* Jumper is set -> request clock below 8 kHz */
//    //    ispClockDelay = (U8)(70/TIMER_TICK_US);   /* 140 us -> 7.14 kHz clock rate */
//    //}else
//    if(stk500Delay == 0){ /* 1.8 MHz nominal */
//        ispClockDelay = 0;
//    }else if(stk500Delay == 1){ /* 460 kHz nominal */
//        ispClockDelay = 1;
//    }else if(stk500Delay == 2){ /* 115 kHz nominal */
//        ispClockDelay = 2;
//    }else if(stk500Delay == 3){ /* 58 kHz nominal */
//        ispClockDelay = 3;
//    }else{
//        ispClockDelay = 1 + stk500Delay/4 + stk500Delay/16;
//    }
//    /* setup initial condition: SCK, MOSI = 0 */
//    PORT_OUT(HWPIN_ISP_SCK) &= ~((1 << PORT_BIT(HWPIN_ISP_SCK)) | (1 << PORT_BIT(HWPIN_ISP_MOSI)));
//    PORT_PIN_CLR(HWPIN_ISP_RESET);  /* set RESET */
//    //PORT_DDR_CLR(HWPIN_ISP_DRIVER); /* make input: use internal pullup to control driver */
//    //PORT_PIN_SET(HWPIN_ISP_DRIVER); /* attach to device: */
////    TCCR2 |= (1 << COM20);  /* set toggle on compare match mode -> activate clock */
//    timerMsDelay(stabDelay);
//    timerTicksDelay(ispClockDelay);    /* stabDelay may have been 0 */
//    /* We now need to give a positive pulse on RESET since we can't guarantee
//     * that SCK was low during power up (according to instructions in Atmel's
//     * data sheets).
//     */
//    PORT_PIN_SET(HWPIN_ISP_RESET);  /* give a positive RESET pulse */
//    timerTicksDelay(ispClockDelay);
//    PORT_PIN_CLR(HWPIN_ISP_RESET);
}

static void isp_DetachFromDevice(void) //U8 removeResetDelay)
{
//    PORT_OUT(HWPIN_ISP_SCK) &= ~((1 << PORT_BIT(HWPIN_ISP_SCK)) | (1 << PORT_BIT(HWPIN_ISP_MOSI)));
//    PORT_PIN_CLR(HWPIN_ISP_RESET);
//    timerMsDelay(removeResetDelay);
////    TCCR2 &= ~(1 << COM20);  /* clear toggle on compare match mode */
//    //PORT_PIN_CLR(HWPIN_ISP_DRIVER); /* detach from device */
//    //PORT_DDR_SET(HWPIN_ISP_DRIVER); /* set pin level to low-Z 0 */
//    PORT_PIN_SET(HWPIN_ISP_RESET);
//    //PORT_PIN_CLR(HWPIN_LED);
//
//    PORT_DDR_CLR(HWPIN_ISP_SCK);
//    PORT_DDR_CLR(HWPIN_ISP_MOSI);
//    PORT_DDR_CLR(HWPIN_ISP_RESET);
//
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
    rval = isp_BlockTransfer(gIspParams.cmd, gIspParams.pollIndex);
    if (gIspParams.pollIndex < 4)
    {
      isp_BlockTransfer
      (
        &gIspParams.cmd[gIspParams.pollIndex],
        4 - gIspParams.pollIndex
      );
    }
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

//void    ispLeaveProgmode(stkLeaveProgIsp_t *param)
//{
//    ispDetachFromDevice(param->preDelay);
//    timerMsDelay(param->postDelay);
//}

/* ------------------------------------------------------------------------- */

//U8   ispChipErase(stkChipEraseIsp_t *param)
//{
//U8   maxDelay = param->eraseDelay;
//U8   rval = STK_STATUS_CMD_OK;
//
//    PORT_PIN_SET(HWPIN_LED_WR);
//	ispBlockTransfer(param->cmd, 4);
//    if(param->pollMethod != 0){
//        if(maxDelay < 10)   /* allow at least 10 ms */
//            maxDelay = 10;
//        rval = waitUntilReady(maxDelay);
//    }else{
//        timerMsDelay(maxDelay);
//    }
//	PORT_PIN_CLR(HWPIN_LED_WR);
//    return rval;
//}

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

/* ------------------------------------------------------------------------- */

//uint    ispReadMemory(stkReadFlashIsp_t *param, stkReadFlashIspResult_t *result, U8 isEeprom)
//{
//utilWord_t  numBytes;
//U8       *p, cmd0;
//uint        i;
//
//    PORT_PIN_SET(HWPIN_LED_RD);
//    cmdBuffer[3] = 0;
//    numBytes.bytes[1] = param->numBytes[0];
//    numBytes.bytes[0] = param->numBytes[1];
//    p = result->data;
//    result->status1 = STK_STATUS_CMD_OK;
//    cmd0 = param->cmd;
//    for(i = 0; i < numBytes.word; i++){
//        wdt_reset();
//        cmdBuffer[1] = stkAddress.bytes[1];
//        cmdBuffer[2] = stkAddress.bytes[0];
//        if(!isEeprom){
//            if((U8)i & 1){
//                cmd0 |= 0x08;
//                stkIncrementAddress();
//            }else{
//                cmd0 &= ~0x08;
//            }
//        }else{
//            stkIncrementAddress();
//        }
//        cmdBuffer[0] = cmd0;
//        *p++ = ispBlockTransfer(cmdBuffer, 4);
//    }
//    *p = STK_STATUS_CMD_OK; /* status2 */
//	PORT_PIN_CLR(HWPIN_LED_RD);
//    return numBytes.word + 2;
//}

/* ------------------------------------------------------------------------- */

//U8   ispProgramFuse(stkProgramFuseIsp_t *param)
//{
//    PORT_PIN_SET(HWPIN_LED_WR);
//    ispBlockTransfer(param->cmd, 4);
//	PORT_PIN_CLR(HWPIN_LED_WR);
//    return STK_STATUS_CMD_OK;
//}

/* ------------------------------------------------------------------------- */

//U8   ispReadFuse(stkReadFuseIsp_t *param)
//{
//U8   rval;
//    PORT_PIN_SET(HWPIN_LED_RD);
//    rval = ispBlockTransfer(param->cmd, param->retAddr);
//    if(param->retAddr < 4)
//        ispBlockTransfer(param->cmd + param->retAddr, 4 - param->retAddr);
//	PORT_PIN_CLR(HWPIN_LED_RD);
//    return rval;
//}

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
