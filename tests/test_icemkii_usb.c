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

static U8 msgs[14 * 2] =
{
  /* Message with incorrect CRC */
  0x1B, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x0E, 0x21, 0x43, 0x65, 0x87, 0xFF, 0xFF,
  /* Correct message */
  0x1B, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x0E, 0x21, 0x43, 0x65, 0x87, 0xC2, 0xC3,
};

//-----------------------------------------------------------------------------

#define ICEMKII_RX_READY     (1 << 0)
#define ICEMKII_RX_WAITING   (1 << 1)
//#define ICEMKII_TX_READY         (1)
//#define ICEMKII_RX_READY         (1)

extern FIFO_t                gRxFifo;
extern FIFO_t                gTxFifo;
extern EventGroupHandle_t    hEvtGroup;

extern void icemkii_ProcessTx(void);
extern void icemkii_ProcessRx(void);

//-----------------------------------------------------------------------------

void USB_IRQHandler(void)
{
  //
}

void USB_SetCb_Ep(U32 aNumber, USB_CbEp pCbEp)
{
    (void)aNumber;
    (void)pCbEp;
}

U32 USB_EpReadToFifo(U32 aNumber, USB_CbEpPut pPutCb, U32 aSize)
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

//-----------------------------------------------------------------------------

void vLEDTask(void * pvParameters)
{
    //LOG("LED Task Started\r\n");

    while(TRUE)
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

static void vLog_Result(BOOLEAN result)
{
    if (TRUE == result)
    {
        LOG(" ----- PASS\r\n");
    }
    else
    {
        LOG(" ----- FAIL\r\n");
    }
}

static void vTest_RxIsReady(void)
{
    EventBits_t uxReturned;
    BOOLEAN result = FALSE;
    U8 data = 0;

    LOG("--- ICEMKII Test USB Rx Is Ready ------------------------------\r\n");

    LOG(" - Put one bunch of data from USB EP to FIFO\r\n");
    icemkii_ProcessRx();

    LOG(" - Check if flag in Event Group is set\r\n");
    uxReturned = xEventGroupWaitBits
                 (
                   hEvtGroup,
                   ICEMKII_RX_READY,
                   pdFALSE,
                   pdFALSE,
                   0
                 );

//  if (0 != (uxReturned & ICEMKII_RX_WAITING))
//  {
//    /* Read from OUT EP */
//    LOG("ICEMKII READ:\r\n  - ");
//    size = USB_EpReadToFifo
//           (
//             USB_ICEMKII_EP_BULK_OUT,
//             icemkii_Put,
//             FIFO_Free(&gRxFifo)
//           );
//    LOG(" : Len = %d\r\n", size);
//  }

    if (0 != (uxReturned & ICEMKII_RX_READY))
    {
        result = TRUE;

        LOG("   - FIFO Size = %d\r\n", FIFO_Size(&gRxFifo));

        LOG("   - Clearing FIFO...\r\n");
        while (FIFO_IS_EMPTY != FIFO_Get(&gRxFifo, &data)) {};
        LOG("   - FIFO Size = %d\r\n", FIFO_Size(&gRxFifo));

        (void)xEventGroupClearBits(hEvtGroup, ICEMKII_RX_READY);
    }

    vLog_Result(result);
}

static void vTest_NoSpace(void)
{
    EventBits_t uxReturned;
    BOOLEAN result = FALSE;
    U32 bunch = 0, bunchExpctd, space, spaceExpctd;
    U8 data = 0;

    LOG("--- ICEMKII Test USB No Space ---------------------------------\r\n");

    bunchExpctd = FIFO_Capacity(&gRxFifo) / sizeof(msgs);
    spaceExpctd = bunchExpctd * sizeof(msgs);

    while (TRUE)
    {
        LOG(" - Put %d bunch of data from USB EP to FIFO\r\n", bunch);
        icemkii_ProcessRx();

        LOG(" - Check if flag in Event Group is set\r\n");
        uxReturned = xEventGroupWaitBits
                     (
                       hEvtGroup,
                       ICEMKII_RX_WAITING,
                       pdFALSE,
                       pdFALSE,
                       0
                     );

        if (0 != (uxReturned & ICEMKII_RX_WAITING))
        {
            space = FIFO_Size(&gRxFifo);
            if ((space == spaceExpctd) && (bunch == bunchExpctd))
            {
                result = TRUE;
            }

            LOG("   - FIFO Size = %d Expected = %d\r\n", space, spaceExpctd);
            LOG("   - Bunches = %d Expected = %d\r\n", bunch, bunchExpctd);

            LOG("   - Clearing FIFO...\r\n");
            while (FIFO_IS_EMPTY != FIFO_Get(&gRxFifo, &data)) {};
            LOG("   - FIFO Size = %d\r\n", FIFO_Size(&gRxFifo));

            (void)xEventGroupClearBits(hEvtGroup, ICEMKII_RX_WAITING);
            (void)xEventGroupClearBits(hEvtGroup, ICEMKII_RX_READY);

            break;
        }

        bunch++;
    }

    vLog_Result(result);
}

void vTestTask(void * pvParameters)
{
  ICEMKII_Init();
//    U32 i = 0, result = ICEMKII_MSG_SUCCESS;
//
//    msg.MaxSize = sizeof(msgBuffer);
//    msg.Buffer = msgBuffer;
//
//    LOG("Test Task Started\r\n");
//
//    //vTaskDelay(200);
//
//    for(i = 0; i < sizeof(msgs); i++)
//    {
//        LOG(" - Put Byte - %0.2X", msgs[i]);
//        result = ICEMKII_MESSAGE_PutByte(&msg, msgs[i]);
//        switch ( result )
//        {
//            case ICEMKII_MSG_SUCCESS:
//                LOG("\r\n");
//                break;
//            case ICEMKII_MSG_COMPLETE:
//                LOG(" <- COMPLETE\r\n");
//                break;
//            default:
//                LOG(" <- ERROR: ID = %d\r\n", result - ICEMKII_MSG_ERROR);
//                break;
//        }
//    }

    vTest_RxIsReady();
    vTest_NoSpace();

    while(TRUE)
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
