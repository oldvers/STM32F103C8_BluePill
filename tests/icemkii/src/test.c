#include <stdio.h>
#include <string.h>

#include "types.h"
#include "debug.h"
#include "usb.h"
#include "fifo.h"

//#include "usb_definitions.h"
//#include "usb_control.h"
#include "icemkii_message.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

typedef FW_BOOLEAN (* TestFunction_t)(void);

/* -------------------------------------------------------------------------- */

/* --- Incorrect CRC -------------------
   MESSAGE_START   - 27 - 0x1B
   SEQUENCE_NUMBER      - 0x0005
   MESSAGE_SIZE         - 0x00000004
   TOKEN           - 14 - 0x0E
   MESSAGE_BODY         - 0x87654321
   CRC                  - 0xXXXX */

static U8 msgs[14 * 2] =
{
  /* Message with incorrect CRC */
  0x1B, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x0E, 0x21, 0x43, 0x65, 0x87, 0xFF, 0xFF,
  /* Correct message */
  0x1B, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x0E, 0x21, 0x43, 0x65, 0x87, 0xC2, 0xC3,
};

#define ICEMKII_RX_READY     (1 << 0)
#define ICEMKII_RX_WAITING   (1 << 1)
//#define ICEMKII_TX_READY         (1)
//#define ICEMKII_RX_READY         (1)

//extern FIFO_p              gRxFifo;
//extern FIFO_p              gTxFifo;
extern EventGroupHandle_t  hEvtGroup;

//extern void icemkii_ProcessTx(void);
//extern void icemkii_ProcessRx(void);

static U8            gIceMkIiMsg[24] = {0};
static ICEMKII_MSG_p pIceMkIiMsg     = NULL;
static U8            gIceMkIiBuf[64] = {0};

/* --- Mocks ---------------------------------------------------------------- */

U32 USBD_EndPointRdWsCb(U32 aNumber, USB_CbByte pPutCb, U32 aSize)
{
    (void)aNumber;

    if (sizeof(msgs) > aSize)
    {
      return 0; /* The Buffer size is too small for received bytes */
    }

    for (U32 n = 0; n < sizeof(msgs); n++)
    {
      pPutCb(&msgs[n]);
    }

    return (sizeof(msgs));
}

/* --- Tests ---------------------------------------------------------------- */

