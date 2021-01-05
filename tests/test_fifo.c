#include "stm32f1xx.h"
#include "types.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "fifo.h"

#include "usb.h"
#include "usb_defs.h"
#include "usb_core.h"
#include "usb_icemkii.h"
#include "icemkii_message.h"

// --- Incorrect CRC -------------------
// MESSAGE_START   - 27 - 0x1B
// SEQUENCE_NUMBER      - 0x0005
// MESSAGE_SIZE         - 0x00000004
// TOKEN           - 14 - 0x0E
// MESSAGE_BODY         - 0x87654321
// CRC                  - 0xXXXX

//static U8 msgs[14 * 2] =
//{
//  /* Message with incorrect CRC */
//  0x1B, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00,
//  0x0E, 0x21, 0x43, 0x65, 0x87, 0xFF, 0xFF,
//  /* Correct message */
//  0x1B, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00,
//  0x0E, 0x21, 0x43, 0x65, 0x87, 0xC2, 0xC3,
//};

//-----------------------------------------------------------------------------

typedef void (* TestFunction_t)(void);

static FIFO_p pFIFO          = NULL;
static U8     fifoBuffer[25] = {0};
static U32    gPass          = 0;
static U32    gFail          = 0;
static U32    gTested        = 0;

//#define ICEMKII_RX_READY     (1 << 0)
//#define ICEMKII_RX_WAITING   (1 << 1)
//#define ICEMKII_TX_READY         (1)
//#define ICEMKII_RX_READY         (1)

//extern FIFO_t                gRxFifo;
//extern FIFO_t                gTxFifo;
//extern EventGroupHandle_t    hEvtGroup;
//
//extern void icemkii_ProcessTx(void);
//extern void icemkii_ProcessRx(void);

//--- Mocks -------------------------------------------------------------------

void USB_IRQHandler(void)
{
  //
}

//void USB_SetCb_Ep(U32 aNumber, USB_CbEp pCbEp)
//{
//    (void)aNumber;
//    (void)pCbEp;
//}
//
//U32 USB_EpReadToFifo(U32 aNumber, USB_CbEpPut pPutCb, U32 aSize)
//{
//    (void)aNumber;
//
//    if (sizeof(msgs) > aSize)
//    {
//      return 0; /* The Buffer size is too small for received bytes */
//    }
//
//    for (U32 n = 0; n < sizeof(msgs); n++)
//    {
//      pPutCb(&msgs[n]);
//    }
//
//    return (sizeof(msgs));
//}

//-----------------------------------------------------------------------------

void vLEDTask(void * pvParameters)
{
    //LOG("LED Task Started\r\n");

    while(FW_TRUE)
    {
        //LOG("LED Hi\r\n");
        vTaskDelay(50);
        //LOG("LED Lo\r\n");
        vTaskDelay(50);
    }
    //vTaskDelete(NULL);
}

//-----------------------------------------------------------------------------

//static U8 msgBuffer[100];
//static ICEMKII_MESSAGE msg = { 0 };

static void vLog_Result(FW_BOOLEAN result)
{
    if (FW_TRUE == result)
    {
        LOG("-----> PASS\r\n");
        gPass++;
    }
    else
    {
        LOG("-----> FAIL\r\n");
        gFail++;
    }
    gTested++;
}

//-----------------------------------------------------------------------------

