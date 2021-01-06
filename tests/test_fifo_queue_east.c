#include "stm32f1xx.h"
#include "types.h"
#include "gpio.h"
#include "debug.h"
#include "uniquedevid.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "fifo.h"
#include "east.h"
#include "blockqueue.h"

//-----------------------------------------------------------------------------

typedef __packed struct Command_s
{
    U8  a;
    U8  b;
    U8  c;
    U8  d;
    U8  e;
} Command_t, * Command_p;

typedef FW_BOOLEAN (* TestFunction_t)(void);

static FIFO_p       pUartFifo           = NULL;
static U8           uartFifoBuffer[100] = {0};
static BlockQueue_p pQueue              = NULL;
static U8           QueueBuffer[100]    = {0};
static EAST_p       pEAST               = NULL;
static U8           eastContainer[16] = {0};
//static U8           eastBuffer[100]   = {0};

//--- Mocks -------------------------------------------------------------------

void USB_IRQHandler(void)
{
    //
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN Test_FillTheQueue(void)
{
    FW_BOOLEAN result = FW_TRUE;
    FW_RESULT status = FW_ERROR;
    U8 * pPacket = NULL;
    U32 aPacketSize = 0, i = 0;
    U8 byte = 0;

    /* Create the Block Queue */
    LOG("*** Main Task: Block Queue Init ***\r\n");
    pQueue = BlockQueue_Init
             (
                 QueueBuffer,
                 sizeof(QueueBuffer),
                 sizeof(Command_t)
             );
    result = (FW_BOOLEAN)(NULL != pQueue);

    /* Create the EAST packet */
    LOG("*** Main Task: EAST Init ***\r\n");

    pEAST = EAST_Init(eastContainer, sizeof(eastContainer), NULL, 0);
    result = (FW_BOOLEAN)(NULL != pEAST);

    while (FW_TRUE == result)
    {
        /* Allocate the Block for the EAST packet */
        LOG("*** Main Task: Alocate Block ***\r\n");
        status = BlockQueue_Allocate(pQueue, &pPacket, &aPacketSize);
        result = (FW_BOOLEAN)(FW_SUCCESS == status);
        result &= (FW_BOOLEAN)(NULL != pPacket);
        result &= (FW_BOOLEAN)(0 != aPacketSize);
        if (FW_FALSE == result) break;

        /* Link the Block with the EAST packet */
        LOG("*** Main Task: Link Block with EAST ***\r\n");
        status = EAST_SetBuffer(pEAST, pPacket, aPacketSize);
        result = (FW_BOOLEAN)(FW_SUCCESS == status);
        if (FW_FALSE == result) break;

        /* Wait for UART FIFO Inited */
        LOG("*** Main Task: Wait for UART FIFO Inited ***\r\n");
        while (NULL == pUartFifo)
        {
            vTaskDelay(5);
        }

        /* Read the UART FIFO */
        LOG("*** Main Task: Read the UART FIFO ***\r\n");
        status = FW_INPROGRESS;
        while (FW_INPROGRESS == status)
        {
            if (0 == FIFO_Count(pUartFifo))
            {
                vTaskDelay(5);
                continue;
            }

            for (i = 0; i < FIFO_Count(pUartFifo); i++)
            {
                (void)FIFO_Get(pUartFifo, &byte);
                status = EAST_PutByte(pEAST, byte);
                if (FW_COMPLETE == status) break;
            }
        }

        /* Enqueue the block */
        LOG("*** Enqueue the block ***\r\n");
        status = BlockQueue_Enqueue(pQueue, 5);
        result = (FW_BOOLEAN)(FW_SUCCESS == status);
    }

    LOG("*** Main Task: Done! ***\r\n");

    return FW_TRUE;
}

//-----------------------------------------------------------------------------

static FW_BOOLEAN Test_DoNothing(void)
{
    while (FW_TRUE)
    {
        vTaskDelay(500);
    }
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

static U8 gEastPacket[] =
{
    /* Start Token */
    0x24,
    /* Length */
    0x05, 0x00,
    /* Data */
    0x01, 0x02, 0x03, 0x04, 0x05,
    /* Stop Token */
    0x42,
    /* Control Sum */
    0x01, 0x00
};

void vTestHelpTaskFunction(void * pvParameters)
{
    FW_BOOLEAN result      = FW_FALSE;
    FW_RESULT  status      = FW_ERROR;
    U32        i           = 0;
    U32        count       = 0;
    U8 *       pPacket     = NULL;
    U32        aPacketSize = 0;
    U8         byte        = 0;

    LOG("*** Help Task: UART FIFO Init ***\r\n");

    pUartFifo = FIFO_Init(uartFifoBuffer, sizeof(uartFifoBuffer));
    result = (FW_BOOLEAN)(NULL != pUartFifo);
    LOG("UART FIFO Cap = %d\r\n", FIFO_Size(pUartFifo));

    /* Fill the UART FIFO */
    while (FW_TRUE == result)
    {
        for (i = 0; i < sizeof(gEastPacket); i++)
        {
            status = FIFO_Put(pUartFifo, &gEastPacket[i]);
            result &= (FW_BOOLEAN)(FW_SUCCESS == status);
        }
        vTaskDelay(25);
    }

    /* FIFO is full that meas Main Task enqueued all the blocks */
    result = FW_TRUE;
    count = BlockQueue_GetCountOfAllocated(pQueue);
    for (i = 0; i < count; i++)
    {
        /* Dequeue the block */
        LOG("*** Help Task: Deueue the Block ***\r\n");
        status = BlockQueue_Dequeue(pQueue, &pPacket, &aPacketSize);
        result &= (FW_BOOLEAN)(FW_SUCCESS == status);
        result &= (FW_BOOLEAN)(NULL != pPacket);
        result &= (FW_BOOLEAN)(0 != aPacketSize);
        if (FW_FALSE == result) break;

        /* Link the Block with the EAST packet */
        LOG("*** Help Task: Link Block with EAST ***\r\n");
        status = EAST_SetBuffer(pEAST, pPacket, aPacketSize);
        result = (FW_BOOLEAN)(FW_SUCCESS == status);
        if (FW_FALSE == result) break;

        /* Read the EAST */
        LOG("*** Help Task: Read the EAST ***\r\n");
        status = FW_INPROGRESS;
        while (FW_INPROGRESS == status)
        {
            status = EAST_GetByte(pEAST, &byte);
        }
        result = (FW_BOOLEAN)(FW_COMPLETE == status);
        if (FW_FALSE == result) break;

        /* Release the block */
        LOG("*** Help Task: Release the block ***\r\n");
        status = BlockQueue_Release(pQueue);
        result &= (FW_BOOLEAN)(FW_SUCCESS == status);
        if (FW_FALSE == result) break;
    }

    LOG("*** Help Task: Deueued = %d, Count = %d ***\r\n", i, count);
    LOG("*** Help Task: Done! ***\r\n");

    while (FW_TRUE)
    {
        vTaskDelay(500);
    }
}

//-----------------------------------------------------------------------------

const TestFunction_t gTests[] =
{
    Test_FillTheQueue,
    Test_DoNothing,
};

U32 TestsGetCount(void)
{
    return (sizeof(gTests) / sizeof(TestFunction_t));
}

//-----------------------------------------------------------------------------