static FW_BOOLEAN Test_InitSuccess(void)
{
    FW_BOOLEAN res = FW_TRUE;
    FW_RESULT sts = FW_ERROR;

    DBG("*** ICEMKII Test Init *************************************\r\n");

    /* Call the initialization function */
    pIceMkIiMsg = ICEMKII_MSG_Init
                  (
                      gIceMkIiMsg,
                      sizeof(gIceMkIiMsg),
                      gIceMkIiBuf,
                      sizeof(gIceMkIiBuf)
                  );

    res = (FW_BOOLEAN)(NULL != pIceMkIiMsg);
    if (FW_FALSE == res) return res;

    sts = ICEMKII_MSG_SetBuffer(pIceMkIiMsg, gIceMkIiBuf, sizeof(gIceMkIiBuf));
    res = (FW_BOOLEAN)(FW_SUCCESS == sts);

    return res;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_PutSuccess(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 byte = 0, test = 0, size = 0;
    U8 * testPacket = NULL;

    DBG("*** ICEMKII Success Put Byte Test *************************\r\n");

    testPacket = &msgs[14];
    size = 14;

    /* Init the ICEMKII message */
    pIceMkIiMsg = ICEMKII_MSG_Init
                  (
                      gIceMkIiMsg,
                      sizeof(gIceMkIiMsg),
                      gIceMkIiBuf,
                      sizeof(gIceMkIiBuf)
                  );
    result = (FW_BOOLEAN)(NULL != pIceMkIiMsg);
    if (FW_FALSE == result) return result;

    for (test = 0; test < 1; test++)
    {
        /* Setup/Reset the ICEMKII message */
        status = ICEMKII_MSG_SetBuffer
                 (
                     pIceMkIiMsg,
                     gIceMkIiBuf,
                     sizeof(gIceMkIiBuf)
                 );
        result = (FW_BOOLEAN)(FW_SUCCESS == status);
        if (FW_FALSE == result) break;

        /* Fill the ICEMKII packet */
        for (byte = 0; byte < 14; byte++)
        {
            status = ICEMKII_MSG_PutByte(pIceMkIiMsg, testPacket[byte]);
            if ( (size - 1) == byte )
            {
                result &= (FW_BOOLEAN)(FW_COMPLETE == status);
            }
            else
            {
                result &= (FW_BOOLEAN)(FW_INPROGRESS == status);
            }
            if (FW_FALSE == result) break;
        }
        if (FW_FALSE == result) break;

        result &= (FW_BOOLEAN)(gIceMkIiBuf[0] == testPacket[8]);
        result &= (FW_BOOLEAN)(gIceMkIiBuf[1] == testPacket[9]);
        result &= (FW_BOOLEAN)(gIceMkIiBuf[3] == testPacket[11]);

        size = ICEMKII_MSG_GetDataSize(pIceMkIiMsg);
        result &= (FW_BOOLEAN)(4 == size);
        size = ICEMKII_MSG_GetPacketSize(pIceMkIiMsg);
        result &= (FW_BOOLEAN)(14 == size);

        if (FW_FALSE == result) break;
    }

    return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_GetSuccess(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 byte = 0, size = 0;
    U8 testPacket[14] = {0};
    U8 testMessage[4] = {0};
    U8 value = 0;

    DBG("*** ICEMKII Success Get Byte Test *************************\r\n");

    //testPacket = gTestEastPacketSuccess;
    size = sizeof(testPacket);

    /* Init the ICEMKII message */
    pIceMkIiMsg = ICEMKII_MSG_Init
                  (
                      gIceMkIiMsg,
                      sizeof(gIceMkIiMsg),
                      gIceMkIiBuf,
                      sizeof(gIceMkIiBuf)
                  );
    result = (FW_BOOLEAN)(NULL != pIceMkIiMsg);
    if (FW_FALSE == result) return result;

    /* Fill the packet buffer */
    for (byte = 0; byte < 4; byte++)
    {
        testMessage[byte] = msgs[14 + 8 + byte];
    }
    memset(testPacket, 0, 14);
    ICEMKII_MSG_SetSequenceNumber(pIceMkIiMsg, 5);

    /* Setup/Reset the ICEMKII message */
    status = ICEMKII_MSG_SetBuffer
             (
                 pIceMkIiMsg,
                 testMessage,
                 sizeof(testMessage)
             );
    result = (FW_BOOLEAN)(FW_SUCCESS == status);
    if (FW_FALSE == result) return result;

    /* Get the EAST packet */
    for (byte = 0; byte < size; byte++)
    {
        status = ICEMKII_MSG_GetByte(pIceMkIiMsg, &value);
        if ( (size - 1) == byte )
        {
            result &= (FW_BOOLEAN)(FW_COMPLETE == status);
        }
        else
        {
            result &= (FW_BOOLEAN)(FW_INPROGRESS == status);
        }
        if (FW_FALSE == result) break;

        result &= (FW_BOOLEAN)(value == msgs[14 + byte]);

        size = ICEMKII_MSG_GetDataSize(pIceMkIiMsg);
        result &= (FW_BOOLEAN)(4 == size);
        size = ICEMKII_MSG_GetPacketSize(pIceMkIiMsg);
        result &= (FW_BOOLEAN)((14 - 1 - byte) == size);

        if (FW_FALSE == result) break;
    }

    return result;
}

/* -------------------------------------------------------------------------- */

//static FW_BOOLEAN Test_RxIsReady(void)
//{
//  EventBits_t uxReturned;
//  FW_BOOLEAN result = FW_FALSE;
//  U8 data = 0;
//
//  DBG("*** ICEMKII Test USB Rx Is Ready **************************\r\n");
//
//  DBG(" - Put one bunch of data from USB EP to FIFO\r\n");
//  icemkii_ProcessRx();
//
//  DBG(" - Check if flag in Event Group is set\r\n");
//  uxReturned = xEventGroupWaitBits
//               (
//                 hEvtGroup,
//                 ICEMKII_RX_READY,
//                 pdFALSE,
//                 pdFALSE,
//                 0
//               );
//
//  if (0 != (uxReturned & ICEMKII_RX_READY))
//  {
//    result = FW_TRUE;
//
//    DBG("   - FIFO Count = %d\r\n", FIFO_Count(gRxFifo));
//
//    DBG("   - Clearing FIFO...\r\n");
//    while (FW_EMPTY != FIFO_Get(gRxFifo, &data)) {};
//    DBG("   - FIFO Count = %d\r\n", FIFO_Count(gRxFifo));
//
//    (void)xEventGroupClearBits(hEvtGroup, ICEMKII_RX_READY);
//  }
//
//  return result;
//}

/* -------------------------------------------------------------------------- */

//static FW_BOOLEAN Test_NoSpace(void)
//{
//  EventBits_t uxReturned;
//  FW_BOOLEAN result = FW_FALSE;
//  U32 bunch = 0, bunchExpctd, space, spaceExpctd;
//  U8 data = 0;
//
//  DBG("*** ICEMKII Test USB No Space *****************************\r\n");
//
//  bunchExpctd = FIFO_Size(gRxFifo) / sizeof(msgs);
//  spaceExpctd = bunchExpctd * sizeof(msgs);
//
//  while (FW_TRUE)
//  {
//    DBG(" - Put %d bunch of data from USB EP to FIFO\r\n", bunch);
//    icemkii_ProcessRx();
//
//    DBG(" - Check if flag in Event Group is set\r\n");
//    uxReturned = xEventGroupWaitBits
//                 (
//                   hEvtGroup,
//                   ICEMKII_RX_WAITING,
//                   pdFALSE,
//                   pdFALSE,
//                   0
//                 );
//
//    if (0 != (uxReturned & ICEMKII_RX_WAITING))
//    {
//      space = FIFO_Count(gRxFifo);
//      if ((space == spaceExpctd) && (bunch == bunchExpctd))
//      {
//          result = FW_TRUE;
//      }
//
//      DBG("   - FIFO Size = %d Expected = %d\r\n", space, spaceExpctd);
//      DBG("   - Bunches = %d Expected = %d\r\n", bunch, bunchExpctd);
//
//      DBG("   - Clearing FIFO...\r\n");
//      while (FW_EMPTY != FIFO_Get(gRxFifo, &data)) {};
//      DBG("   - FIFO Size = %d\r\n", FIFO_Count(gRxFifo));
//
//      (void)xEventGroupClearBits(hEvtGroup, ICEMKII_RX_WAITING);
//      (void)xEventGroupClearBits(hEvtGroup, ICEMKII_RX_READY);
//
//      break;
//    }
//
//    bunch++;
//  }
//
//  return result;
//}

/* --- Test Start Up Function (mandatory, called before RTOS starts) -------- */

void vTestStartUpFunction(void)
{
  DBG_ClearScreen();
  DBG("*** Start Up Test ***\r\n");
}

/* --- Test Prepare Function (mandatory, called before RTOS starts) --------- */

void vTestPrepareFunction (void)
{
  DBG("*** Prepare Test ***\r\n");
}

/* --- Helper Task Main Function (mandatory) -------------------------------- */

void vTestHelpTaskFunction(void * pvParameters)
{
  //DBG("LED Task Started\r\n");

  while (FW_TRUE)
  {
      //DBG("LED Hi\r\n");
      vTaskDelay(50);
      //DBG("LED Lo\r\n");
      vTaskDelay(50);
  }
  //vTaskDelete(NULL);
}

/* --- Test Cases List (mandatory) ------------------------------------------ */

const TestFunction_t gTests[] =
{
    Test_InitSuccess,
    Test_PutSuccess,
    Test_GetSuccess,
//    Test_RxIsReady,
//    Test_NoSpace,
};

U32 uiTestsGetCount(void)
{
    return (sizeof(gTests) / sizeof(TestFunction_t));
}

/* --- Error callback function (mandatory) ---------------------------------- */

void on_error(S32 parameter)
{
  while (FW_TRUE) {};
}

/* -------------------------------------------------------------------------- */