static void vTest_FiFo(void)
{
    FW_BOOLEAN result = FW_FALSE;
    FW_RESULT status = FW_ERROR;
    U32 i = 0;
    U8 data = 0;

    LOG("--- FIFO Test ---------------------------\r\n");

    LOG("- FIFO Init\r\n");
    pFIFO = FIFO_Init(fifoBuffer, sizeof(fifoBuffer));
    result = (FW_BOOLEAN)(NULL != pFIFO);
    vLog_Result(result);
    LOG("FIFO Capacity = %d\r\n", FIFO_Size(pFIFO));


    result = FW_TRUE;
    for (i = 0; i < 8; i++)
    {
        data = 0x33;
        status = FIFO_Put(pFIFO, &data);
        result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    }
    LOG("--- FIFO Put 8 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));
    vLog_Result(result);


    result = FW_TRUE;
    for (i = 0; i < 5; i++)
    {
        status = FIFO_Get(pFIFO, &data);
        result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    }
    LOG("--- FIFO Get 5 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(3 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(5 == FIFO_Free(pFIFO));
    vLog_Result(result);


    result = FW_TRUE;
    for (i = 0; i < 10; i++)
    {
        data = 0x44;
        status = FIFO_Put(pFIFO, &data);
        if (5 > i)
        {
            result &= (FW_BOOLEAN)(FW_SUCCESS == status);
        }
        else
        {
            result &= (FW_BOOLEAN)(FW_FULL == status);
        }
    }
    LOG("--- FIFO Put 10 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));
    vLog_Result(result);


    result = FW_TRUE;
    for (i = 0; i < 3; i++)
    {
        status = FIFO_Get(pFIFO, &data);
        result &= (FW_BOOLEAN)(FW_SUCCESS == status);
    }
    LOG("--- FIFO Get 3 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(5 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(3 == FIFO_Free(pFIFO));
    vLog_Result(result);


    result = FW_TRUE;
    for (i = 0; i < 13; i++)
    {
        data = 0x55;
        status = FIFO_Put(pFIFO, &data);
        if (3 > i)
        {
            result &= (FW_BOOLEAN)(FW_SUCCESS == status);
        }
        else
        {
            result &= (FW_BOOLEAN)(FW_FULL == status);
        }
    }
    LOG("--- FIFO Put 13 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));
    vLog_Result(result);


    result = FW_TRUE;
    for (i = 0; i < 20; i++)
    {
        status = FIFO_Get(pFIFO, &data);
        if (8 > i)
        {
            result &= (FW_BOOLEAN)(FW_SUCCESS == status);
        }
        else
        {
            result &= (FW_BOOLEAN)(FW_EMPTY == status);
        }
    }
    LOG("--- FIFO Get 20 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Free(pFIFO));
    vLog_Result(result);


    result = FW_TRUE;
    for (i = 0; i < 13; i++)
    {
        data = 0x66;
        status = FIFO_Put(pFIFO, &data);
        if (8 > i)
        {
            result &= (FW_BOOLEAN)(FW_SUCCESS == status);
        }
        else
        {
            result &= (FW_BOOLEAN)(FW_FULL == status);
        }
    }
    LOG("--- FIFO Put 13 ---\r\n");
    LOG("FIFO Count = %d\r\n", FIFO_Count(pFIFO));
    LOG("FIFO Free  = %d\r\n", FIFO_Free(pFIFO));
    LOG("FIFO Sum   = %d\r\n", FIFO_Free(pFIFO) + FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(8 == FIFO_Count(pFIFO));
    result &= (FW_BOOLEAN)(0 == FIFO_Free(pFIFO));
    vLog_Result(result);
}

//-----------------------------------------------------------------------------

static void vTest_RxIsReady(void)
{
//    EventBits_t uxReturned;
//    FW_BOOLEAN result = FW_FALSE;
//    U8 data = 0;

//    LOG("--- ICEMKII Test USB Rx Is Ready ------------------------------\r\n");

//    LOG(" - Put one bunch of data from USB EP to FIFO\r\n");
//    icemkii_ProcessRx();
//
//    LOG(" - Check if flag in Event Group is set\r\n");
//    uxReturned = xEventGroupWaitBits
//                 (
//                   hEvtGroup,
//                   ICEMKII_RX_READY,
//                   pdFALSE,
//                   pdFALSE,
//                   0
//                 );
//
//    if (0 != (uxReturned & ICEMKII_RX_READY))
//    {
//        result = FW_TRUE;
//
//        LOG("   - FIFO Size = %d\r\n", FIFO_Size(&gRxFifo));
//
//        LOG("   - Clearing FIFO...\r\n");
//        while (FW_EMPTY != FIFO_Get(&gRxFifo, &data)) {};
//        LOG("   - FIFO Size = %d\r\n", FIFO_Size(&gRxFifo));
//
//        (void)xEventGroupClearBits(hEvtGroup, ICEMKII_RX_READY);
//    }

//    vLog_Result(result);
}

//-----------------------------------------------------------------------------

static void vTest_NoSpace(void)
{
//    EventBits_t uxReturned;
//    FW_BOOLEAN result = FW_FALSE;
//    U32 bunch = 0, bunchExpctd, space, spaceExpctd;
//    U8 data = 0;

//    LOG("--- ICEMKII Test USB No Space ---------------------------------\r\n");

//    bunchExpctd = FIFO_Capacity(&gRxFifo) / sizeof(msgs);
//    spaceExpctd = bunchExpctd * sizeof(msgs);
//
//    while (FW_TRUE)
//    {
//        LOG(" - Put %d bunch of data from USB EP to FIFO\r\n", bunch);
//        icemkii_ProcessRx();
//
//        LOG(" - Check if flag in Event Group is set\r\n");
//        uxReturned = xEventGroupWaitBits
//                     (
//                       hEvtGroup,
//                       ICEMKII_RX_WAITING,
//                       pdFALSE,
//                       pdFALSE,
//                       0
//                     );
//
//        if (0 != (uxReturned & ICEMKII_RX_WAITING))
//        {
//            space = FIFO_Size(&gRxFifo);
//            if ((space == spaceExpctd) && (bunch == bunchExpctd))
//            {
//                result = FW_TRUE;
//            }
//
//            LOG("   - FIFO Size = %d Expected = %d\r\n", space, spaceExpctd);
//            LOG("   - Bunches = %d Expected = %d\r\n", bunch, bunchExpctd);
//
//            LOG("   - Clearing FIFO...\r\n");
//            while (FW_EMPTY != FIFO_Get(&gRxFifo, &data)) {};
//            LOG("   - FIFO Size = %d\r\n", FIFO_Size(&gRxFifo));
//
//            (void)xEventGroupClearBits(hEvtGroup, ICEMKII_RX_WAITING);
//            (void)xEventGroupClearBits(hEvtGroup, ICEMKII_RX_READY);
//
//            break;
//        }
//
//        bunch++;
//    }

//    vLog_Result(result);
}

//-----------------------------------------------------------------------------

void vTestTask(void * pvParameters)
{
//  FIFO_Init(&A, a, sizeof(a));
//  FIFO_Init(&B, b, sizeof(b));
//  ICEMKII_Init();
//  U32 i = 0; //, result = ICEMKII_MSG_SUCCESS;
//  U8 d = 15;
//
//    msg.MaxSize = sizeof(msgBuffer);
//    msg.Buffer = msgBuffer;
//
//    LOG("Test Task Started\r\n");
//
//    //vTaskDelay(200);
//
//  for(i = 0; i < 7; i++)
//  {
//    A.Put(&A, &d);
//    A.Put(&B, (U8 *)&i);
////        LOG(" - Put Byte - %0.2X", msgs[i]);
////        result = ICEMKII_MESSAGE_PutByte(&msg, msgs[i]);
////        switch ( result )
////        {
////            case ICEMKII_MSG_SUCCESS:
////                LOG("\r\n");
////                break;
////            case ICEMKII_MSG_COMPLETE:
////                LOG(" <- COMPLETE\r\n");
////                break;
////            default:
////                LOG(" <- ERROR: ID = %d\r\n", result - ICEMKII_MSG_ERROR);
////                break;
////        }
//  }

    vTest_FiFo();
    vTest_RxIsReady();
    vTest_NoSpace();

    LOG("-------------------------------------------\r\n");
    LOG(" - Tested = %d  Pass = %d  Fail = %d\r\n", gTested, gPass, gFail);
    LOG("-------------------------------------------\r\n");

    while(FW_TRUE)
    {
        //LOG("Template Task Iteration\r\n");
        vTaskDelay(500);
    }
    //vTaskDelete(NULL);
}

//-----------------------------------------------------------------------------

void Prepare_And_Run_Test( void )
{
    LOG("Preparing Template Test To Run...\r\n");

    xTaskCreate
    (
        vLEDTask,
        "LED",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );

    xTaskCreate
    (
        vTestTask,
        "Template",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );
}

//-----------------------------------------------------------------------------
